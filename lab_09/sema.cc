// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "sema.h"

#include <memory>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

void Sema::EnterScope() {
    scope_.EnterScope();
}

void Sema::ExitScope() {
    scope_.ExitScope();
}

std::shared_ptr<AstNode> Sema::SemaVariableDeclNode(Token& token, std::shared_ptr<CType> ctype) {
    // 1. Has the variable name already been defined?
    auto name = token.GetContent();
    auto symbol = scope_.FindVarSymbolInCurrentEnv(name);

    if (symbol) {
        diag_engine_.Report(
                llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                Diag::kErrRedefined,
                name);
    }

    // 2. Add the symbol name to symbol table.
    scope_.AddSymbol(name, SymbolKind::kLocalVariable, ctype);

    // 3. Allocate the variable declare node object.
    auto node = std::make_shared<VariableDecl>();
    node->SetBoundToken(token);
    node->SetCType(ctype);
    return node;
}

std::shared_ptr<AstNode> Sema::SemaVariableAccessNode(Token& token) {
    auto name = token.GetContent();
    auto symbol = scope_.FindVarSymbol(name);

    if (!symbol) {
        diag_engine_.Report(
                llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                Diag::kErrUndefined,
                name);
    }

    auto variable_access_node = std::make_shared<VariableAccessExpr>();
    variable_access_node->SetCType(symbol->GetCType());
    variable_access_node->SetBoundToken(token);
    variable_access_node->SetLValue(true);

    return variable_access_node;
}

std::shared_ptr<AstNode> Sema::SemaBinaryExprNode(
        std::shared_ptr<AstNode> left, 
        std::shared_ptr<AstNode> right, 
        BinaryOpCode op) 
{
    assert(left && right);

    auto expr = std::make_shared<BinaryExpr>();
    expr->left_ = left;
    expr->right_ = right;
    expr->op_ = op;
    expr->SetCType(left->GetCType());

    if (op == BinaryOpCode::kAdd || 
        op == BinaryOpCode::kSub ||
        op == BinaryOpCode::kAddAssign ||
        op == BinaryOpCode::kSubAssign) 
    {
        if (left->GetCType()->GetKind() == CType::TypeKind::kInt &&
            right->GetCType()->GetKind() == CType::TypeKind::kPointer) 
        {
            expr->SetCType(right->GetCType());
        }
    }

    return expr;
}

std::shared_ptr<AstNode> Sema::SemaUnaryExprNode(std::shared_ptr<AstNode> sub, UnaryOpCode op, Token &token) {
    auto node = std::make_shared<UnaryExpr>();
    node->op_ = op;
    node->sub_node_ = sub;

    auto sub_ctype = sub->GetCType();
    switch (op) {
        case UnaryOpCode::kPositive:
        case UnaryOpCode::kNegative:
        case UnaryOpCode::kLogicalNot:
        case UnaryOpCode::kBitwiseNot: {
            if (sub_ctype->GetKind() != CType::TypeKind::kInt) {
                diag_engine_.Report(
                    llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                    Diag::kErrExpectedType, 
                    "int type");
            }
            node->SetCType(sub_ctype);
            break;            
        }
        case UnaryOpCode::kAddress: {
            if (!sub->IsLValue()) {
                diag_engine_.Report(
                    llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                    Diag::kErrExpectedLValue);
            }
            node->SetCType(std::make_shared<CPointerType>(sub_ctype));
            break;            
        }
        case UnaryOpCode::kDereference: {
            if (sub_ctype->GetKind() != CType::TypeKind::kPointer) {
                diag_engine_.Report(
                    llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                    Diag::kErrExpectedType, 
                    "pointer type");
            }
            auto pointer_type = llvm::dyn_cast<CPointerType>(sub_ctype.get());
            node->SetCType(pointer_type->GetBaseType());
            node->SetLValue(true);
            break;            
        }
        // We can use `++` or `--` for both integer and pointer variable.
        case UnaryOpCode::kSelfIncreasing:
        case UnaryOpCode::kSelfDecreasing: {
            if (!sub->IsLValue()) {
                diag_engine_.Report(
                    llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                    Diag::kErrExpectedLValue);
            }
            node->SetCType(sub_ctype);
            break;            
        }
    }

    return node;
}

std::shared_ptr<AstNode> Sema::SemaTernaryExprNode(
    std::shared_ptr<AstNode> cond_node, 
    std::shared_ptr<AstNode> then_node, 
    std::shared_ptr<AstNode> els_node,
    Token& token)
{
    if (then_node->GetCType()->GetKind() != els_node->GetCType()->GetKind()) {
        diag_engine_.Report(llvm::SMLoc::getFromPointer(token.GetRawContentPtr()), Diag::kErrSameType);
    }

    auto node = std::make_shared<TernaryExpr>();
    node->cond_ = cond_node;
    node->then_ = then_node;
    node->els_ = els_node;
    node->SetCType(then_node->GetCType());

    return node;
}

std::shared_ptr<AstNode> Sema::SemaSizeofExprNode(
    std::shared_ptr<AstNode> sub, 
    std::shared_ptr<CType> ctype)
{
    auto node = std::make_shared<SizeofExpr>();
    node->sub_ctype_ = ctype;
    node->sub_node_ = sub;
    node->SetCType(CType::kIntType);
    return node;
}

std::shared_ptr<AstNode> Sema::SemaPostIncExprNode(std::shared_ptr<AstNode> sub, Token& token) {
    if (!sub->IsLValue()) {
        diag_engine_.Report(
            llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
            Diag::kErrExpectedLValue);
    }

    auto node = std::make_shared<PostIncExpr>();
    node->sub_node_ = sub;
    node->SetCType(sub->GetCType());
    return node;
}

std::shared_ptr<AstNode> Sema::SemaPostDecExprNode(std::shared_ptr<AstNode> sub, Token& token) {
    if (!sub->IsLValue()) {
        diag_engine_.Report(
            llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
            Diag::kErrExpectedLValue);
    }

    auto node = std::make_shared<PostDecExpr>();
    node->sub_node_ = sub;
    node->SetCType(sub->GetCType());
    return node;
}

std::shared_ptr<VariableDecl::InitValue> Sema::SemaDeclInitValueStruct(
    std::shared_ptr<CType> decl_type, 
    std::shared_ptr<AstNode> init_node, 
    std::vector<int> &index_list,
    Token& token)
{
    if (decl_type->GetKind() != init_node->GetCType()->GetKind()) {
        diag_engine_.Report(
            llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
            Diag::kErrMiss,
            "same type");
        return nullptr;
    }

    auto init_value_struct = std::make_shared<VariableDecl::InitValue>();
    init_value_struct->decl_type = decl_type;
    init_value_struct->init_node = init_node;
    init_value_struct->index_list = index_list;
    return init_value_struct;
}

// `ar[123]` means `*(ar + 123 * element_size)`
std::shared_ptr<AstNode> Sema::SemaPostSubscriptExprNode(
    std::shared_ptr<AstNode> sub_node, 
    std::shared_ptr<AstNode> index_node,
    Token& token)
{
    auto sub_type = sub_node->GetCType()->GetKind();
    std::shared_ptr<CType> element_type = nullptr;

    switch (sub_type) {
        case CType::TypeKind::kArray: {
            CArrayType* sub_node_type = llvm::dyn_cast<CArrayType>(sub_node->GetCType().get());
            element_type = sub_node_type->GetElementType();
            break;
        }
        case CType::TypeKind::kPointer: {
            CPointerType* sub_node_type = llvm::dyn_cast<CPointerType>(sub_node->GetCType().get());
            element_type = sub_node_type->GetBaseType();
            break;
        }
        default: {
            diag_engine_.Report(llvm::SMLoc::getFromPointer(token.GetRawContentPtr()),
                                Diag::kErrExpectedType,
                                "array or pointer");
            break;
        }
    }

    auto node = std::make_shared<PostSubscriptExpr>();
    node->sub_node_ = sub_node;
    node->index_node_ = index_node;
    node->SetCType(element_type);

    return node;
}

std::shared_ptr<AstNode> Sema::SemaNumberExprNode(Token& token, std::shared_ptr<CType> ctype) {
    auto expr = std::make_shared<NumberExpr>();
    expr->SetCType(ctype);
    expr->SetBoundToken(token);
    return expr;
}

std::shared_ptr<AstNode> Sema::SemaIfStmtNode(
        std::shared_ptr<AstNode> cond_node, 
        std::shared_ptr<AstNode> then_node, 
        std::shared_ptr<AstNode> else_node)
{
    auto node = std::make_shared<IfStmt>();
    node->cond_node_ = cond_node;
    node->then_node_ = then_node;
    node->else_node_ = else_node;

    return node;
}
