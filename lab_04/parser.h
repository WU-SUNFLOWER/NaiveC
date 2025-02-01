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

 public:
    explicit Parser(Lexer& lexer, Sema& sema);

    std::shared_ptr<Program> ParserProgram();

 private:
    std::vector<std::shared_ptr<AstNode>> ParseDeclStmt();
    std::shared_ptr<AstNode> ParseExprStmt();
    std::shared_ptr<AstNode> ParseAssignExpr();
    std::shared_ptr<AstNode> ParseExpr();
    std::shared_ptr<AstNode> ParseFactor();
    std::shared_ptr<AstNode> ParseTerm();

    bool Expect(TokenType token_type);
    bool Consume(TokenType token_type);
    void Advance();

    DiagEngine& GetDiagEngine() const {
      return lexer_.GetDiagEngine();
    }
};

#endif  // PARSER_H_
