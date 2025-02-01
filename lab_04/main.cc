// Copyright 2025 WU-SUNFLOWER. All rights reserved.
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
#include "codegen.h"
#include "sema.h"
#include "diag-engine.h"

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
    
    llvm::SourceMgr mgr;
    DiagEngine diag_engine(mgr);
    mgr.AddNewSourceBuffer(std::move(*file), llvm::SMLoc());

    Lexer lexer(mgr, diag_engine);

    /* Lab 03 Step 01 
    Token token;
    while (token.GetType() != TokenType::kEOF) {
        lexer.GetNextToken(token);
        token.Dump();
    }*/

    /* Lab 02 Step 02 */
    Sema sema(diag_engine);
    Parser parser(lexer, sema);
    auto program = parser.ParserProgram();

    //PrintVisitor visitor(program);
    
    /* Lab 03 Step 03*/
    CodeGen generator(program);

    return 0;
}
