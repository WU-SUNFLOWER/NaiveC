// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "lexer.h"

#include <iostream>

bool IsWhiteSpace(char ch) {
    return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t';
}

bool IsDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

Lexer::Lexer(llvm::StringRef source_code) {
    line_head_ = source_code.begin();
    buf_ = source_code.begin();
    buf_end_ = source_code.end();
    row_ = 1;
}

void Lexer::GetNextToken(Token& token) {
    // 1. Filter the white space.
    while (IsWhiteSpace(*buf_)) {
        if (*buf_ == '\n') {
            ++row_;
            line_head_ = buf_ + 1;
        }
        ++buf_;
    }
    token.row_ = row_;
    token.col_ = buf_ - line_head_ + 1;

    // 2. Have we reached the end of file?
    if (buf_ >= buf_end_) {
        token.type_ = TokenType::kEOF;
        token.content_ = llvm::StringRef("EOF");
        return;
    }

    const char* start = buf_;
    if (IsDigit(*buf_)) {
        int number = 0;
        int len = 0;
        while (IsDigit(*buf_)) {
            number = number * 10 + (*buf_ - '0');
            ++buf_;
            ++len;
        }
        token.type_ = TokenType::kNumber;
        token.value_ = number;
        token.content_ = llvm::StringRef(start, len);
    } else {
        switch (*buf_) {
            case '+':
                token.type_ = TokenType::kPlus;
                break;
            case '-':
                token.type_ = TokenType::kMinus;
                break;
            case '*':
                token.type_ = TokenType::kStar;
                break;
            case '/':
                token.type_ = TokenType::kSlash;
                break;
            case '(':
                token.type_ = TokenType::kLParent;
                break;
            case ')':
                token.type_ = TokenType::kRParent;
                break;
            case ';':
                token.type_ = TokenType::kSemi;
                break;
            default:
                token.type_ = TokenType::kUnknown;
                std::cerr << "Invalid Token: " << *buf_ << std::endl;
        }
        ++buf_;
        token.content_ = llvm::StringRef(start, 1);
    }
}
