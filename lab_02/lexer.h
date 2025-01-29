// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef LEXER_H_
#define LEXER_H_

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

enum class TokenType {
    kNumber,
    kPlus,      // '+'
    kMinus,     // '-'
    kStar,      // '*'
    kSlash,     // '/'
    kLParent,   // '('
    kRParent,   // ')'
    kSemi,      // ';'

    kEOF,       // The end of file
    kUnknown,   // Unknown token
};

class Token;
class Lexer;

class Token {
 private:
    int row_, col_;
    TokenType type_;
    int value_;
    llvm::StringRef content_;

 public:
    friend class Lexer;

    Token() : row_(-1), col_(-1), type_(TokenType::kUnknown), value_(-1) {}

    TokenType GetType() const {
        return type_;
    }

    void Dump() {
        llvm::outs() << "{ " << content_ 
                     << " | " 
                     << static_cast<int>(type_) 
                     << " | row=" 
                     << row_
                     << ", col=" 
                     << col_ << "}\n";
    }
};

class Lexer {
 private:
    const char* buf_;
    const char* line_head_;
    const char* buf_end_;

    int row_;

 public:
    explicit Lexer(llvm::StringRef source_code);

    void GetNextToken(Token&);
};

#endif  // LEXER_H_
