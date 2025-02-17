// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "print-visitor.h"

#include "llvm/Support/raw_ostream.h"

PrintVisitor::PrintVisitor(std::shared_ptr<Program> program, llvm::raw_ostream *out) {
    this->out_ = out;
    VisitProgram(program.get());
}

llvm::Value* PrintVisitor::VisitProgram(Program* prog) {
    for (const auto& node : prog->nodes_) {
        node->Accept(this);
        *out_ << "\n";
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
    *out_ << "{\n";
    for (const auto& node : block_stmt->nodes_) {
        *out_ << "   ";
        node->Accept(this);
    }
    *out_ << "\n}\n";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitIfStmt(IfStmt* if_stmt) {
    *out_ << "if (";
    if_stmt->cond_node_->Accept(this);
    *out_ << ") \n";
    if_stmt->then_node_->Accept(this);
    if (if_stmt->else_node_) {
        *out_ << "\nelse\n";
        if_stmt->else_node_->Accept(this);
    }

    return nullptr;
}

llvm::Value *PrintVisitor::VisitForStmt(ForStmt* for_stmt) {
    *out_ << "for (";
    if (for_stmt->init_node_) {
        for_stmt->init_node_->Accept(this);
    }
    *out_ << "; ";
    if (for_stmt->cond_node_) {
        for_stmt->cond_node_->Accept(this);
    }
    *out_ << "; ";
    if (for_stmt->inc_node_) {
        for_stmt->inc_node_->Accept(this);
    }
    *out_ << ") ";
    if (for_stmt->body_node_) {
        for_stmt->body_node_->Accept(this);
    }
    *out_ << "; ";
    
    return nullptr;
}

llvm::Value *PrintVisitor::VisitBreakStmt(BreakStmt *) {
    *out_ << "break;";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitContinueStmt(ContinueStmt *) {
    *out_ << "continue;";
    return nullptr;
}

llvm::Value* PrintVisitor::VisitBinaryExpr(BinaryExpr *binary_expr) {
    binary_expr->left_->Accept(this);

    switch (binary_expr->op_) {
        case OpCode::kEqualEqual:
            *out_ << " == ";
            break;
        case OpCode::kNotEqual:
            *out_ << " != ";
            break;
        case OpCode::kLess:
            *out_ << " < ";
            break;
        case OpCode::kGreater:
            *out_ << " > ";
            break;
        case OpCode::kLessEqual:
            *out_ << " <= ";
            break;
        case OpCode::kGreaterEqual:
            *out_ << " >= ";
            break;
        case OpCode::kAdd:
            *out_ << " + ";
            break;
        case OpCode::kSub:
            *out_ << " - ";
            break;
        case OpCode::kMul:
            *out_ << " * ";
            break;
        case OpCode::kDiv:
            *out_ << " / ";
            break;
        case OpCode::kMod:
            *out_ << " % ";
            break;
        case OpCode::kLogicOr:
            *out_ << " || ";
            break;
        case OpCode::kLogicAnd:
            *out_ << " && ";
            break;
        case OpCode::kBitwiseOr:
            *out_ << " | ";
            break;
        case OpCode::kBitwiseAnd:
            *out_ << " & ";
            break;
        case OpCode::kBitwiseXor:
            *out_ << " ^ ";
            break;
        case OpCode::kLeftShift:
            *out_ << " << ";
            break;
        case OpCode::kRightShift:
            *out_ << " >> ";
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
    *out_ << access_expr->GetVariableName();
    return nullptr;
}

llvm::Value *PrintVisitor::VisitVariableDecl(VariableDecl* decl) {
    if (decl->GetCType() == CType::GetIntType()) {
        *out_ << "int " << decl->GetVariableName() << ";";
    }
    return nullptr;
}

llvm::Value *PrintVisitor::VisitAssignExpr(AssignExpr* assign_expr) {
    assign_expr->left_->Accept(this);
    *out_ << " = ";
    assign_expr->right_->Accept(this);
    *out_ << "\n";
    return nullptr;
}

llvm::Value* PrintVisitor::VisitNumberExpr(NumberExpr *number_expr) {
    *out_ << number_expr->GetNumber();
    return nullptr;
}
