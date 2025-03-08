// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef PRINT_VISITOR_H_
#define PRINT_VISITOR_H_

#include "ast.h"
#include "type.h"

class PrintVisitor : public Visitor, public TypeVisitor {
 private:
    llvm::raw_ostream *out_;

 public:
    explicit PrintVisitor(std::shared_ptr<Program> program, llvm::raw_ostream *out = &llvm::outs());

    // Virtual functions of Visitor.
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

    // Virtual functions of TypeVisitor.
    llvm::Type* VisitPrimaryType(CPrimaryType*) override;
    llvm::Type* VisitPointerType(CPointerType*) override;
    llvm::Type* VisitArrayType(CArrayType*) override;
};

#endif  // PRINT_VISITOR_H_
