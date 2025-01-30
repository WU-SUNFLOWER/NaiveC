// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef LEXER_H_
#define LEXER_H_

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include "type.h"

enum class TokenType {
    kNumber,
    kPlus,          // '+'
    kMinus,         // '-'
    kStar,          // '*'
    kSlash,         // '/'
    kLParent,       // '('
    kRParent,       // ')'
    kSemi,          // ';'

    kIdentifier,    // (a-zA-Z_)(a-zA-Z0-9_)*
    kEqual,         // '='
    kComma,         // ','
    kInt,           // 'int'

    kEOF,           // The end of file
    kUnknown,       // Unknown token
};

class Token;
class Lexer;

class Token {
 private:
    int row_, col_;
    TokenType type_;
    int value_;
    llvm::StringRef content_;
    CType* ctype_;

 public:
    friend class Lexer;

    Token() 
        : row_(-1), col_(-1), type_(TokenType::kUnknown), value_(-1), ctype_(nullptr) {}

    TokenType GetType() const {
        return type_;
    }

    CType* GetCType() const {
        return ctype_;
    }

    int GetValue() const {
        return value_;
    }

    const llvm::StringRef& GetContent() const {
        return content_;
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
