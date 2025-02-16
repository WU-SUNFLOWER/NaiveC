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

    llvm::Value* result;
    for (auto& node : prog->nodes_) {
        result = node->Accept(this);
    }
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
    for (const auto& node : block_stmt->nodes_) {
        node->Accept(this);
    }
    return nullptr;
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

    return nullptr;
}

llvm::Value* CodeGen::VisitBinaryExpr(BinaryExpr *binary_expr) {
    llvm::Value* left = binary_expr->left_->Accept(this);
    llvm::Value* right = binary_expr->right_->Accept(this);

    switch (binary_expr->op_) {
        case OpCode::kEqualEqual: {
            auto val = ir_builder_.CreateICmpEQ(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case OpCode::kNotEqual: {
            auto val = ir_builder_.CreateICmpNE(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case OpCode::kLess: {
            auto val = ir_builder_.CreateICmpSLT(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case OpCode::kLessEqual: {
            auto val = ir_builder_.CreateICmpSLE(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case OpCode::kGreater: {
            auto val = ir_builder_.CreateICmpSGT(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case OpCode::kGreaterEqual: {
            auto val = ir_builder_.CreateICmpSGE(left, right);
            return ir_builder_.CreateIntCast(val, ir_builder_.getInt32Ty(), true);
        }
        case OpCode::kAdd:
            return ir_builder_.CreateNSWAdd(left, right);
        case OpCode::kSub:
            return ir_builder_.CreateNSWSub(left, right);
        case OpCode::kMul:
            return ir_builder_.CreateNSWMul(left, right);
        case OpCode::kDiv:
            return ir_builder_.CreateSDiv(left, right);
        default:
            llvm::errs() << "Unknown opcode: " 
                         << static_cast<int>(binary_expr->op_) 
                         << "\n";
    }

    return nullptr;
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
    llvm::Type* ir_type = nullptr;
    const llvm::StringRef& variable_name = decl_node->GetVariableName();
    if (decl_node->GetCType() == CType::GetIntType()) {
        ir_type = ir_builder_.getInt32Ty();
    }
    else {
        llvm::errs() << "Try to delcare variable with unknown type: " << variable_name;
        return nullptr;
    }

    llvm::Value* value = ir_builder_.CreateAlloca(ir_type, nullptr, variable_name);
    variable_map_.insert({ variable_name, { value, ir_type } });

    return value;
}

llvm::Value* CodeGen::VisitAssignExpr(AssignExpr* assign_node) {
    auto access_node = assign_node->GetLeftChild();

    assert(llvm::isa<VariableAccessExpr>(access_node.get()));

    auto variable_name = static_cast<VariableAccessExpr*>(access_node.get())->GetVariableName();
    auto& variable_info = variable_map_[variable_name];
    llvm::Value* variable_addr = variable_info.first;
    llvm::Type* variable_ir_type = variable_info.second;

    // Compute the right node at first.
    auto right_node = assign_node->GetRightChild();
    llvm::Value* right_value = right_node->Accept(this);

    // Generate store ir instruction.
    ir_builder_.CreateStore(right_value, variable_addr);

    // Return our target variable as a lvalue.
    // auto ret = ir_builder_.CreateLoad(variable_ir_type, variable_addr, variable_name);
    return ir_builder_.CreateLoad(variable_ir_type, variable_addr, variable_name);
}

llvm::Value* CodeGen::VisitNumberExpr(NumberExpr *factor_expr) {
    return ir_builder_.getInt32(factor_expr->GetNumber());
}
