// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "sema.h"

#include <memory>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

std::shared_ptr<AstNode> Sema::SemaVariableDeclNode(const llvm::StringRef& name, CType *ctype) {
    // 1. Has the variable name already been defined?
    auto symbol = scope_.FindVarSymbolInCurrentEnv(name);
    if (symbol) {
        llvm::errs() << "Try to redefine variable: " << name << "\n";
        return nullptr;
    }

    // 2. Add the symbol name to symbol table.
    scope_.AddSymbol(name, SymbolKind::kLocalVariable, ctype);

    // 3. Allocate the variable declare node object.
    auto node = std::make_shared<VariableDecl>();
    node->SetName(name);
    node->SetCType(ctype);
    return node;
}

std::shared_ptr<AstNode> Sema::SemaAssignExprNode(
        std::shared_ptr<AstNode> left, 
        std::shared_ptr<AstNode> right) {
    if (!left || !right) {
        llvm::errs() << "The left or right child of assign expression is null!\n";
        return nullptr;
    }

    if (!llvm::isa<VariableAccessExpr>(left.get())) {
        llvm::errs() << "The left child of assign expression isn't access expression\n";
        return nullptr;
    }

    auto assign_expr = std::make_shared<AssignExpr>();
    assign_expr->left_ = left;
    assign_expr->right_ = right;

    return assign_expr;
}

std::shared_ptr<AstNode> Sema::SemaVariableAccessNode(const llvm::StringRef& name) {
    auto symbol = scope_.FindVarSymbol(name);
    if (!symbol) {
        llvm::errs() << "Try to access undefined symbol: " << name << "\n";
        return nullptr;
    }

    auto variable_access_node = std::make_shared<VariableAccessExpr>();
    variable_access_node->SetCType(symbol->GetCType());
    variable_access_node->SetName(name);
    return variable_access_node;
}

std::shared_ptr<AstNode> Sema::SemaBinaryExprNode(
        std::shared_ptr<AstNode> left, 
        std::shared_ptr<AstNode> right, 
        OpCode op) {
    if (!left || !right) {
        llvm::errs() << "The left or right child of binary expression is null!\n";
        return nullptr;
    }

    auto expr = std::make_shared<BinaryExpr>();
    expr->left_ = left;
    expr->right_ = right;
    expr->op_ = op;
    return expr;
}

std::shared_ptr<AstNode> Sema::SemaNumberExprNode(int number, CType* ctype) {
    auto expr = std::make_shared<NumberExpr>();
    expr->number_ = number;
    expr->SetCType(ctype);
    return expr;
}
