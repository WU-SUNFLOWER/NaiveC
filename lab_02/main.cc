// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include <utility>
#include <memory>
#include <iostream>

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ErrorOr.h"

#include "lexer.h"
#include "parser.h"
#include "print-visitor.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        llvm::errs() << "Please input filename!\n";
        return -1;
    }

    const char* file_name = argv[1];
    auto file = llvm::MemoryBuffer::getFile(file_name);
    if (!file) {
        llvm::errs() << "Fail to open file: " << file_name << "\n";
        return -1;
    }
    
    std::unique_ptr<llvm::MemoryBuffer> file_buf = std::move(*file);
    Lexer lexer(file_buf->getBuffer());

    /*
    Token token;
    while (token.GetType() != TokenType::kEOF) {
        lexer.GetNextToken(token);
        token.Dump();
    }    
    */

    Parser parser(lexer);
    auto program = parser.ParserProgram();

    PrintVisitor visitor(program);
    return 0;
}
