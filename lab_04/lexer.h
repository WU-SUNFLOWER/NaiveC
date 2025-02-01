// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef LEXER_H_
#define LEXER_H_

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include "type.h"
#include "diag-engine.h"

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
};

class Token;
class Lexer;

class Token {
 private:
    int row_ { -1 };
    int col_ { -1 };
    TokenType type_ { TokenType::kEOF };
    
    int value_ { -1 };  // For number token.

    const char* content_ptr_ { nullptr };  // For debug && diag.
    size_t content_length_ { 0 };

    CType* ctype_ { nullptr };  // For built-in type.

 public:
    friend class Lexer;

    Token() = default;

    Token(const Token& other) = default;

    TokenType GetType() const {
        return type_;
    }

    CType* GetCType() const {
        return ctype_;
    }

    int GetValue() const {
        return value_;
    }

    llvm::StringRef GetContent() const {
        return llvm::StringRef(content_ptr_, content_length_);
    }

    const char* GetRawContentPtr() const {
        return content_ptr_;
    } 

    void Dump() {
        llvm::outs() << "{ " << llvm::StringRef(content_ptr_, content_length_) 
                     << " | " 
                     << static_cast<int>(type_) 
                     << " | row=" 
                     << row_
                     << ", col=" 
                     << col_ << "}\n";
    }

    static llvm::StringRef GetSpellingText(TokenType token_type);
};

class Lexer {
 private:
    const char* buf_;
    const char* line_head_;
    const char* buf_end_;
    int row_;

    struct State {
        const char* buf;
        const char* line_head;
        const char* buf_end;
        int row;
    } state_;

    llvm::SourceMgr& mgr_;
    DiagEngine& diag_engine_;

 public:
    Lexer(llvm::SourceMgr& mgr, DiagEngine& diag_engine);

    void GetNextToken(Token&);
    void SaveState();
    void RestoreState();

    DiagEngine& GetDiagEngine() const {
        return diag_engine_;
    }
};

#endif  // LEXER_H_
