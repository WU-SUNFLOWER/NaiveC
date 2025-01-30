// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef PARSER_H_
#define PARSER_H_

#include <memory>

#include "lexer.h"
#include "ast.h"

class Parser {
 private:
    Lexer& lexer_;
    Token token_;

 public:
    explicit Parser(Lexer& lexer);

    std::shared_ptr<Program> ParserProgram();

 private:
    std::shared_ptr<Expression> ParserExpression();
    std::shared_ptr<Expression> ParserFactor();
    std::shared_ptr<Expression> ParseTerm();

    bool Expect(TokenType token_type);
    bool Consume(TokenType token_type);
    void Advance();
};

#endif  // PARSER_H_
