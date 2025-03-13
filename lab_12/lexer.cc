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

    file_name_ = mgr_.getMemoryBuffer(id)->getBufferIdentifier();
}

bool Lexer::BufferStartWith(const char *target) {
    auto len = strlen(target);
    return (buf_ <= buf_end_ - len) && (strncmp(buf_, target, strlen(target)) == 0);
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
    // 1. Filter the white space and comment.
    while (buf_ < buf_end_ && (IsWhiteSpace(*buf_) || BufferStartWith("//") || BufferStartWith("/*"))) {
        // Remove single line comment.
        if (BufferStartWith("//")) {
            while (buf_ < buf_end_&& *buf_ != '\n') ++buf_;
        }
        // Remove multiline comment.
        if (BufferStartWith("/*")) {
            buf_ += 2;
            while (buf_ < buf_end_ && !BufferStartWith("*/")) {
                if (*buf_ == '\n') {
                    ++row_;
                    line_head_ = buf_ + 1;
                }
                ++buf_;
            }
            buf_ += 2;
        }
        // Remove '\n'
        if (buf_ < buf_end_ && *buf_ == '\n') {
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
        token.ctype_ = CType::kIntType;
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

        llvm::StringRef identifier = llvm::StringRef(token.content_ptr_, token.content_length_);
        
#define IS_KEYWORD(kw) (identifier == kw)
        if (IS_KEYWORD("int")) {
            token.type_ = TokenType::kInt;
        }
        else if (IS_KEYWORD("if")) {
            token.type_ = TokenType::kIf;
        }
        else if (IS_KEYWORD("else")) {
            token.type_ = TokenType::kElse;
        }
        else if (IS_KEYWORD("for")) {
            token.type_ = TokenType::kFor;
        }
        else if (IS_KEYWORD("break")) {
            token.type_ = TokenType::kBreak;
        }
        else if (IS_KEYWORD("continue")) {
            token.type_ = TokenType::kContinue;
        }
        else if (IS_KEYWORD("sizeof")) {
            token.type_ = TokenType::kSizeof;
        }
        else if (IS_KEYWORD("struct")) {
            token.type_ = TokenType::kStruct;
        }
        else if (IS_KEYWORD("union")) {
            token.type_ = TokenType::kUnion;
        }
        else if (IS_KEYWORD("return")) {
            token.type_ = TokenType::kReturn;
        }
        else if (IS_KEYWORD("void")) {
            token.type_ = TokenType::kVoid;
        }
#undef IS_KEYWORD
    }
    else {
        switch (*buf_) {
            case '+':
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kPlusEqual;
                }
                else if (*(buf_ + 1) == '+') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kPlusPlus;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kPlus;
                }
                break;
            case '-':
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kMinusEqual;
                }
                else if (*(buf_ + 1) == '-') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kMinusMinus;
                }
                else if (*(buf_ + 1) == '>') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kArrow;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kMinus;
                }
                break;
            case '*':
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kStarEqual;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kStar;
                }
                break;
            case '/':
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kSlashEqual;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kSlash;
                }
                break;
            case '%':
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kPercentEqual;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kPercent;
                }
                break;
            case '^':
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kCaretEqual;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kCaret;
                }
                break;
            case '~':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kTilde;
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
            case '.':
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kDot;
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
                } else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kNot;
                }
                break;
            }
            case '<': {
                if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kLessEqual;
                } 
                else if (*(buf_ + 1) == '<') {
                    if (*(buf_ + 2) == '=') {
                        buf_ += 3;
                        token.content_ptr_ = start;
                        token.content_length_ = 3;
                        token.type_ = TokenType::kLessLessEqual;    
                    } else {
                        buf_ += 2;
                        token.content_ptr_ = start;
                        token.content_length_ = 2;
                        token.type_ = TokenType::kLessLess;                        
                    }
                }
                else {
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
                }
                else if (*(buf_ + 1) == '>') {
                    if (*(buf_ + 2) == '=') {
                        buf_ += 3;
                        token.content_ptr_ = start;
                        token.content_length_ = 3;
                        token.type_ = TokenType::kGreaterGreaterEqual;    
                    } else {
                        buf_ += 2;
                        token.content_ptr_ = start;
                        token.content_length_ = 2;
                        token.type_ = TokenType::kGreaterGreater;                        
                    }
                } 
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kGreater;
                }
                break;
            }
            case '|': {
                if (*(buf_ + 1) == '|') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kPipePipe;
                }
                else if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kPipeEqual;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kPipe;
                }
                break;
            }
            case '&': {
                if (*(buf_ + 1) == '&') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kAmpAmp;
                }
                else if (*(buf_ + 1) == '=') {
                    buf_ += 2;
                    token.content_ptr_ = start;
                    token.content_length_ = 2;
                    token.type_ = TokenType::kAmpEqual;
                }
                else {
                    ++buf_;
                    token.content_ptr_ = start;
                    token.content_length_ = 1;
                    token.type_ = TokenType::kAmp;
                }
                break;
            }
            case '?': {
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kQuestion;
                break;
            }
            case ':': {
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kColon;
                break;
            }
            case '[': {
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kLBracket;
                break;
            }
            case ']': {
                ++buf_;
                token.content_ptr_ = start;
                token.content_length_ = 1;
                token.type_ = TokenType::kRBracket;
                break;
            }
            default:
                diag_engine_.Report(llvm::SMLoc::getFromPointer(buf_), Diag::kErrUnknownChar, *buf_);
        }
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
        case TokenType::kPipePipe:
            return "||";
        case TokenType::kPipe:
            return "|";
        case TokenType::kAmpAmp:
            return "&&";
        case TokenType::kAmp:
            return "&";
        case TokenType::kPercent:
            return "%";
        case TokenType::kSemi:
            return ";";
        case TokenType::kCaret:
            return "^";
        case TokenType::kPlusPlus:
            return "++";
        case TokenType::kMinusMinus:
            return "--";
        case TokenType::kTilde:
            return "~";
        case TokenType::kPlusEqual:
            return "+=";
        case TokenType::kMinusEqual:
            return "-=";
        case TokenType::kStarEqual:
            return "*=";
        case TokenType::kSlashEqual:
            return "/=";
        case TokenType::kPercentEqual:
            return "%=";
        case TokenType::kLessLessEqual:
            return "<<=";
        case TokenType::kGreaterGreaterEqual:
            return ">>=";
        case TokenType::kAmpEqual:
            return "&=";
        case TokenType::kCaretEqual:
            return "^=";
        case TokenType::kPipeEqual:
            return "|=";
        case TokenType::kQuestion:
            return "?";
        case TokenType::kColon:
            return ":";
        case TokenType::kIdentifier:
            return "Identifier";
        case TokenType::kEqual:
            return "=";
        case TokenType::kComma:
            return ",";
        case TokenType::kDot:
            return ".";
        case TokenType::kInt:
            return "int";
        case TokenType::kIf:
            return "if";
        case TokenType::kElse:
            return "else";
        case TokenType::kFor:
            return "for";
        case TokenType::kBreak:
            return "break";
        case TokenType::kContinue:
            return "continue";
        case TokenType::kSizeof:
            return "sizeof";
        case TokenType::kStruct:
            return "struct";
        case TokenType::kUnion:
            return "union";
        case TokenType::kReturn:
            return "return";
        case TokenType::kVoid:
            return "void";
        case TokenType::kLBracket:
            return "[";
        case TokenType::kRBracket:
            return "]";
        case TokenType::kArrow:
            return "->";
        case TokenType::kEOF:
            return "EOF";
        default:
            break;
    }
    llvm::llvm_unreachable_internal();
}
