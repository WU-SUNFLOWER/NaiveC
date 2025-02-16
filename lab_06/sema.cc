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

std::shared_ptr<AstNode> Sema::SemaVariableDeclNode(Token& token, CType *ctype) {
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

std::shared_ptr<AstNode> Sema::SemaAssignExprNode(
        Token& token,
        std::shared_ptr<AstNode> left, 
        std::shared_ptr<AstNode> right) {
    assert(left && right);

    if (!llvm::isa<VariableAccessExpr>(left.get())) {
        const Token& left_token = left->GetBoundToken();
        diag_engine_.Report(
                llvm::SMLoc::getFromPointer(left_token.GetRawContentPtr()),
                Diag::kErrLValue);
    }

    auto assign_expr = std::make_shared<AssignExpr>();
    assign_expr->left_ = left;
    assign_expr->right_ = right;

    return assign_expr;
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
    return variable_access_node;
}

std::shared_ptr<AstNode> Sema::SemaBinaryExprNode(
        std::shared_ptr<AstNode> left, 
        std::shared_ptr<AstNode> right, 
        OpCode op) {
    assert(left && right);

    auto expr = std::make_shared<BinaryExpr>();
    expr->left_ = left;
    expr->right_ = right;
    expr->op_ = op;
    return expr;
}

std::shared_ptr<AstNode> Sema::SemaNumberExprNode(Token& token, CType* ctype) {
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
