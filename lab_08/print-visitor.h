// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef PRINT_VISITOR_H_
#define PRINT_VISITOR_H_

#include "ast.h"

class PrintVisitor : public Visitor {
 private:
    llvm::raw_ostream *out_;

 public:
    explicit PrintVisitor(std::shared_ptr<Program> program, llvm::raw_ostream *out = &llvm::outs());

    llvm::Value* VisitProgram(Program*) override;

    llvm::Value* VisitDeclStmt(DeclStmt*) override;
    llvm::Value* VisitBlockStmt(BlockStmt*) override;
    llvm::Value* VisitIfStmt(IfStmt*) override;
    llvm::Value* VisitForStmt(ForStmt*) override;
    llvm::Value* VisitBreakStmt(BreakStmt*) override;
    llvm::Value* VisitContinueStmt(ContinueStmt*) override;

    llvm::Value* VisitBinaryExpr(BinaryExpr*) override;
    llvm::Value* VisitTernaryExpr(TernaryExpr*) override;

    llvm::Value* VisitNumberExpr(NumberExpr*) override;
    llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) override;
    llvm::Value* VisitVariableDecl(VariableDecl*) override;
    llvm::Value* VisitAssignExpr(AssignExpr*) override;
};

#endif  // PRINT_VISITOR_H_
