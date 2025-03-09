// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <memory>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"

#include "ast.h"
#include "parser.h"
#include "type.h"

class CodeGen : public Visitor, public TypeVisitor {
 private:
    llvm::LLVMContext context_;
    llvm::IRBuilder<> ir_builder_ { context_ };

    std::unique_ptr<llvm::Module> module_ { nullptr };

    llvm::StringMap<std::pair<llvm::Value*, llvm::Type*>> variable_map_;

    llvm::Function* cur_func_ { nullptr };

    void SetCurrentFunc(llvm::Function* func) {
        cur_func_ = func;
    }

    llvm::Function* GetCurrentFunc() const {
        return cur_func_;
    }

    llvm::DenseMap<AstNode*, llvm::BasicBlock*> break_block_map_;
    llvm::DenseMap<AstNode*, llvm::BasicBlock*> continue_block_map_;

 public:
    explicit CodeGen(std::shared_ptr<Program> prog);

    std::unique_ptr<llvm::Module>& GetModule() {
        return module_;
    }

    llvm::Value* VisitProgram(Program*) override;

    llvm::Value* VisitDeclStmt(DeclStmt*) override;
    llvm::Value* VisitBlockStmt(BlockStmt*) override;
    llvm::Value* VisitIfStmt(IfStmt*) override;
    llvm::Value* VisitForStmt(ForStmt*) override;
    llvm::Value* VisitBreakStmt(BreakStmt*) override;
    llvm::Value* VisitContinueStmt(ContinueStmt*) override;

    llvm::Value* VisitUnaryExpr(UnaryExpr*) override;
    llvm::Value* VisitBinaryExpr(BinaryExpr*) override;
    llvm::Value* VisitTernaryExpr(TernaryExpr*) override;

    llvm::Value* VisitNumberExpr(NumberExpr*) override;
    llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) override;
    llvm::Value* VisitVariableDecl(VariableDecl*) override;
    llvm::Value* VisitSizeofExpr(SizeofExpr*) override;

    llvm::Value* VisitPostIncExpr(PostIncExpr*) override;
    llvm::Value* VisitPostDecExpr(PostDecExpr*) override;

    llvm::Value* VisitPostSubscript(PostSubscriptExpr*) override;

    llvm::Type* VisitPrimaryType(CPrimaryType*) override;
    llvm::Type* VisitPointerType(CPointerType*) override;
    llvm::Type* VisitArrayType(CArrayType*) override;
};

#endif  // CODEGEN_H_
