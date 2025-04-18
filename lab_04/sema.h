// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef SEMA_H_
#define SEMA_H_

#include <memory>

#include "scope.h"
#include "ast.h"
#include "diag-engine.h"

class Sema {
 private:
    Scope scope_;
    DiagEngine& diag_engine_;

 public:
    explicit Sema(DiagEngine& diag_engine) : diag_engine_(diag_engine) {}

    std::shared_ptr<AstNode> SemaVariableDeclNode(Token& token, CType* ctype);
    
    std::shared_ptr<AstNode> SemaAssignExprNode(
                                    Token& token,
                                    std::shared_ptr<AstNode> left, 
                                    std::shared_ptr<AstNode> right);

    std::shared_ptr<AstNode> SemaVariableAccessNode(Token& token);

    std::shared_ptr<AstNode> SemaBinaryExprNode(
                                    std::shared_ptr<AstNode> left, 
                                    std::shared_ptr<AstNode> right, 
                                    OpCode op);

    std::shared_ptr<AstNode> SemaNumberExprNode(Token& token, CType* ctype);
};

#endif  // SEMA_H_
