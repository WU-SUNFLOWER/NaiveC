// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef PRINT_VISITOR_H_
#define PRINT_VISITOR_H_

#include "ast.h"

class PrintVisitor : public Visitor {
 public:
    explicit PrintVisitor(std::shared_ptr<Program> prog);

    llvm::Value* VisitProgram(Program* prog) override;
    llvm::Value* VisitBinaryExpr(BinaryExpression* binary_expr) override;
    llvm::Value* VisitFactorExpr(FactorExpression* factor_expr) override;
};

#endif  // PRINT_VISITOR_H_
