// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "codegen.h"

#include <memory>

#include "llvm/IR/Verifier.h"

CodeGen::CodeGen(std::shared_ptr<Program> prog) {
    module_ = std::make_shared<llvm::Module>("Expr Module", context_);
    VisitProgram(prog.get());
}

llvm::Value* CodeGen::VisitProgram(Program *prog) {
    // Build "print" function
    auto print_func_type = llvm::FunctionType::get(
                                    ir_builder_.getInt32Ty(), 
                                    { ir_builder_.getInt8PtrTy() }, 
                                    true);
    auto print_func = llvm::Function::Create(
                                print_func_type, 
                                llvm::GlobalValue::ExternalLinkage, 
                                "printf", 
                                module_.get());

    // Build "main" function
    auto func_type = llvm::FunctionType::get(ir_builder_.getInt32Ty(), false);
    auto func = llvm::Function::Create(func_type, llvm::GlobalValue::ExternalLinkage, "main", module_.get());

    SetCurrentFunc(func);  // Don't forget to record current function!

    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(context_, "entry", func);
    ir_builder_.SetInsertPoint(entry_block);

    llvm::Value* result = prog->node_->Accept(this);

    if (result) {
        ir_builder_.CreateCall(print_func, { ir_builder_.CreateGlobalStringPtr("expr val: %d\n"), result });
    } else {
        ir_builder_.CreateCall(print_func, { ir_builder_.CreateGlobalStringPtr("last result isn't an expression!\n") });
    }
    

    llvm::Value* ret = ir_builder_.CreateRet(ir_builder_.getInt32(0));
    llvm::verifyFunction(*func);

    module_->print(llvm::outs(), nullptr);

    return ret;
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
    // return nullptr;
}

llvm::Value *CodeGen::VisitIfStmt(IfStmt* if_stmt) {
    auto cond_block = llvm::BasicBlock::Create(context_, "cond", GetCurrentFunc());
    auto then_block = llvm::BasicBlock::Create(context_, "then", GetCurrentFunc());
    auto final_block = llvm::BasicBlock::Create(context_, "final", GetCurrentFunc());

    llvm::BasicBlock* else_block = nullptr;
    if (if_stmt->else_node_) {
        else_block = llvm::BasicBlock::Create(context_, "else", GetCurrentFunc());
    }

    // Don't forget to let our program jump into cond block at first!
    ir_builder_.CreateBr(cond_block);

    // Let's deal with cond block at first.
    ir_builder_.SetInsertPoint(cond_block);
    // Generate the instructions of condition expression itself.
    llvm::Value* cond_expr_val = if_stmt->cond_node_->Accept(this);
    // Generate a compare instruction, 
    // to check whether the return value of condition expression is true.
    llvm::Value* is_cond_expr_val_true = ir_builder_.CreateICmpNE(cond_expr_val, ir_builder_.getInt32(0));
    // Generate the conditional jumping instruction.
    ir_builder_.CreateCondBr(is_cond_expr_val_true, then_block, if_stmt->else_node_ ? else_block : final_block);

    // Generate the instructions of then block.
    ir_builder_.SetInsertPoint(then_block);
    if_stmt->then_node_->Accept(this);
    ir_builder_.CreateBr(final_block);

    // Generate the instructions of else block.
    if (if_stmt->else_node_) {
        ir_builder_.SetInsertPoint(else_block);
        if_stmt->else_node_->Accept(this);
        ir_builder_.CreateBr(final_block);
    }

    // Let our code generator to generate later code after if statement in final block...
    ir_builder_.SetInsertPoint(final_block);

    return nullptr;
}

llvm::Value *CodeGen::VisitForStmt(ForStmt* for_stmt) {
    auto init_block = llvm::BasicBlock::Create(context_, "for.init", GetCurrentFunc());
    auto cond_block = llvm::BasicBlock::Create(context_, "for.cond", GetCurrentFunc());
    auto inc_block = llvm::BasicBlock::Create(context_, "for.inc", GetCurrentFunc());
    auto body_block = llvm::BasicBlock::Create(context_, "for.body", GetCurrentFunc());
    auto final_block = llvm::BasicBlock::Create(context_, "for.final", GetCurrentFunc());

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
    ir_builder_.SetInsertPoint(cond_block);
    if (for_stmt->cond_node_) {
        llvm::Value* cond_result = for_stmt->cond_node_->Accept(this);
        llvm::Value* is_cond_result_true = ir_builder_.CreateICmpNE(cond_result, ir_builder_.getInt32(0));
        ir_builder_.CreateCondBr(is_cond_result_true, body_block, final_block);
    } else {
        ir_builder_.CreateBr(body_block);
    }

    // Build body block.
    ir_builder_.SetInsertPoint(body_block);
    if (for_stmt->body_node_) {
        for_stmt->body_node_->Accept(this);
    }
    ir_builder_.CreateBr(inc_block);

    // Build inc block.
    ir_builder_.SetInsertPoint(inc_block);
    if (for_stmt->inc_node_) {
        for_stmt->inc_node_->Accept(this);
    }
    ir_builder_.CreateBr(cond_block);

    // Let our code generator to generate later code after for statement in final block...
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
    if (expr->ctype_) {
        return ir_builder_.getInt64(expr->ctype_->GetSize());
    }
    if (expr->sub_node_) {
        return ir_builder_.getInt64(expr->sub_node_->GetCType()->GetSize());
    }
    assert(0);
    return nullptr;
}

llvm::Value *CodeGen::VisitUnaryExpr(UnaryExpr* expr) {

    return nullptr;
}

llvm::Value *CodeGen::VisitBinaryExpr(BinaryExpr* binary_expr) {
    auto op_code = binary_expr->op_;
    switch (op_code) {
        // left && right
        case BinaryOpCode::kLogicalAnd: {
            auto left = binary_expr->left_->Accept(this);
            auto is_left_true = ir_builder_.CreateICmpNE(left, ir_builder_.getInt32(0));

            auto next_block = llvm::BasicBlock::Create(context_, "next_block", GetCurrentFunc());
            auto false_block = llvm::BasicBlock::Create(context_, "false_block", GetCurrentFunc());
            auto merge_block = llvm::BasicBlock::Create(context_, "merge_block", GetCurrentFunc());

            ir_builder_.CreateCondBr(is_left_true, next_block, false_block);

            // Build next_block
            ir_builder_.SetInsertPoint(next_block);
            auto right = binary_expr->right_->Accept(this);
            auto is_right_true = ir_builder_.CreateICmpNE(right, ir_builder_.getInt32(0));
            is_right_true = ir_builder_.CreateZExt(is_right_true, ir_builder_.getInt32Ty());
            ir_builder_.CreateBr(merge_block);
            // Don't forget to update the next_block,
            // since new block might be created after 
            // `binary_expr->right_->Accept(this)` is called.
            next_block = ir_builder_.GetInsertBlock();

            // Build false_block
            ir_builder_.SetInsertPoint(false_block);
            ir_builder_.CreateBr(merge_block);

            // Build merge_block
            ir_builder_.SetInsertPoint(merge_block);
            auto phi = ir_builder_.CreatePHI(ir_builder_.getInt32Ty(), 2);
            phi->addIncoming(ir_builder_.getInt32(0), false_block);
            phi->addIncoming(is_right_true, next_block);

            return phi;
        }
        // left || right
        case BinaryOpCode::kLogicalOr: {
            auto left = binary_expr->left_->Accept(this);
            auto is_left_true = ir_builder_.CreateICmpNE(left, ir_builder_.getInt32(0));

            auto next_block = llvm::BasicBlock::Create(context_, "next_block", GetCurrentFunc());
            auto true_block = llvm::BasicBlock::Create(context_, "true_block", GetCurrentFunc());
            auto merge_block = llvm::BasicBlock::Create(context_, "merge_block", GetCurrentFunc());

            ir_builder_.CreateCondBr(is_left_true, true_block, next_block);

            // Build next_block
            ir_builder_.SetInsertPoint(next_block);
            auto right = binary_expr->right_->Accept(this);
            auto is_right_true = ir_builder_.CreateICmpNE(right, ir_builder_.getInt32(0));
            is_right_true = ir_builder_.CreateZExt(is_right_true, ir_builder_.getInt32Ty());
            ir_builder_.CreateBr(merge_block);
            next_block = ir_builder_.GetInsertBlock();

            // Build true_block
            ir_builder_.SetInsertPoint(true_block);
            ir_builder_.CreateBr(merge_block);

            // Build merge_block
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
        case BinaryOpCode::kAdd:
            return ir_builder_.CreateNSWAdd(left, right);
        case BinaryOpCode::kSub:
            return ir_builder_.CreateNSWSub(left, right);
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
            // a += 3  => a = a + 3
            auto new_value = ir_builder_.CreateNSWAdd(left, right);
            ir_builder_.CreateStore(new_value, target->getPointerOperand());
            return new_value;
        }
        case BinaryOpCode::kSubAssign: {
            llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(left);
            assert(target);
            auto new_value = ir_builder_.CreateNSWSub(left, right);
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
    auto then_block = llvm::BasicBlock::Create(context_, "ternary.then", GetCurrentFunc());
    auto els_block = llvm::BasicBlock::Create(context_, "ternary.else", GetCurrentFunc());
    auto merge_block = llvm::BasicBlock::Create(context_, "ternary.merge", GetCurrentFunc());

    auto cond_val = expr->cond_->Accept(this);
    auto is_cond_true = ir_builder_.CreateICmpNE(cond_val, ir_builder_.getInt32(0));
    ir_builder_.CreateCondBr(is_cond_true, then_block, els_block);

    ir_builder_.SetInsertPoint(then_block);
    auto then_value = expr->then_->Accept(this);
    then_block = ir_builder_.GetInsertBlock();
    ir_builder_.CreateBr(merge_block);

    ir_builder_.SetInsertPoint(els_block);
    auto els_value = expr->els_->Accept(this);
    els_block = ir_builder_.GetInsertBlock();
    ir_builder_.CreateBr(merge_block);

    ir_builder_.SetInsertPoint(merge_block);
    auto phi = ir_builder_.CreatePHI(expr->GetCType()->Accept(this), 2);
    phi->addIncoming(then_value, then_block);
    phi->addIncoming(els_value, els_block);

    return phi;
}

llvm::Value* CodeGen::VisitVariableAccessExpr(VariableAccessExpr* access_node) {
    const llvm::StringRef& variable_name = access_node->GetVariableName();
    auto& variable_info = variable_map_[variable_name];

    llvm::Value* variable_addr = variable_info.first;
    llvm::Type* variable_ir_type = variable_info.second;

    auto ret = ir_builder_.CreateLoad(variable_ir_type, variable_addr, variable_name);

    return ret;
}

llvm::Value* CodeGen::VisitVariableDecl(VariableDecl* decl_node) {
    llvm::Type* ir_type = decl_node->GetCType()->Accept(this);
    const llvm::StringRef& variable_name = decl_node->GetVariableName();

    llvm::Value* variable_addr = ir_builder_.CreateAlloca(ir_type, nullptr, variable_name);
    variable_map_.insert({ variable_name, { variable_addr, ir_type } });

    if (decl_node->init_node_) {
        auto init_val = decl_node->init_node_->Accept(this);
        ir_builder_.CreateStore(init_val, variable_addr);
    }

    return variable_addr;
}

llvm::Value *CodeGen::VisitPostIncExpr(PostIncExpr* expr) {
    llvm::Value* value = expr->sub_node_->Accept(this);
    llvm::LoadInst* target = llvm::dyn_cast<llvm::LoadInst>(value);
    assert(target);

    llvm::Value* new_value;
    auto ctype = expr->sub_node_->GetCType();
    if (ctype->GetKind() == CType::TypeKind::kPointer) {
        auto pointer_type = llvm::dyn_cast<CPointerType>(ctype.get());
        auto element_type = pointer_type->GetBaseType();
        new_value = ir_builder_.CreateNUWAdd(value, ir_builder_.getInt64(element_type->GetSize()));
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
        auto pointer_type = llvm::dyn_cast<CPointerType>(ctype.get());
        auto element_type = pointer_type->GetBaseType();
        new_value = ir_builder_.CreateNUWSub(value, ir_builder_.getInt64(element_type->GetSize()));
    } else {
        new_value = ir_builder_.CreateNSWSub(value, ir_builder_.getInt32(1));
    }
    ir_builder_.CreateStore(new_value, target->getPointerOperand());

    return value;
}


llvm::Value* CodeGen::VisitNumberExpr(NumberExpr *factor_expr) {
    return ir_builder_.getInt32(factor_expr->GetNumber());
}

llvm::Type *CodeGen::VisitPrimaryType(CPrimaryType* ctype) {
    if (ctype->GetKind() == CType::TypeKind::kInt) {
        return ir_builder_.getInt32Ty();
    }
    else {
        assert(0);
    }
    return nullptr;
}

llvm::Type *CodeGen::VisitPointerType(CPointerType* ctype) {
    llvm::Type* base_type = ctype->GetBaseType()->Accept(this);
    return llvm::PointerType::getUnqual(base_type);
}
