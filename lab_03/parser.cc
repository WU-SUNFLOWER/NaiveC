// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "parser.h"

#include <cassert>
#include <vector>
#include <memory>
#include <utility>

Parser::Parser(Lexer& lexer, Sema& sema) : lexer_(lexer), sema_(sema) {
    Advance();
}

std::shared_ptr<Program> Parser::ParserProgram() {
    std::vector<std::shared_ptr<AstNode>> exprs;
    while (token_.GetType() != TokenType::kEOF) {
        if (token_.GetType() == TokenType::kSemi) {
            Advance();
            continue;
        }

        if (token_.GetType() == TokenType::kInt) {
            auto decl_exprs = std::move(ParseDecl());
            for (auto& decl_expr : decl_exprs) {
                exprs.push_back(decl_expr);
            }
        } else {
            auto expression = ParserExpression();
            exprs.push_back(expression);
        }
    }

    auto program = std::make_shared<Program>();
    program->expr_vec_ = std::move(exprs);

    return program;
}

std::vector<std::shared_ptr<AstNode>> Parser::ParseDecl() {
    // Step 1. int x, y = 1, z = 2;
    Consume(TokenType::kInt);

    CType* variable_ctype = CType::GetIntType();
    std::vector<std::shared_ptr<AstNode>> ast_vec;
    // Step 2. x, y = 1, z = 2;
    while (token_.GetType() != TokenType::kSemi) {
        auto variable_name = token_.GetContent();
        auto variable_decl_node = sema_.SemaVariableDeclNode(variable_name, variable_ctype);
        ast_vec.push_back(variable_decl_node);

        Consume(TokenType::kIdentifier);

        if (token_.GetType() == TokenType::kEqual) {
            Advance();

            auto access_expr = sema_.SemaVariableAccessNode(variable_name);
            auto value_expr = ParserExpression();

            auto assign_expr = sema_.SemaAssignExprNode(access_expr, value_expr);
            ast_vec.push_back(assign_expr);
        }

        if (token_.GetType() == TokenType::kComma) {
            Advance();
        }
    }

    // Step 3. ;
    Consume(TokenType::kSemi);

    return ast_vec;
}

std::shared_ptr<AstNode> Parser::ParserExpression() {
    auto left_expr = ParseTerm();
    while (token_.GetType() == TokenType::kPlus 
           || token_.GetType() == TokenType::kMinus) {
        OpCode op;
        if (token_.GetType() == TokenType::kPlus) {
            op = OpCode::kAdd;
        } else {
            op = OpCode::kSub;
        }

        Advance();

        auto right_expr = ParseTerm();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseTerm() {
    auto left_expr = ParserFactor();
    while (token_.GetType() == TokenType::kStar 
           || token_.GetType() == TokenType::kSlash) {
        OpCode op;
        if (token_.GetType() == TokenType::kStar) {
            op = OpCode::kMul;
        } else {
            op = OpCode::kDiv;
        }

        Advance();

        auto right_expr = ParserFactor();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParserFactor() {
    if (token_.GetType() == TokenType::kLParent) {
        Advance();
        auto sub_expr = ParserExpression();
        assert(Expect(TokenType::kRParent));
        Advance();
        return sub_expr;
    }

    if (token_.GetType() == TokenType::kIdentifier) {
        auto access_expr = sema_.SemaVariableAccessNode(token_.GetContent());
        Advance();
        return access_expr;
    }

    auto number_expr = sema_.SemaNumberExprNode(token_.GetValue(), token_.GetCType());
    Advance();

    return number_expr;
}

bool Parser::Expect(TokenType token_type) {
    return token_.GetType() == token_type;
}

bool Parser::Consume(TokenType token_type) {
    assert(Expect(token_type));
    if (Expect(token_type)) {
        Advance();
        return true;
    }
    return false;
}

void Parser::Advance() {
    lexer_.GetNextToken(token_);
}
