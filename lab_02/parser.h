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

 public:
    explicit Parser(Lexer& lexer) : lexer_(lexer) {}

    std::shared_ptr<Program> ParserProgram();

 private:
    std::shared_ptr<Expression> ParserFactor();
    std::shared_ptr<Expression> ParseTerm();
};

#endif  // PARSER_H_
