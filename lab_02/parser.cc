// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "parser.h"

#include <cassert>
#include <vector>
#include <memory>
#include <utility>

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    Advance();
}

std::shared_ptr<Program> Parser::ParserProgram() {
    std::vector<std::shared_ptr<Expression>> exprs;
    while (token_.GetType() != TokenType::kEOF) {
        if (token_.GetType() == TokenType::kSemi) {
            Advance();
            continue;
        }
        auto expression = ParserExpression();
        exprs.push_back(expression);
    }

    auto program = std::make_shared<Program>();
    program->expr_vec_ = std::move(exprs);

    return program;
}

std::shared_ptr<Expression> Parser::ParserExpression() {
    auto left_term = ParseTerm();
    while (token_.GetType() == TokenType::kPlus 
           || token_.GetType() == TokenType::kMinus) {
        OpCode op;
        if (token_.GetType() == TokenType::kPlus) {
            op = OpCode::kAdd;
        } else {
            op = OpCode::kSub;
        }

        Advance();

        auto binary_expr = std::make_shared<BinaryExpression>();
        binary_expr->op_ = op;
        binary_expr->left_ = left_term;
        binary_expr->right_ = ParseTerm();

        left_term = binary_expr;
    }
    return left_term;
}

std::shared_ptr<Expression> Parser::ParseTerm() {
    auto left_factor = ParserFactor();
    while (token_.GetType() == TokenType::kStar 
           || token_.GetType() == TokenType::kSlash) {
        OpCode op;
        if (token_.GetType() == TokenType::kStar) {
            op = OpCode::kMul;
        } else {
            op = OpCode::kDiv;
        }

        Advance();

        auto binary_expr = std::make_shared<BinaryExpression>();
        binary_expr->op_ = op;
        binary_expr->left_ = left_factor;
        binary_expr->right_ = ParserFactor();

        left_factor = binary_expr;
    }
    return left_factor;
}

std::shared_ptr<Expression> Parser::ParserFactor() {
    if (token_.GetType() == TokenType::kLParent) {
        Advance();
        auto sub_expr = ParserExpression();
        assert(Expect(TokenType::kRParent));
        Advance();
        return sub_expr;
    }

    auto factor = std::make_shared<FactorExpression>();
    factor->number_ = token_.GetValue();
    Advance();

    return factor;
}

bool Parser::Expect(TokenType token_type) {
    return token_.GetType() == token_type;
}

bool Parser::Consume(TokenType token_type) {
    if (Expect(token_type)) {
        Advance();
        return true;
    }
    return false;
}

void Parser::Advance() {
    lexer_.GetNextToken(token_);
}
