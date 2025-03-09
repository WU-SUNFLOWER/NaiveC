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
    prog->node_->Accept(this);
    return nullptr;
}

llvm::Value *PrintVisitor::VisitDeclStmt(DeclStmt* decl_stmt) {
    int i = 0;
    int j = decl_stmt->nodes_.size() - 1;
    for (const auto& node : decl_stmt->nodes_) {
        node->Accept(this);
        ++i;
        if (i == j) {
            *out_ << ";";
        }
    }

    return nullptr;
}

llvm::Value *PrintVisitor::VisitBlockStmt(BlockStmt* block_stmt) {
    *out_ << "{";
    for (const auto& node : block_stmt->nodes_) {
        node->Accept(this);
        *out_ << ";";
    }
    *out_ << "}";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitIfStmt(IfStmt* if_stmt) {
    *out_ << "if(";
    if_stmt->cond_node_->Accept(this);
    *out_ << ")";
    if_stmt->then_node_->Accept(this);
    if (if_stmt->else_node_) {
        *out_ << "else";
        if_stmt->else_node_->Accept(this);
    }

    return nullptr;
}

llvm::Value *PrintVisitor::VisitForStmt(ForStmt* for_stmt) {
    *out_ << "for(";
    if (for_stmt->init_node_) {
        for_stmt->init_node_->Accept(this);
    }
    *out_ << ";";
    if (for_stmt->cond_node_) {
        for_stmt->cond_node_->Accept(this);
    }
    *out_ << ";";
    if (for_stmt->inc_node_) {
        for_stmt->inc_node_->Accept(this);
    }
    *out_ << ")";

    if (for_stmt->body_node_) {
        for_stmt->body_node_->Accept(this);
    }
    
    return nullptr;
}

llvm::Value *PrintVisitor::VisitBreakStmt(BreakStmt *) {
    *out_ << "break";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitContinueStmt(ContinueStmt *) {
    *out_ << "continue";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitUnaryExpr(UnaryExpr* expr) {
    switch (expr->op_) {
        case UnaryOpCode::kPositive:
            *out_ << "+";
            break;
        case UnaryOpCode::kNegative:
            *out_ << "-";
            break;
        case UnaryOpCode::kSelfIncreasing:
            *out_ << "++";
            break;
        case UnaryOpCode::kSelfDecreasing:
            *out_ << "--";
            break;
        case UnaryOpCode::kDereference:
            *out_ << "*";
            break;
        case UnaryOpCode::kAddress:
            *out_ << "&";
            break;
        case UnaryOpCode::kLogicalNot:
            *out_ << "!";
            break;
        case UnaryOpCode::kBitwiseNot:
            *out_ << "~";
            break;
        default:
            llvm::errs() << "Unknown unary opcode: " 
                         << static_cast<int>(expr->op_) 
                         << "\n";
    }

    expr->sub_node_->Accept(this);

    return nullptr;
}

llvm::Value *PrintVisitor::VisitBinaryExpr(BinaryExpr *binary_expr) {
    binary_expr->left_->Accept(this);

    switch (binary_expr->op_) {
        case BinaryOpCode::kEqualEqual:
            *out_ <<"==";
            break;
        case BinaryOpCode::kNotEqual:
            *out_ <<"!=";
            break;
        case BinaryOpCode::kLess:
            *out_ <<"<";
            break;
        case BinaryOpCode::kGreater:
            *out_ <<">";
            break;
        case BinaryOpCode::kLessEqual:
            *out_ <<"<=";
            break;
        case BinaryOpCode::kGreaterEqual:
            *out_ <<">=";
            break;
        case BinaryOpCode::kAdd:
            *out_ <<"+";
            break;
        case BinaryOpCode::kSub:
            *out_ <<"-";
            break;
        case BinaryOpCode::kMul:
            *out_ <<"*";
            break;
        case BinaryOpCode::kDiv:
            *out_ <<"/";
            break;
        case BinaryOpCode::kMod:
            *out_ <<"%";
            break;
        case BinaryOpCode::kLogicalOr:
            *out_ <<"||";
            break;
        case BinaryOpCode::kLogicalAnd:
            *out_ <<"&&";
            break;
        case BinaryOpCode::kBitwiseOr:
            *out_ <<"|";
            break;
        case BinaryOpCode::kBitwiseAnd:
            *out_ <<"&";
            break;
        case BinaryOpCode::kBitwiseXor:
            *out_ <<"^";
            break;
        case BinaryOpCode::kLeftShift:
            *out_ <<"<<";
            break;
        case BinaryOpCode::kRightShift:
            *out_ <<">>";
            break;
        case BinaryOpCode::kAssign:
            *out_ <<"=";
            break;
        case BinaryOpCode::kAddAssign:
            *out_ <<"+=";
            break;
        case BinaryOpCode::kSubAssign:
            *out_ <<"-=";
            break;
        case BinaryOpCode::kMulAssign:
            *out_ <<"*=";
            break;
        case BinaryOpCode::kDivAssign:
            *out_ <<"/=";
            break;
        case BinaryOpCode::kModAssign:
            *out_ <<"%=";
            break;
        case BinaryOpCode::kLeftShiftAssign:
            *out_ <<"<<=";
            break;
        case BinaryOpCode::kRightShiftAssign:
            *out_ <<">>=";
            break;
        case BinaryOpCode::kBitwiseAndAssign:
            *out_ <<"&=";
            break;
        case BinaryOpCode::kBitwiseOrAssign:
            *out_ <<"|=";
            break;
        case BinaryOpCode::kBitwiseXorAssign:
            *out_ <<"^=";
            break;
        case BinaryOpCode::kComma:
            *out_ <<",";
            break;
        default:
            llvm::errs() <<"Unknown binary opcode:"
                         << static_cast<int>(binary_expr->op_) 
                         <<"\n";
    }

    binary_expr->right_->Accept(this);

    return nullptr;
}

llvm::Value *PrintVisitor::VisitTernaryExpr(TernaryExpr* expr) {
    expr->cond_->Accept(this);
    *out_ << "?";
    expr->then_->Accept(this);
    *out_ << ":";
    expr->els_->Accept(this);

    return nullptr;
}

llvm::Value* PrintVisitor::VisitNumberExpr(NumberExpr *number_expr) {
    *out_ << number_expr->GetNumber();
    return nullptr;
}

llvm::Value *PrintVisitor::VisitVariableAccessExpr(VariableAccessExpr* access_expr) {
    *out_ << access_expr->GetVariableName();
    return nullptr;
}

llvm::Value *PrintVisitor::VisitVariableDecl(VariableDecl* decl) {
    decl->GetCType()->Accept(this);
    *out_ << decl->GetVariableName();

    int nr_init_values = decl->init_values_.size();
    if (nr_init_values > 0) {
        *out_ << "=";
        for (int i = 0; i < nr_init_values; ++i) {
            const auto& init_value_struct = decl->init_values_[i];
            init_value_struct->init_node->Accept(this);
            if (i != nr_init_values - 1) {
                *out_ << ",";
            }
        }
    }
    
    return nullptr;
}

llvm::Value *PrintVisitor::VisitSizeofExpr(SizeofExpr* expr) {
    *out_ << "sizeof ";
    if (expr->sub_ctype_) {
        *out_ << "(";
        expr->sub_ctype_->Accept(this);
        *out_ << ")";
    }
    else if (expr->sub_node_) {
        expr->sub_node_->Accept(this);
    }
    return nullptr;
}

llvm::Value *PrintVisitor::VisitPostIncExpr(PostIncExpr* expr) {
    expr->sub_node_->Accept(this);
    *out_ << "++";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitPostDecExpr(PostDecExpr* expr) {
    expr->sub_node_->Accept(this);
    *out_ << "--";
    return nullptr;
}

llvm::Value *PrintVisitor::VisitPostSubscript(PostSubscriptExpr* expr) {
    expr->sub_node_->Accept(this);
    *out_ << "[";
    expr->index_node_->Accept(this);
    *out_ << "]";

    return nullptr;
}

llvm::Type *PrintVisitor::VisitPrimaryType(CPrimaryType* ctype) {
    if (ctype->GetKind() == CType::TypeKind::kInt) {
        *out_ << "int ";
    }
    return nullptr;
}

llvm::Type *PrintVisitor::VisitPointerType(CPointerType* ctype) {
    ctype->GetBaseType()->Accept(this);
    *out_ << "*";
    return nullptr;
}

llvm::Type *PrintVisitor::VisitArrayType(CArrayType* ctype) {
    *out_ << "[";
    *out_ << ctype->GetElementCount();
    *out_ << "]";
    ctype->GetElementType()->Accept(this);

    return nullptr;
}

llvm::Type *PrintVisitor::VisitRecordType(CRecordType* ctype) {
    auto tag = ctype->GetTagKind();
    switch (tag) {
        case CType::TagKind::kStruct:
            *out_ << "struct ";
            break;
        case CType::TagKind::kUnion:
            *out_ << "union ";
            break;
    }

    *out_ << ctype->GetName() << "{";
    for (const auto& member : ctype->GetMembers()) {
        member.type->Accept(this);
        *out_ << member.name << ";";
    }
    *out_ << "} ";

    return nullptr;
}
