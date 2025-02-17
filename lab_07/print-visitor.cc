// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "print-visitor.h"

#include "llvm/Support/raw_ostream.h"

PrintVisitor::PrintVisitor(std::shared_ptr<Program> prog) {
    VisitProgram(prog.get());
}

llvm::Value* PrintVisitor::VisitProgram(Program* prog) {
    for (const auto& node : prog->nodes_) {
        node->Accept(this);
        llvm::outs() << "\n";
    }
    return nullptr;
}

llvm::Value *PrintVisitor::VisitDeclStmt(DeclStmt* decl_stmt) {
    for (const auto& node : decl_stmt->nodes_) {
        node->Accept(this);
    }

    return nullptr;
}

llvm::Value *PrintVisitor::VisitBlockStmt(BlockStmt* block_stmt) {
    llvm::outs() << "{\n";
    for (const auto& node : block_stmt->nodes_) {
        llvm::outs() << "   ";
        node->Accept(this);
    }
    llvm::outs() << "\n}\n";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitIfStmt(IfStmt* if_stmt) {
    llvm::outs() << "if (";
    if_stmt->cond_node_->Accept(this);
    llvm::outs() << ") \n";
    if_stmt->then_node_->Accept(this);
    if (if_stmt->else_node_) {
        llvm::outs() << "\nelse\n";
        if_stmt->else_node_->Accept(this);
    }

    return nullptr;
}

llvm::Value *PrintVisitor::VisitForStmt(ForStmt* for_stmt) {
    llvm::outs() << "for (";
    if (for_stmt->init_node_) {
        for_stmt->init_node_->Accept(this);
    }
    llvm::outs() << "; ";
    if (for_stmt->cond_node_) {
        for_stmt->cond_node_->Accept(this);
    }
    llvm::outs() << "; ";
    if (for_stmt->inc_node_) {
        for_stmt->inc_node_->Accept(this);
    }
    llvm::outs() << ") ";
    if (for_stmt->body_node_) {
        for_stmt->body_node_->Accept(this);
    }
    llvm::outs() << "; ";
    
    return nullptr;
}

llvm::Value *PrintVisitor::VisitBreakStmt(BreakStmt *) {
    llvm::outs() << "break;";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitContinueStmt(ContinueStmt *) {
    llvm::outs() << "continue;";
    return nullptr;
}

llvm::Value* PrintVisitor::VisitBinaryExpr(BinaryExpr *binary_expr) {
    binary_expr->left_->Accept(this);

    switch (binary_expr->op_) {
        case OpCode::kEqualEqual:
            llvm::outs() << " == ";
            break;
        case OpCode::kNotEqual:
            llvm::outs() << " != ";
            break;
        case OpCode::kLess:
            llvm::outs() << " < ";
            break;
        case OpCode::kGreater:
            llvm::outs() << " > ";
            break;
        case OpCode::kLessEqual:
            llvm::outs() << " <= ";
            break;
        case OpCode::kGreaterEqual:
            llvm::outs() << " >= ";
            break;
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
    llvm::outs() << access_expr->GetVariableName();
    return nullptr;
}

llvm::Value *PrintVisitor::VisitVariableDecl(VariableDecl* decl) {
    if (decl->GetCType() == CType::GetIntType()) {
        llvm::outs() << "int " << decl->GetVariableName() << ";";
    }
    return nullptr;
}

llvm::Value *PrintVisitor::VisitAssignExpr(AssignExpr* assign_expr) {
    assign_expr->left_->Accept(this);
    llvm::outs() << " = ";
    assign_expr->right_->Accept(this);
    llvm::outs() << "\n";
    return nullptr;
}

llvm::Value* PrintVisitor::VisitNumberExpr(NumberExpr *number_expr) {
    llvm::outs() << number_expr->GetNumber();
    return nullptr;
}
