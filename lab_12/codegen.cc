// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "codegen.h"

#include <memory>
#include <cassert>

#include "llvm/IR/Verifier.h"

void CodeGen::AddLocalVariable(llvm::StringRef name, llvm::Value *addr, llvm::Type *llvm_type) {
    assert(local_variable_map_.size() > 0);
    local_variable_map_.back().insert({ name, { addr, llvm_type } });
}

void CodeGen::AddGlobalVariable(llvm::StringRef name, llvm::Value *addr, llvm::Type *llvm_type) {
    global_variable_map_.insert({ name, { addr, llvm_type } });
}

std::pair<llvm::Value *, llvm::Type *> CodeGen::GetVariableByName(llvm::StringRef name) {
    for (auto iter = local_variable_map_.rbegin(); iter != local_variable_map_.rend(); ++iter) {
        const auto& map = *iter;
        if (map.contains(name)) {
            return map.at(name);
        }
    }

    assert(global_variable_map_.contains(name));
    return global_variable_map_.at(name);
}

void CodeGen::PushScope() {
    local_variable_map_.emplace_back();
}

void CodeGen::PopScope() {
    local_variable_map_.pop_back();
}

void CodeGen::ClearVariableScope() {
    local_variable_map_.clear();
}

CodeGen::CodeGen(std::shared_ptr<Program> prog) {
    module_ = std::make_unique<llvm::Module>(prog->file_name_, context_);
    VisitProgram(prog.get());
}

llvm::Value* CodeGen::VisitProgram(Program *prog) {
    for (const auto& node : prog->nodes_) {
        node->Accept(this);
    }
    return nullptr;
}

llvm::Value *CodeGen::VisitDeclStmt(DeclStmt* decl_stmt) {
    for (const auto& node : decl_stmt->nodes_) {
        node->Accept(this);
    }
    return nullptr;
}

llvm::Value *CodeGen::VisitBlockStmt(BlockStmt* block_stmt) {
    llvm::Value* ret = nullptr;
    for (const auto& node : block_stmt->nodes_) {
        ret = node->Accept(this);
    }
    return ret;
}

llvm::Value *CodeGen::VisitIfStmt(IfStmt* if_stmt) {
    auto cond_block = llvm::BasicBlock::Create(context_, "cond");
    auto then_block = llvm::BasicBlock::Create(context_, "then");
    llvm::BasicBlock* else_block = nullptr;
    if (if_stmt->else_node_) {
        else_block = llvm::BasicBlock::Create(context_, "else");
    }
    auto final_block = llvm::BasicBlock::Create(context_, "final");

    // Don't forget to let our program jump into cond block at first!
    ir_builder_.CreateBr(cond_block);

    // Let's deal with cond block at first.
    cond_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(cond_block);
    // Generate the instructions of condition expression itself.
    llvm::Value* cond_expr_val = if_stmt->cond_node_->Accept(this);
    CastValue(&cond_expr_val, ir_builder_.getInt32Ty());
    // Generate a compare instruction, 
    // to check whether the return value of condition expression is true.
    llvm::Value* is_cond_expr_val_true = ir_builder_.CreateICmpNE(cond_expr_val, ir_builder_.getInt32(0));
    // Generate the conditional jumping instruction.
    ir_builder_.CreateCondBr(is_cond_expr_val_true, then_block, if_stmt->else_node_ ? else_block : final_block);

    // Generate the instructions of then block.
    then_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(then_block);
    if_stmt->then_node_->Accept(this);

    auto current_block = ir_builder_.GetInsertBlock();
    if (current_block->empty() || !current_block->back().isTerminator()) {
        ir_builder_.CreateBr(final_block);
    }

    // Generate the instructions of else block.
    if (if_stmt->else_node_) {
        else_block->insertInto(GetCurrentFunc());
        ir_builder_.SetInsertPoint(else_block);
        if_stmt->else_node_->Accept(this);

        auto current_block = ir_builder_.GetInsertBlock();
        if (current_block->empty() || !current_block->back().isTerminator()) {
            ir_builder_.CreateBr(final_block);
        }
    }

    // Let our code generator to generate later code after if statement in final block...
    final_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(final_block);

    return nullptr;
}

llvm::Value *CodeGen::VisitForStmt(ForStmt* for_stmt) {
    auto init_block = llvm::BasicBlock::Create(context_, "for.init", GetCurrentFunc());
    auto cond_block = llvm::BasicBlock::Create(context_, "for.cond");
    auto inc_block = llvm::BasicBlock::Create(context_, "for.inc");
    auto body_block = llvm::BasicBlock::Create(context_, "for.body");
    auto final_block = llvm::BasicBlock::Create(context_, "for.final");

    // Don't forget to bind the statement node with its final block, and inc block.
    break_block_map_.insert({ for_stmt, final_block });
    continue_block_map_.insert({ for_stmt, inc_block });

    // Build init block.
    ir_builder_.CreateBr(init_block);
    ir_builder_.SetInsertPoint(init_block);
    if (for_stmt->init_node_) {
        for_stmt->init_node_->Accept(this);
    }
    ir_builder_.CreateBr(cond_block);

    // Build cond block.
    cond_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(cond_block);
    if (for_stmt->cond_node_) {
        llvm::Value* cond_result = for_stmt->cond_node_->Accept(this);
        CastValue(&cond_result, ir_builder_.getInt32Ty());
        llvm::Value* is_cond_result_true = ir_builder_.CreateICmpNE(cond_result, ir_builder_.getInt32(0));
        ir_builder_.CreateCondBr(is_cond_result_true, body_block, final_block);
    } else {
        ir_builder_.CreateBr(body_block);
    }

    // Build body block.
    body_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(body_block);
    if (for_stmt->body_node_) {
        for_stmt->body_node_->Accept(this);
    }

    auto current_block = ir_builder_.GetInsertBlock();
    if (current_block->empty() || !current_block->back().isTerminator()) {
        ir_builder_.CreateBr(inc_block);
    }

    // Build inc block.
    inc_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(inc_block);
    if (for_stmt->inc_node_) {
        for_stmt->inc_node_->Accept(this);
    }
    ir_builder_.CreateBr(cond_block);

    // Let our code generator to generate later code after for statement in final block...
    final_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(final_block);

    // Don't forget to unbind the statement node with its final block, and inc block.
    break_block_map_.erase(for_stmt);
    continue_block_map_.erase(for_stmt);

    return nullptr;
}

llvm::Value *CodeGen::VisitBreakStmt(BreakStmt* stmt) {
    llvm::BasicBlock* target_block = break_block_map_.at(stmt->target_.get());
    ir_builder_.CreateBr(target_block);

    llvm::BasicBlock* death_block = llvm::BasicBlock::Create(context_, "for.break.death", GetCurrentFunc());
    ir_builder_.SetInsertPoint(death_block);

    return nullptr;
}

llvm::Value *CodeGen::VisitContinueStmt(ContinueStmt* stmt) {
    llvm::BasicBlock* target_block = continue_block_map_.at(stmt->target_.get());
    ir_builder_.CreateBr(target_block);

    llvm::BasicBlock* death_block = llvm::BasicBlock::Create(context_, "for.continue.death", GetCurrentFunc());
    ir_builder_.SetInsertPoint(death_block);

    return nullptr;
}

llvm::Value *CodeGen::VisitSizeofExpr(SizeofExpr* expr) {
    if (expr->sub_ctype_) {
        return ir_builder_.getInt32(expr->sub_ctype_->GetSize());
    }
    if (expr->sub_node_) {
        return ir_builder_.getInt32(expr->sub_node_->GetCType()->GetSize());
    }
    assert(0);
    return nullptr;
}

llvm::Value *CodeGen::VisitUnaryExpr(UnaryExpr* expr) {
    auto value = expr->sub_node_->Accept(this);
    auto ctype = expr->sub_node_->GetCType();

    switch (expr->op_) {
        case UnaryOpCode::kPositive: {
            return value;
        }
        case UnaryOpCode::kNegative: {
            return ir_builder_.CreateNeg(value);
        }
        case UnaryOpCode::kSelfIncreasing: {    
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(value);
            assert(target);
            llvm::Value* new_value;
            if (ctype->GetKind() == CType::TypeKind::kPointer) {
                new_value = ir_builder_.CreateInBoundsGEP(ctype->Accept(this), value, { ir_builder_.getInt32(1) });
            } else {
                new_value = ir_builder_.CreateNSWAdd(value, ir_builder_.getInt32(1));
            }
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case UnaryOpCode::kSelfDecreasing: {    
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(value);
            assert(target);
            llvm::Value* new_value;
            if (ctype->GetKind() == CType::TypeKind::kPointer) {
                new_value = ir_builder_.CreateInBoundsGEP(ctype->Accept(this), value, { ir_builder_.getInt32(-1) });
            } else {
                new_value = ir_builder_.CreateNSWSub(value, ir_builder_.getInt32(1));
            }
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case UnaryOpCode::kDereference: {
            auto pointer_ctype = llvm::dyn_cast<CPointerType>(ctype.get());
            assert(pointer_ctype);
            ctype = pointer_ctype->GetBaseType();
            return ir_builder_.CreateLoad(ctype->Accept(this), value);
        }
        case UnaryOpCode::kAddress: {
            return llvm::dyn_cast<llvm::LoadInst>(value)->getPointerOperand();
        }
        case UnaryOpCode::kLogicalNot: {
            auto value_is_true = ir_builder_.CreateICmpNE(value, ir_builder_.getInt32(0));
            return ir_builder_.CreateZExt(ir_builder_.CreateNot(value_is_true), ir_builder_.getInt32Ty());
        }
        case UnaryOpCode::kBitwiseNot: {
            return ir_builder_.CreateNot(value);
        }
    }

    return nullptr;
}

llvm::Value *CodeGen::VisitBinaryExpr(BinaryExpr* binary_expr) {
    auto op_code = binary_expr->op_;
    switch (op_code) {
        // left && right
        case BinaryOpCode::kLogicalAnd: {
            auto left = binary_expr->left_->Accept(this);
            CastValue(&left, ir_builder_.getInt32Ty());
            auto is_left_true = ir_builder_.CreateICmpNE(left, ir_builder_.getInt32(0));

            auto next_block = llvm::BasicBlock::Create(context_, "next_block");
            auto false_block = llvm::BasicBlock::Create(context_, "false_block");
            auto merge_block = llvm::BasicBlock::Create(context_, "merge_block");

            ir_builder_.CreateCondBr(is_left_true, next_block, false_block);

            // Build next_block
            next_block->insertInto(GetCurrentFunc());
            ir_builder_.SetInsertPoint(next_block);
            auto right = binary_expr->right_->Accept(this);
            CastValue(&right, ir_builder_.getInt32Ty());
            auto is_right_true = ir_builder_.CreateICmpNE(right, ir_builder_.getInt32(0));
            is_right_true = ir_builder_.CreateZExt(is_right_true, ir_builder_.getInt32Ty());
            ir_builder_.CreateBr(merge_block);
            // Don't forget to update the next_block,
            // since new block might be created after 
            // `binary_expr->right_->Accept(this)` is called.
            next_block = ir_builder_.GetInsertBlock();

            // Build false_block
            false_block->insertInto(GetCurrentFunc());
            ir_builder_.SetInsertPoint(false_block);
            ir_builder_.CreateBr(merge_block);

            // Build merge_block
            merge_block->insertInto(GetCurrentFunc());
            ir_builder_.SetInsertPoint(merge_block);
            auto phi = ir_builder_.CreatePHI(ir_builder_.getInt32Ty(), 2);
            phi->addIncoming(ir_builder_.getInt32(0), false_block);
            phi->addIncoming(is_right_true, next_block);

            return phi;
        }
        // left || right
        case BinaryOpCode::kLogicalOr: {
            auto left = binary_expr->left_->Accept(this);
            CastValue(&left, ir_builder_.getInt32Ty());
            auto is_left_true = ir_builder_.CreateICmpNE(left, ir_builder_.getInt32(0));

            auto next_block = llvm::BasicBlock::Create(context_, "next_block");
            auto true_block = llvm::BasicBlock::Create(context_, "true_block");
            auto merge_block = llvm::BasicBlock::Create(context_, "merge_block");

            ir_builder_.CreateCondBr(is_left_true, true_block, next_block);

            // Build next_block
            next_block->insertInto(GetCurrentFunc());
            ir_builder_.SetInsertPoint(next_block);
            auto right = binary_expr->right_->Accept(this);
            CastValue(&right, ir_builder_.getInt32Ty());
            auto is_right_true = ir_builder_.CreateICmpNE(right, ir_builder_.getInt32(0));
            is_right_true = ir_builder_.CreateZExt(is_right_true, ir_builder_.getInt32Ty());
            ir_builder_.CreateBr(merge_block);
            next_block = ir_builder_.GetInsertBlock();

            // Build true_block
            true_block->insertInto(GetCurrentFunc());
            ir_builder_.SetInsertPoint(true_block);
            ir_builder_.CreateBr(merge_block);

            // Build merge_block
            merge_block->insertInto(GetCurrentFunc());
            ir_builder_.SetInsertPoint(merge_block);
            auto phi = ir_builder_.CreatePHI(ir_builder_.getInt32Ty(), 2);
            phi->addIncoming(ir_builder_.getInt32(1), true_block);
            phi->addIncoming(is_right_true, next_block);

            return phi;            
        }
    }

    auto left = binary_expr->left_->Accept(this);
    auto right = binary_expr->right_->Accept(this);

    switch (op_code) {
        case BinaryOpCode::kEqualEqual: {
            auto val = ir_builder_.CreateICmpEQ(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case BinaryOpCode::kNotEqual: {
            auto val = ir_builder_.CreateICmpNE(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case BinaryOpCode::kLess: {
            auto val = ir_builder_.CreateICmpSLT(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case BinaryOpCode::kLessEqual: {
            auto val = ir_builder_.CreateICmpSLE(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case BinaryOpCode::kGreater: {
            auto val = ir_builder_.CreateICmpSGT(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case BinaryOpCode::kGreaterEqual: {
            auto val = ir_builder_.CreateICmpSGE(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case BinaryOpCode::kAdd: {
            auto _ctype = left->getType();
            if (_ctype->isPointerTy()) {
                return ir_builder_.CreateInBoundsGEP(_ctype, left, { right });
            }
            return ir_builder_.CreateNSWAdd(left, right);
        }
        case BinaryOpCode::kSub: {
            auto _ctype = left->getType();
            if (_ctype->isPointerTy()) {
                return ir_builder_.CreateInBoundsGEP(_ctype, left, { ir_builder_.CreateNeg(right) });
            }
            return ir_builder_.CreateNSWSub(left, right);
        }
        case BinaryOpCode::kMul:
            return ir_builder_.CreateNSWMul(left, right);
        case BinaryOpCode::kDiv:
            return ir_builder_.CreateSDiv(left, right);
        case BinaryOpCode::kMod:
            return ir_builder_.CreateSRem(left, right);
        case BinaryOpCode::kBitwiseOr:
            return ir_builder_.CreateOr(left, right);
        case BinaryOpCode::kBitwiseAnd:
            return ir_builder_.CreateAnd(left, right);
        case BinaryOpCode::kBitwiseXor:
            return ir_builder_.CreateXor(left, right);
        case BinaryOpCode::kLeftShift:
            return ir_builder_.CreateShl(left, right);
        case BinaryOpCode::kRightShift:
            return ir_builder_.CreateAShr(left, right);
        case BinaryOpCode::kAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            ir_builder_.CreateStore(right, target->getPointerOperand());
            return right;
        }
        case BinaryOpCode::kAddAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);

            llvm::Value* new_value = nullptr;
            auto _ctype = left->getType();
            if (_ctype->isPointerTy()) {
                new_value = ir_builder_.CreateInBoundsGEP(_ctype, left, { right });
            } else {
                new_value = ir_builder_.CreateNSWAdd(left, right);
            }

            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kSubAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);

            llvm::Value* new_value = nullptr;
            auto _ctype = left->getType();
            if (_ctype->isPointerTy()) {
                new_value = ir_builder_.CreateInBoundsGEP(_ctype, left, { ir_builder_.CreateNot(right) });
            } else {
                new_value = ir_builder_.CreateNSWSub(left, right);
            }

            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kMulAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateNSWMul(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kDivAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateSDiv(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kModAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateSRem(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kLeftShiftAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateShl(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kRightShiftAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateAShr(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kBitwiseAndAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateAnd(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kBitwiseOrAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateOr(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kBitwiseXorAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateXor(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kComma: {
            return right;
        }
        default:
            llvm::errs() << "Unknown opcode: " 
                         << static_cast<int>(binary_expr->op_) 
                         << "\n";
    }

    return nullptr;
}

llvm::Value *CodeGen::VisitTernaryExpr(TernaryExpr* expr) {
    auto then_block = llvm::BasicBlock::Create(context_, "ternary.then");
    auto els_block = llvm::BasicBlock::Create(context_, "ternary.else");
    auto merge_block = llvm::BasicBlock::Create(context_, "ternary.merge");

    auto cond_val = expr->cond_->Accept(this);
    CastValue(&cond_val, ir_builder_.getInt32Ty());
    auto is_cond_true = ir_builder_.CreateICmpNE(cond_val, ir_builder_.getInt32(0));
    ir_builder_.CreateCondBr(is_cond_true, then_block, els_block);

    then_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(then_block);

    auto then_value = expr->then_->Accept(this);
    then_block = ir_builder_.GetInsertBlock();
    ir_builder_.CreateBr(merge_block);

    els_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(els_block);

    auto els_value = expr->els_->Accept(this);
    els_block = ir_builder_.GetInsertBlock();
    ir_builder_.CreateBr(merge_block);

    merge_block->insertInto(GetCurrentFunc());
    ir_builder_.SetInsertPoint(merge_block);
    auto phi = ir_builder_.CreatePHI(expr->GetCType()->Accept(this), 2);
    phi->addIncoming(then_value, then_block);
    phi->addIncoming(els_value, els_block);

    return phi;
}

llvm::Value* CodeGen::VisitVariableAccessExpr(VariableAccessExpr* access_node) {
    auto variable_name = access_node->GetVariableName();
    auto [variable_addr, variable_llvm_type] = GetVariableByName(variable_name);

    if (variable_llvm_type->isFunctionTy()) {
        return variable_addr;
    } else {
        return ir_builder_.CreateLoad(variable_llvm_type, variable_addr, variable_name);
    }

    return nullptr;
}

std::shared_ptr<VariableDecl::InitValue> CodeGen::GetInitValueStructByIndexList(
    const VariableDecl* decl_node, 
    const std::vector<int> &target_index_list) 
{
    const auto& init_values = decl_node->init_values_;
    for (const auto& init_value_struct : init_values) {
        const auto& cur_index_list = init_value_struct->index_list;
        if (cur_index_list.size() != target_index_list.size()) {
            continue;
        } 
        bool find = true;
        for (int i = 0; i < target_index_list.size(); ++i) {
            if (cur_index_list[i] != target_index_list[i]) {
                find = false;
                break;
            } 
        }
        if (find) {
            return init_value_struct;
        }
    }

    return nullptr;
}

llvm::Constant *CodeGen::GetInitialValueForGlobalVariable(
    const VariableDecl* decl_node, 
    llvm::Type *type, 
    std::vector<int>& index_list) 
{
    if (type->isIntegerTy()) {
        auto init_value_struct = GetInitValueStructByIndexList(decl_node, index_list);
        if (init_value_struct) {
            auto init_value = init_value_struct->init_node->Accept(this);
            auto init_type = init_value_struct->decl_type->Accept(this);
            assert(type == init_type);
            CastValue(&init_value, init_type);
            return llvm::dyn_cast<llvm::Constant>(init_value);
        }
        return ir_builder_.getInt32(0);
    }
    else if (type->isPointerTy()) {
        auto init_value_struct = GetInitValueStructByIndexList(decl_node, index_list);
        if (init_value_struct) {
            auto init_value = init_value_struct->init_node->Accept(this);
            auto init_type = init_value_struct->decl_type->Accept(this);
            assert(type == init_type);
            CastValue(&init_value, init_type);
            return llvm::dyn_cast<llvm::Constant>(init_value);
        } else {
            auto pointer_type = llvm::dyn_cast<llvm::PointerType>(type);
            return llvm::ConstantPointerNull::get(pointer_type);            
        }
    }
    else if (type->isStructTy()) {
        auto struct_type = llvm::dyn_cast<llvm::StructType>(type);
        auto element_count = struct_type->getStructNumElements();
        llvm::SmallVector<llvm::Constant*> element_value_vec;
        for (auto i = 0; i < element_count; ++i) {
            auto element_type = struct_type->getStructElementType(i);
            index_list.push_back(i);
            auto element_value = GetInitialValueForGlobalVariable(decl_node, element_type, index_list);
            index_list.pop_back();
            element_value_vec.push_back(element_value);
        }
        return llvm::ConstantStruct::get(struct_type, element_value_vec);
    }
    else if (type->isArrayTy()) {
        auto array_type = llvm::dyn_cast<llvm::ArrayType>(type);
        auto element_type = array_type->getArrayElementType();
        auto element_count = array_type->getArrayNumElements();
        llvm::SmallVector<llvm::Constant*> element_value_vec;
        for (auto i = 0; i < element_count; ++i) {
            index_list.push_back(i);
            auto element_value = GetInitialValueForGlobalVariable(decl_node, element_type, index_list);
            index_list.pop_back();
            element_value_vec.push_back(element_value);
        }
        return llvm::ConstantArray::get(array_type, element_value_vec);
    }
    else {
        assert(0);
    }

    return nullptr;
}

llvm::Value* CodeGen::VisitGlobalVariableDecl(VariableDecl* decl_node) {
    auto variable_type = decl_node->GetCType();
    auto variable_llvm_type = variable_type->Accept(this);
    auto variable_name = decl_node->GetVariableName();
    auto variable_addr = new llvm::GlobalVariable(
                                            *module_,
                                            variable_llvm_type,
                                            false, 
                                            llvm::GlobalValue::ExternalLinkage,
                                            nullptr,
                                            variable_name);

    variable_addr->setAlignment(llvm::Align(variable_type->GetAlign()));

    std::vector<int> index_list = { 0 };
    variable_addr->setInitializer(GetInitialValueForGlobalVariable(decl_node, variable_llvm_type, index_list));

    AddGlobalVariable(variable_name, variable_addr, variable_llvm_type);

    return variable_addr;
}

llvm::Value *CodeGen::VisitLocalVariableDecl(VariableDecl* decl_node) {
    auto variable_type = decl_node->GetCType();
    auto variable_llvm_type = decl_node->GetCType()->Accept(this);
    auto variable_name = decl_node->GetVariableName();    
    
    llvm::IRBuilder tmp_ir_builder(
                            &GetCurrentFunc()->getEntryBlock(), 
                            GetCurrentFunc()->getEntryBlock().begin());
    // NOTE: 
    // We generate `alloca` instruction for local variable in entry basic block, 
    // which is the start of a function.
    // This is to help LLVM to generate more optimized machine code.
    auto variable_addr = tmp_ir_builder.CreateAlloca(variable_llvm_type, nullptr, variable_name);
    variable_addr->setAlignment(llvm::Align(variable_type->GetAlign()));

    AddLocalVariable(variable_name, variable_addr, variable_llvm_type);

    int nr_init_values = decl_node->init_values_.size();
    if (nr_init_values > 0) {
        if (nr_init_values == 1) {
            auto init_value_struct = decl_node->init_values_[0];
            auto init_value_type = decl_node->init_values_[0]->decl_type->Accept(this);
            auto init_value = init_value_struct->init_node->Accept(this);
            CastValue(&init_value, init_value_type);
            ir_builder_.CreateStore(init_value, variable_addr);
        }
        else if (llvm::ArrayType* arr_llvm_type = llvm::dyn_cast<llvm::ArrayType>(variable_llvm_type)) {
            for (const auto& init_value_struct : decl_node->init_values_) {
                // Create llvm-style index list, by splicing `init_values_`list of `decl_node`.
                llvm::SmallVector<llvm::Value*> llvm_index_list;
                for (auto index : init_value_struct->index_list) {
                    llvm_index_list.push_back(ir_builder_.getInt32(index));
                }
                // Create code to compute the address and value of this element.
                auto element_addr = ir_builder_.CreateInBoundsGEP(
                                                            arr_llvm_type,
                                                            variable_addr,
                                                            llvm_index_list);
                auto element_value = init_value_struct->init_node->Accept(this);
                // Force cast the type of element value.
                auto element_type = init_value_struct->decl_type->Accept(this);
                CastValue(&element_value, element_type);
                // Create store code.
                ir_builder_.CreateStore(element_value, element_addr);
            }
        }
        else if (llvm::StructType* struct_llvm_type = llvm::dyn_cast<llvm::StructType>(variable_llvm_type)) {
            auto record_type = llvm::dyn_cast<CRecordType>(variable_type.get());
            auto record_tag_kind = record_type->GetTagKind();
            switch (record_tag_kind) {
                case CType::TagKind::kStruct: {
                    for (const auto& init_value_struct : decl_node->init_values_) {
                        llvm::SmallVector<llvm::Value*> llvm_index_list;
                        for (auto index : init_value_struct->index_list) {
                            llvm_index_list.push_back(ir_builder_.getInt32(index));
                        }
                        auto member_addr = ir_builder_.CreateInBoundsGEP(
                                                                struct_llvm_type,
                                                                variable_addr,
                                                                llvm_index_list);
                        auto member_value = init_value_struct->init_node->Accept(this);
                        auto member_type = init_value_struct->decl_type->Accept(this);
                        CastValue(&member_value, member_type);
                        ir_builder_.CreateStore(member_value, member_addr);
                    }
                    break;
                }
                case CType::TagKind::kUnion: {
                    auto init_value_struct = decl_node->init_values_[0];
                    assert(decl_node->init_values_.size() == 1);
                    
                    llvm::SmallVector<llvm::Value*> llvm_index_list;

                    assert(init_value_struct->index_list.size() == 2);
                    for (auto index : init_value_struct->index_list) {
                        llvm_index_list.push_back(ir_builder_.getInt32(index));
                    }

                    auto member_type = init_value_struct->decl_type->Accept(this);
                    auto member_value = init_value_struct->init_node->Accept(this);
                    auto member_pointer = ir_builder_.CreateInBoundsGEP(
                                                            struct_llvm_type, 
                                                            variable_addr, 
                                                            llvm_index_list);
                    auto vaild_pointer_llvm_type = llvm::PointerType::getUnqual(member_type);
                    auto cast_pointer = ir_builder_.CreateBitCast(member_pointer, vaild_pointer_llvm_type);
                    
                    ir_builder_.CreateStore(member_value, cast_pointer);
                    break;
                }
                default:
                    assert(0);
            }
        }
        else {
            assert(0);
        }
    }

    return variable_addr;
}

llvm::Value* CodeGen::VisitVariableDecl(VariableDecl* decl_node) {
    return decl_node->is_global_ ? 
                VisitGlobalVariableDecl(decl_node) : 
                VisitLocalVariableDecl(decl_node);
}

llvm::Value *CodeGen::VisitPostIncExpr(PostIncExpr* expr) {
    llvm::Value* value = expr->sub_node_->Accept(this);
    llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(value);
    assert(target);

    llvm::Value* new_value;
    auto ctype = expr->sub_node_->GetCType();
    if (ctype->GetKind() == CType::TypeKind::kPointer) {
        new_value = ir_builder_.CreateInBoundsGEP(ctype->Accept(this), value, { ir_builder_.getInt32(1) });
    } else {
        new_value = ir_builder_.CreateNSWAdd(value, ir_builder_.getInt32(1));
    }
    ir_builder_.CreateStore(new_value, target->getPointerOperand());

    return value;
}

llvm::Value *CodeGen::VisitPostDecExpr(PostDecExpr* expr) {
    llvm::Value* value = expr->sub_node_->Accept(this);
    llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(value);
    assert(target);

    llvm::Value* new_value;
    auto ctype = expr->sub_node_->GetCType();
    if (ctype->GetKind() == CType::TypeKind::kPointer) {
        new_value = ir_builder_.CreateInBoundsGEP(ctype->Accept(this), value, { ir_builder_.getInt32(-1) });
    } else {
        new_value = ir_builder_.CreateNSWSub(value, ir_builder_.getInt32(1));
    }
    ir_builder_.CreateStore(new_value, target->getPointerOperand());

    return value;
}

llvm::Value *CodeGen::VisitPostSubscript(PostSubscriptExpr* expr) {
    llvm::Type* element_type = expr->GetCType()->Accept(this);
    llvm::Value* target = expr->sub_node_->Accept(this);
    llvm::Value* index = expr->index_node_->Accept(this);

    llvm::Type* target_type = target->getType();
    llvm::Value* element_addr = nullptr;
    if (target_type->isArrayTy()) {
        // Q: Why we should call `getPointerOperand()` method of `array` here?
        // A: Get the first address of our target.
        // For example, since `array` is an array pointer, for accessing `array[index]`, 
        // we should get the array's starting address at first,
        // instead of the value of the first element in this array, i.e. `*array`.
        llvm::Value* array_addr = llvm::dyn_cast<llvm::LoadInst>(target)->getPointerOperand();
        // Compute the address of `array[index]`.
        element_addr = ir_builder_.CreateInBoundsGEP(element_type, array_addr, { index });
    } 
    else if (target_type->isPointerTy()) {
        element_addr = ir_builder_.CreateInBoundsGEP(element_type, target, { index });
    }
    else {
        assert(0);
    }

    return ir_builder_.CreateLoad(element_type, element_addr);
}

llvm::Value* CodeGen::VisitNumberExpr(NumberExpr *factor_expr) {
    return ir_builder_.getInt32(factor_expr->GetNumber());
}

llvm::Value *CodeGen::VisitPostMemberDotExpr(PostMemberDotExpr* expr) {
    auto struct_object = expr->struct_node_->Accept(this);
    auto struct_pointer = llvm::dyn_cast<llvm::LoadInst>(struct_object)->getPointerOperand();
    
    auto struct_type = llvm::dyn_cast<CRecordType>(expr->struct_node_->GetCType().get());
    auto struct_llvm_type = struct_type->Accept(this);
    
    auto struct_tag = struct_type->GetTagKind();

    auto& struct_member = expr->target_member_;
    auto struct_member_llvm_type = struct_member.type->Accept(this);

    auto zero = ir_builder_.getInt32(0);
    switch (struct_tag) {
        case CType::TagKind::kStruct: {
            auto next = ir_builder_.getInt32(struct_member.rank);
            auto member_addr = ir_builder_.CreateInBoundsGEP(
                                                struct_llvm_type,
                                                struct_pointer, 
                                                { zero, next });
            return ir_builder_.CreateLoad(struct_member_llvm_type, member_addr);
        }
        case CType::TagKind::kUnion: {
            auto member_pointer = ir_builder_.CreateInBoundsGEP(
                                                    struct_llvm_type, 
                                                    struct_pointer, 
                                                    { zero, zero });
            auto vaild_pointer_llvm_type = llvm::PointerType::getUnqual(struct_member_llvm_type);
            auto cast_pointer = ir_builder_.CreateBitCast(member_pointer, vaild_pointer_llvm_type);
            return ir_builder_.CreateLoad(struct_member_llvm_type, cast_pointer);   
        }
    }

    return nullptr;
}

llvm::Value *CodeGen::VisitPostMemberArrowExpr(PostMemberArrowExpr* expr) {
    auto struct_pointer = expr->struct_pointer_node_->Accept(this);

    auto struct_pointer_type = llvm::dyn_cast<CPointerType>(expr->struct_pointer_node_->GetCType().get());
    auto struct_type = llvm::dyn_cast<CRecordType>(struct_pointer_type->GetBaseType().get());

    auto struct_llvm_type = struct_type->Accept(this);
    auto struct_tag = struct_type->GetTagKind();

    auto& struct_member = expr->target_member_;
    auto struct_member_llvm_type = struct_member.type->Accept(this);

    auto zero = ir_builder_.getInt32(0);
    switch (struct_tag) {
        case CType::TagKind::kStruct: {
            auto next = ir_builder_.getInt32(struct_member.rank);
            auto member_addr = ir_builder_.CreateInBoundsGEP(struct_llvm_type, struct_pointer, { zero, next });
            return ir_builder_.CreateLoad(struct_member_llvm_type, member_addr);
        }
        case CType::TagKind::kUnion: {
            auto member_pointer = ir_builder_.CreateInBoundsGEP(struct_llvm_type, struct_pointer, { zero, zero });
            auto vaild_pointer_llvm_type = llvm::PointerType::getUnqual(struct_member_llvm_type);
            auto cast_pointer = ir_builder_.CreateBitCast(member_pointer, vaild_pointer_llvm_type);
            return ir_builder_.CreateLoad(struct_member_llvm_type, cast_pointer);
        }
    }

    return nullptr;
}

llvm::Type *CodeGen::VisitPrimaryType(CPrimaryType* ctype) {
    switch (ctype->GetKind()) {
        case CType::TypeKind::kInt:
            return ir_builder_.getInt32Ty();
        case CType::TypeKind::kVoid:
            return ir_builder_.getVoidTy();
        default:
            assert(0);
    }
    return nullptr;
}

llvm::Type* CodeGen::VisitPointerType(CPointerType* ctype) {
    llvm::Type* base_type = ctype->GetBaseType()->Accept(this);
    return llvm::PointerType::getUnqual(base_type);
}

llvm::Type* CodeGen::VisitArrayType(CArrayType* ctype) {
    llvm::Type* element_type = ctype->GetElementType()->Accept(this);
    return llvm::ArrayType::get(element_type, ctype->GetElementCount());
}

llvm::Type* CodeGen::VisitRecordType(CRecordType* ctype) {
    auto struct_type_name = ctype->GetName();
    auto struct_type = llvm::StructType::getTypeByName(context_, struct_type_name);
    if (struct_type) {
        return struct_type;
    }

    struct_type = llvm::StructType::create(context_, struct_type_name);

    auto tag_kind = ctype->GetTagKind();
    switch (tag_kind) {
        case CType::TagKind::kStruct: {
            llvm::SmallVector<llvm::Type*> member_type_vec;
            for (const auto& member : ctype->GetMembers()) {
                member_type_vec.push_back(member.type->Accept(this));
            }
            struct_type->setBody(member_type_vec);
            break;
        }
        case CType::TagKind::kUnion: {
            auto& members = ctype->GetMembers();
            auto rank = ctype->GetMaxSizeMemberRank();
            auto llvm_type = members[rank].type->Accept(this);
            struct_type->setBody(llvm_type);
            break;
        }
        default: {
            assert(0);
        }
    }

    return struct_type;
}

llvm::Value *CodeGen::VisitFuncDecl(FuncDecl* func_decl) {
    // We assume that you cannot nest a new function within a function.
    ClearVariableScope();

    auto func_type = llvm::dyn_cast<CFuncType>(func_decl->GetCType().get());
    auto func_llvm_type = llvm::dyn_cast<llvm::FunctionType>(func_type->Accept(this));
    auto func_name = func_type->GetFuncName();

    auto func = module_->getFunction(func_name);
    
    // 1.Create LLVM function instance.
    if (!func) {
        func = llvm::Function::Create(func_llvm_type, 
                                      llvm::GlobalValue::ExternalLinkage, 
                                      func_name,
                                      module_.get());
    }

    // 2. Save function instance to global variable map.
    AddGlobalVariable(func_name, func, func_llvm_type);

    // 3. Process function's parameters.
    const auto& params = func_type->GetParams();
    int i = 0;
    for (auto& arg : func->args()) {
        arg.setName(params[i++].name);
    }

    // 4.1 Does the function have valid body?
    //     If not, return the object directly.
    if (func_decl->block_stmt_ == nullptr) {
        return func;
    }

    // 4.2 If yes, create the entry block for the function.
    //     and going to generate its inner code.
    auto entry_block = llvm::BasicBlock::Create(context_, "entry", func);
    ir_builder_.SetInsertPoint(entry_block);
    SetCurrentFunc(func);

    PushScope();
    {
        // 5. Alloc space for the arguments of the function.
        for (auto& arg : func->args()) {
            auto arg_addr = ir_builder_.CreateAlloca(arg.getType(), nullptr, arg.getName());
            arg_addr->setAlignment(arg.getParamAlign().valueOrOne());
            ir_builder_.CreateStore(&arg, arg_addr);

            AddLocalVariable(arg.getName(), arg_addr, arg.getType());
        }

        // 6.Generate inner code for function's block statement.
        func_decl->block_stmt_->Accept(this);     
        assert(GetCurrentFunc() == func);

        // 7. Generate default `return` instruction for function's block statement.
        const auto& back_block = func->back();
        if (back_block.empty() || 
            !llvm::isa<llvm::ReturnInst>(back_block.back())) 
        {
            switch (func_type->GetRetType()->GetKind()) {
                case CType::TypeKind::kVoid: {
                    ir_builder_.CreateRetVoid();
                    break;                    
                }
                case CType::TypeKind::kInt: {
                    ir_builder_.CreateRet(ir_builder_.getInt32(0));
                    break;                    
                }
                case CType::TypeKind::kPointer: {
                    auto ret_type = func_llvm_type->getReturnType();
                    auto pointer_type = llvm::dyn_cast<llvm::PointerType>(ret_type);
                    auto null_ret_val = llvm::ConstantPointerNull::get(pointer_type);
                    ir_builder_.CreateRet(null_ret_val);
                    break;                    
                }
                default: {
                    assert(0);
                }
            }
        }
    }
    PopScope();

    assert(GetCurrentFunc() == func);

    assert(!llvm::verifyFunction(*func, &llvm::outs()));
    assert(!llvm::verifyModule(*module_, &llvm::outs()));

    return func;
}

llvm::Value *CodeGen::VisitPostFuncCallExpr(PostFuncCallExpr* func_call_expr) {
    auto func_node = func_call_expr->func_node_;
    auto func_type = llvm::dyn_cast<CFuncType>(func_node->GetCType().get());
    auto func_llvm_inst = llvm::dyn_cast<llvm::Function>(func_node->Accept(this));
    auto func_llvm_type = llvm::dyn_cast<llvm::FunctionType>(func_type->Accept(this));

    const auto& func_params = func_type->GetParams();
    const auto& func_args = func_call_expr->arg_nodes_;
    assert(func_params.size() == func_args.size());

    llvm::SmallVector<llvm::Value*> args;
    for (int i = 0; i < func_args.size(); ++i) {
        // Force cast the type of argument.
        auto arg_value = func_args[i]->Accept(this);
        auto param_type = func_params[i].type->Accept(this);
        CastValue(&arg_value, param_type);
        // Add the argument to argument list.
        args.push_back(arg_value);
    }

    return ir_builder_.CreateCall(func_llvm_type, func_llvm_inst, args);
}

llvm::Value *CodeGen::VisitReturnStmt(ReturnStmt* ret_stmt) {
    auto ret_value_node = ret_stmt->value_node_;
    if (ret_value_node) {
        auto ret_value = ret_value_node->Accept(this);
        return ir_builder_.CreateRet(ret_value);
    } else {
        return ir_builder_.CreateRetVoid();
    }
    return nullptr;
}

llvm::Type *CodeGen::VisitFuncType(CFuncType* func_type) {
    llvm::Type* ret_llvm_type = func_type->GetRetType()->Accept(this);

    llvm::SmallVector<llvm::Type*> param_llvm_types;
    for (const auto& param : func_type->GetParams()) {
        param_llvm_types.push_back(param.type->Accept(this));
    }

    return llvm::FunctionType::get(ret_llvm_type, param_llvm_types, false);
}

void CodeGen::CastValue(llvm::Value** value, llvm::Type* dest_type) {
    auto cur_type = (*value)->getType();
    if (cur_type == dest_type) {
        return;
    }

    if (cur_type->isIntegerTy()) {
        if (dest_type->isPointerTy()) {
            *value = ir_builder_.CreateIntToPtr(*value, dest_type);
        }
    }
    else if (cur_type->isPointerTy()) {
        if (dest_type->isIntegerTy()) {
            *value = ir_builder_.CreatePtrToInt(*value, dest_type);
        }
    }
    else if (cur_type->isArrayTy()) {
        if (dest_type->isPointerTy()) {
            *value = llvm::dyn_cast<llvm::LoadInst>(*value)->getPointerOperand();
        }
    }
}
