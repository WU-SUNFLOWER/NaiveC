// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef PARSER_H_
#define PARSER_H_

#include <memory>
#include <vector>

#include "lexer.h"
#include "ast.h"
#include "sema.h"

class Parser {
 private:
    Lexer& lexer_;
    Sema& sema_;
    Token token_ {};

    std::vector<std::shared_ptr<AstNode>> breaked_able_nodes_;
    std::vector<std::shared_ptr<AstNode>> continued_able_nodes_;

    void AddBreakedAbleNode(std::shared_ptr<AstNode> node) {
        breaked_able_nodes_.emplace_back(node);
    }

    void AddContinuedAbleNode(std::shared_ptr<AstNode> node) {
        continued_able_nodes_.emplace_back(node);
    }

    void RemoveBreakedAbleNode(std::shared_ptr<AstNode> node) {
        assert(!breaked_able_nodes_.empty() && breaked_able_nodes_.back() == node);
        breaked_able_nodes_.pop_back();
    }

    void RemoveContinuedAbleNode(std::shared_ptr<AstNode> node) {
        assert(!continued_able_nodes_.empty() && continued_able_nodes_.back() == node);
        continued_able_nodes_.pop_back();
    }

 public:
    explicit Parser(Lexer& lexer, Sema& sema);

    std::shared_ptr<Program> ParserProgram();

 private:
    std::shared_ptr<AstNode> ParseStmt();
    std::shared_ptr<AstNode> ParseDeclStmt();
    std::shared_ptr<AstNode> ParseExprStmt();
    std::shared_ptr<AstNode> ParseIfStmt();
    std::shared_ptr<AstNode> ParseBlockStmt();
    std::shared_ptr<AstNode> ParseForStmt();
    std::shared_ptr<AstNode> ParseBreakStmt();
    std::shared_ptr<AstNode> ParseContinueStmt();
    
    std::shared_ptr<AstNode> ParseExpr();
    std::shared_ptr<AstNode> ParseAssignExpr();
    std::shared_ptr<AstNode> ParseEqualExpr();
    std::shared_ptr<AstNode> ParseRelationalExpr();

    std::shared_ptr<AstNode> ParseAddExpr();
    std::shared_ptr<AstNode> ParseMultiExpr();

    std::shared_ptr<AstNode> ParsePrimaryExpr();

    bool Expect(TokenType token_type);
    bool Consume(TokenType token_type);
    void Advance();

    DiagEngine& GetDiagEngine() const {
      return lexer_.GetDiagEngine();
    }

    bool CurrentTokenIsTypeName() const;
};

#endif  // PARSER_H_
