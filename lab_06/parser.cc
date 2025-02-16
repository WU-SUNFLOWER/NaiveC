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
    std::vector<std::shared_ptr<AstNode>> nodes;

    while (token_.GetType() != TokenType::kEOF) {
        auto stmt = ParseStmt();
        if (stmt) {
            nodes.emplace_back(stmt);
        }
    }

    auto program = std::make_shared<Program>();
    program->nodes_ = std::move(nodes);

    return program;
}


std::shared_ptr<AstNode> Parser::ParseStmt() {
#define TOKEN_TYPE_IS(type) (token_.GetType() == type)

    if (TOKEN_TYPE_IS(TokenType::kSemi)) {
        Advance();
        return nullptr;
    }
    else if (CurrentTokenIsTypeName()) {
        return ParseDeclStmt();
    } 
    else if (TOKEN_TYPE_IS(TokenType::kIf)) {
        return ParseIfStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kLBrace)) {
        return ParseBlockStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kFor)) {
        return ParseForStmt();
    }

#undef TOKEN_TYPE_IS

    return ParseExprStmt();
}


std::shared_ptr<AstNode> Parser::ParseDeclStmt() {
    auto decl_stmt = std::make_shared<DeclStmt>();

    // Step 1. int x, y = 1, z = 2;
    Consume(TokenType::kInt);

    CType* variable_ctype = CType::GetIntType();
    // Step 2. x, y = 1, z = 2;
    while (token_.GetType() != TokenType::kSemi) {
        Token variable_token = token_;
        auto variable_name = token_.GetContent();
        auto variable_decl_node = sema_.SemaVariableDeclNode(variable_token, variable_ctype);

        decl_stmt->nodes_.emplace_back(variable_decl_node);

        Consume(TokenType::kIdentifier);

        if (token_.GetType() == TokenType::kEqual) {
            Token equal_op_token = token_;

            Advance();

            auto access_expr = sema_.SemaVariableAccessNode(variable_token);
            auto value_expr = ParseExpr();

            auto assign_expr = sema_.SemaAssignExprNode(equal_op_token, access_expr, value_expr);
            decl_stmt->nodes_.emplace_back(assign_expr);
        }

        if (token_.GetType() == TokenType::kComma) {
            Advance();
        }
    }

    // Step 3. ;
    Consume(TokenType::kSemi);

    return decl_stmt;
}

std::shared_ptr<AstNode> Parser::ParseIfStmt() {
    Consume(TokenType::kIf);

    Consume(TokenType::kLParent);
    auto cond_expr = ParseExpr();
    Consume(TokenType::kRParent);

    auto then_stmt = ParseStmt();

    std::shared_ptr<AstNode> else_stmt = nullptr;
    if (token_.GetType() == TokenType::kElse) {
        Consume(TokenType::kElse);
        else_stmt = ParseStmt();
    }

    return sema_.SemaIfStmtNode(cond_expr, then_stmt, else_stmt);
}

std::shared_ptr<AstNode> Parser::ParseBlockStmt() {
    auto block_stmt = std::make_shared<BlockStmt>();

    Consume(TokenType::kLBrace);
    sema_.EnterScope();
    
    while (token_.GetType() != TokenType::kRBrace) {
        block_stmt->nodes_.emplace_back(ParseStmt());
    }
    
    sema_.ExitScope();
    Consume(TokenType::kRBrace);

    return block_stmt;
}

std::shared_ptr<AstNode> Parser::ParseForStmt() {
    Consume(TokenType::kFor);
    Consume(TokenType::kLParent);

    // For statement has its own scope.
    sema_.EnterScope();

    std::shared_ptr<AstNode> init_node = nullptr;
    std::shared_ptr<AstNode> cond_node = nullptr;
    std::shared_ptr<AstNode> inc_node = nullptr;
    std::shared_ptr<AstNode> body_node = nullptr;

    // Build all the sub nodes of forstmt node.
    if (CurrentTokenIsTypeName()) {
        init_node = ParseDeclStmt();
    } else {
        init_node = ParseExprStmt();
    }

    cond_node = ParseExprStmt();

    if (token_.GetType() != TokenType::kRParent) {
        inc_node = ParseExpr();
    }
    Consume(TokenType::kRParent);

    body_node = ParseStmt();

    // Build forstmt node.
    auto for_stmt_node = std::make_shared<ForStmt>();
    for_stmt_node->init_node_ = init_node;
    for_stmt_node->cond_node_ = cond_node;
    for_stmt_node->inc_node_ = inc_node;
    for_stmt_node->body_node_ = body_node;

    // Don't forget to exit the scope of for statement!
    sema_.ExitScope();

    return for_stmt_node;
}

std::shared_ptr<AstNode> Parser::ParseExprStmt() {
    if (token_.GetType() == TokenType::kSemi) {
        Advance();
        return nullptr;
    }

    auto expr = ParseExpr();
    Consume(TokenType::kSemi);
    return expr;
}

std::shared_ptr<AstNode> Parser::ParseExpr() {
    bool is_assign_expr = false;

    lexer_.SaveState();
    {
        if (token_.GetType() == TokenType::kIdentifier) {
            Token tmp;
            lexer_.GetNextToken(tmp);
            if (tmp.GetType() == TokenType::kEqual) {
                is_assign_expr = true;
            }
        }
    }
    lexer_.RestoreState();

    // Process "=" assignment expression.
    if (is_assign_expr) {
        return ParseAssignExpr();
    }

    return ParseEqualExpr();
}

std::shared_ptr<AstNode> Parser::ParseAssignExpr() {
    auto access_node = sema_.SemaVariableAccessNode(token_);
    Consume(TokenType::kIdentifier);

    Token equal_op_token = token_;
    Consume(TokenType::kEqual);

    auto right_node = ParseExpr();
    return sema_.SemaAssignExprNode(equal_op_token, access_node, right_node);
}

// Process "==" or "!=" expression.
std::shared_ptr<AstNode> Parser::ParseEqualExpr() {
    auto left_expr = ParseRelationalExpr();
    while (token_.GetType() == TokenType::kEqualEqual 
           || token_.GetType() == TokenType::kNotEqual) {
        OpCode op;
        if (token_.GetType() == TokenType::kEqualEqual) {
            op = OpCode::kEqualEqual;
        } else {
            op = OpCode::kNotEqual;
        }

        Advance();

        auto right_expr = ParseRelationalExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }

    return left_expr;
}

// Process "<", ">", "<=" or ">=" expression.
std::shared_ptr<AstNode> Parser::ParseRelationalExpr() {
    auto left_expr = ParseAddExpr();

#define TOKEN_TYPE_IS(type) (token_.GetType() == type)

    while (TOKEN_TYPE_IS(TokenType::kLess) 
           || TOKEN_TYPE_IS(TokenType::kGreater)
           || TOKEN_TYPE_IS(TokenType::kLessEqual)
           || TOKEN_TYPE_IS(TokenType::kGreaterEqual))
    {
        OpCode op;
        switch (token_.GetType()) {
            case TokenType::kLess:
                op = OpCode::kLess;
                break;
            case TokenType::kGreater:
                op = OpCode::kGreater;
                break;
            case TokenType::kLessEqual:
                op = OpCode::kLessEqual;
                break;
            case TokenType::kGreaterEqual:
                op = OpCode::kGreaterEqual;
                break;
            default:
                assert(0);
        }

        Advance();

        auto right_expr = ParseAddExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }

    return left_expr;

#undef TOKEN_TYPE_IS
}

// Process "+" or "-" expression.
std::shared_ptr<AstNode> Parser::ParseAddExpr() {
    auto left_expr = ParseMultiExpr();
    while (token_.GetType() == TokenType::kPlus 
           || token_.GetType() == TokenType::kMinus) {
        OpCode op;
        if (token_.GetType() == TokenType::kPlus) {
            op = OpCode::kAdd;
        } else {
            op = OpCode::kSub;
        }

        Advance();

        auto right_expr = ParseMultiExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }

    return left_expr;
}

// Process "*" or "/" expression.
std::shared_ptr<AstNode> Parser::ParseMultiExpr() {
    auto left_expr = ParsePrimaryExpr();
    while (token_.GetType() == TokenType::kStar 
           || token_.GetType() == TokenType::kSlash) {
        OpCode op;
        if (token_.GetType() == TokenType::kStar) {
            op = OpCode::kMul;
        } else {
            op = OpCode::kDiv;
        }

        Advance();

        auto right_expr = ParsePrimaryExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParsePrimaryExpr() {
    if (token_.GetType() == TokenType::kLParent) {
        Consume(TokenType::kLParent);
        auto sub_expr = ParseExpr();
        Consume(TokenType::kRParent);
        return sub_expr;
    }

    if (token_.GetType() == TokenType::kIdentifier) {
        auto access_expr = sema_.SemaVariableAccessNode(token_);
        Advance();
        return access_expr;
    }
    
    Expect(TokenType::kNumber);
    auto number_expr = sema_.SemaNumberExprNode(token_, token_.GetCType());
    Advance();

    return number_expr;
}

bool Parser::Expect(TokenType token_type) {
    if (token_.GetType() == token_type) {
        return true;
    }
    GetDiagEngine().Report(
            llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()),
            Diag::kErrExpected,
            Token::GetSpellingText(token_type),
            token_.GetContent());
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

bool Parser::CurrentTokenIsTypeName() const {
    if (token_.GetType() == TokenType::kInt) {
        return true;
    }
    return false;
}
