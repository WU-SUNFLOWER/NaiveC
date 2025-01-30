// Copyright 2024 WU-SUNFLOWER. All rights reserved.
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

    llvm::BasicBlock* entry_block = llvm::BasicBlock::Create(context_, "entry", func);
    ir_builder_.SetInsertPoint(entry_block);

    for (auto& expr : prog->expr_vec_) {
        llvm::Value* result = expr->Accept(this);
        ir_builder_.CreateCall(print_func, { ir_builder_.CreateGlobalStringPtr("expr val: %d\n"), result});
    }

    llvm::Value* ret = ir_builder_.CreateRet(ir_builder_.getInt32(0));
    llvm::verifyFunction(*func);

    module_->print(llvm::outs(), nullptr);

    return ret;
}

llvm::Value* CodeGen::VisitBinaryExpr(BinaryExpression *binary_expr) {
    llvm::Value* left = binary_expr->left_->Accept(this);
    llvm::Value* right = binary_expr->right_->Accept(this);

    switch (binary_expr->op_) {
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

llvm::Value* CodeGen::VisitFactorExpr(FactorExpression *factor_expr) {
    return ir_builder_.getInt32(factor_expr->number_);
}
