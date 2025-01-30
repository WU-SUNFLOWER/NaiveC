// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "print-visitor.h"

#include "llvm/Support/raw_ostream.h"

PrintVisitor::PrintVisitor(std::shared_ptr<Program> prog) {
    VisitProgram(prog.get());
}

llvm::Value* PrintVisitor::VisitProgram(Program *prog) {
    for (auto& expr : prog->expr_vec_) {
        expr->Accept(this);
        llvm::outs() << "\n";
    }
    return nullptr;
}

llvm::Value* PrintVisitor::VisitBinaryExpr(BinaryExpression *binary_expr) {
    binary_expr->left_->Accept(this);
    binary_expr->right_->Accept(this);

    switch (binary_expr->op_) {
        case OpCode::kAdd:
            llvm::outs() << " + ";
            break;
        case OpCode::kSub:
            llvm::outs() << " - ";
            break;
        case OpCode::kMul:
            llvm::outs() << " * ";
            break;
        case OpCode::kDiv:
            llvm::outs() << " / ";
            break;
        default:
            llvm::errs() << "Unknown opcode: " 
                         << static_cast<int>(binary_expr->op_) 
                         << "\n";
    }

    return nullptr;
}

llvm::Value* PrintVisitor::VisitFactorExpr(FactorExpression *factor_expr) {
    llvm::outs() << " " << factor_expr->number_ << " ";
    return nullptr;
}
