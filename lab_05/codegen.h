// Copyright 2025 WU-SUNFLOWER. All rights reserved.
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

    llvm::StringMap<std::pair<llvm::Value*, llvm::Type*>> variable_map_;

    llvm::Function* cur_func_ { nullptr };

    void SetCurrentFunc(llvm::Function* func) {
        cur_func_ = func;
    }

    llvm::Function* GetCurrentFunc() const {
        return cur_func_;
    }

 public:
    explicit CodeGen(std::shared_ptr<Program> prog);

    llvm::Value* VisitProgram(Program*) override;

    llvm::Value* VisitDeclStmt(DeclStmt*) override;
    llvm::Value* VisitBlockStmt(BlockStmt*) override;
    llvm::Value* VisitIfStmt(IfStmt*) override;

    llvm::Value* VisitNumberExpr(NumberExpr*) override;
    llvm::Value* VisitBinaryExpr(BinaryExpr*) override;
    llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) override;
    llvm::Value* VisitVariableDecl(VariableDecl*) override;
    llvm::Value* VisitAssignExpr(AssignExpr*) override;
};

#endif  // CODEGEN_H_
