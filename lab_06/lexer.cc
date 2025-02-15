// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "lexer.h"

#include <cstring>
#include <iostream>

bool IsWhiteSpace(char ch) {
    return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t';
}

bool IsDigit(char ch) {
    return '0' <= ch && ch <= '9';
}

bool IsLetter(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
}

Lexer::Lexer(llvm::SourceMgr& mgr, DiagEngine& diag_engine)
    : mgr_(mgr), diag_engine_(diag_engine), row_(1) {
    auto id = mgr_.getMainFileID();
    auto wrapped_buf = mgr_.getMemoryBuffer(id)->getBuffer();

    line_head_ = wrapped_buf.begin();
    buf_ = wrapped_buf.begin();
    buf_end_ = wrapped_buf.end();
}

void Lexer::SaveState() {
    state_.buf = buf_;
    state_.buf_end = buf_end_;
    state_.line_head = line_head_;
    state_.row = row_;
}

void Lexer::RestoreState() {
    buf_ = state_.buf;
    buf_end_ = state_.buf_end;
    line_head_ = state_.line_head;
    row_ = state_.row;
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

    // 2. Have we reached the end of file?
    if (buf_ >= buf_end_) {
        token.type_ = TokenType::kEOF;
        return;
    }

    // 3. Now we meet the next legal token
    //    Let's record its position information.
    token.row_ = row_;
    token.col_ = buf_ - line_head_ + 1;

    const char* start = buf_;
    if (IsDigit(*buf_)) {
        int number = 0;
        while (IsDigit(*buf_)) {
            number = number * 10 + (*buf_ - '0');
            ++buf_;
        }

        token.type_ = TokenType::kNumber;
        token.value_ = number;
        token.ctype_ = CType::GetIntType();
        token.content_ptr_ = start;
        token.content_length_ = buf_ - start;
    }
    else if (IsLetter(*buf_)) {
        while (IsLetter(*buf_) || IsDigit(*buf_)) {
            ++buf_;
        }

        token.type_ = TokenType::kIdentifier;
        token.content_ptr_ = start;
        token.content_length_ = buf_ - start;

#define IS_KEYWORD(kw) (strncmp(token.content_ptr_, kw, token.content_length_) == 0)
        if (IS_KEYWORD("int")) {
            token.type_ = TokenType::kInt;
        }
        else if (IS_KEYWORD("if")) {
            token.type_ = TokenType::kIf;
        }
        else if (IS_KEYWORD("else")) {
            token.type_ = TokenType::kElse;
        }
#undef IS_KEYWORD
    }
    else {
        switch (*buf_) {
            case '+':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kPlus;
                break;
            case '-':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kMinus;
                break;
            case '*':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kStar;
                break;
            case '/':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kSlash;
                break;
            case '(':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kLParent;
                break;
            case ')':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kRParent;
                break;
            case '{':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kLBrace;
                break;
            case '}':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kRBrace;
                break;
            case ';':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kSemi;
                break;
            case ',':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kComma;
                break;
            case '=': {
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kEqualEqual;
                } else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kEqual;
                }
                break;
            }
            case '!': {
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kNotEqual;
                    break;
                }
                // If not match, the compiler will throw an error...
            }
            case '<': {
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kLessEqual;
                } else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kLess;
                }
                break;
            }
            case '>': {
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kGreaterEqual;
                } else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kGreater;
                }
                break;
            }
            default:
                diag_engine_.Report(llvm::SMLoc::getFromPointer(buf_), Diag::kErrUnknownChar, *buf_);
        }
        /*
        ++buf_;
        token.content_ptr_ = start;
        token.content_length_ = 1;        
        */
    }
}

llvm::StringRef Token::GetSpellingText(TokenType token_type) {
    switch (token_type) {
        case TokenType::kNumber:
            return "Number";
        case TokenType::kPlus:
            return "+";
        case TokenType::kMinus:
            return "-";
        case TokenType::kStar:
            return "*";
        case TokenType::kSlash:
            return "/";
        case TokenType::kLParent:
            return "(";
        case TokenType::kRParent:
            return ")";
        case TokenType::kLBrace:
            return "{";
        case TokenType::kRBrace:
            return "}";
        case TokenType::kEqualEqual:
            return "==";
        case TokenType::kNotEqual:
            return "!=";
        case TokenType::kLess:
            return "<";
        case TokenType::kLessEqual:
            return "<=";
        case TokenType::kGreater:
            return ">";
        case TokenType::kGreaterEqual:
            return ">=";
        case TokenType::kSemi:
            return ";";
        case TokenType::kIdentifier:
            return "Identifier";
        case TokenType::kEqual:
            return "=";
        case TokenType::kComma:
            return ",";
        case TokenType::kInt:
            return "int";
        case TokenType::kIf:
            return "if";
        case TokenType::kElse:
            return "else";
        case TokenType::kEOF:
            return "EOF";
        default:
            break;
    }
    llvm::llvm_unreachable_internal();
}
