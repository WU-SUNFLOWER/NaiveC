// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <memory>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "ast.h"
#include "parser.h"

class CodeGen : public Visitor {
 private:
    llvm::LLVMContext context_;
    llvm::IRBuilder<> ir_builder_ { context_ };
    std::shared_ptr<llvm::Module> module_ { nullptr };

 public:
    explicit CodeGen(std::shared_ptr<Program> prog);

    llvm::Value* VisitProgram(Program* prog) override;
    llvm::Value* VisitBinaryExpr(BinaryExpression* binary_expr) override;
    llvm::Value* VisitFactorExpr(FactorExpression* factor_expr) override;
};

#endif  // CODEGEN_H_
