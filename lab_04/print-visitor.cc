// Copyright 2025 WU-SUNFLOWER. All rights reserved.
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

llvm::Value* PrintVisitor::VisitBinaryExpr(BinaryExpr *binary_expr) {
    binary_expr->left_->Accept(this);

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

    binary_expr->right_->Accept(this);

    return nullptr;
}

llvm::Value *PrintVisitor::VisitVariableAccessExpr(VariableAccessExpr* access_expr) {
    llvm::outs() << access_expr->name_;
    return nullptr;
}

llvm::Value *PrintVisitor::VisitVariableDecl(VariableDecl* decl) {
    if (decl->GetCType() == CType::GetIntType()) {
        llvm::outs() << "int " << decl->name_ << ";";
    }
    return nullptr;
}

llvm::Value *PrintVisitor::VisitAssignExpr(AssignExpr* assign_expr) {
    assign_expr->left_->Accept(this);
    llvm::outs() << " = ";
    assign_expr->right_->Accept(this);
    return nullptr;
}

llvm::Value* PrintVisitor::VisitNumberExpr(NumberExpr *number_expr) {
    llvm::outs() << number_expr->number_;
    return nullptr;
}
