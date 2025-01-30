// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef SEMA_H_
#define SEMA_H_

#include <memory>

#include "scope.h"
#include "ast.h"

class Sema {
 private:
    Scope scope_;

 public:
    std::shared_ptr<AstNode> SemaVariableDeclNode(const llvm::StringRef& name, CType* ctype);
    
    std::shared_ptr<AstNode> SemaAssignExprNode(
                                    std::shared_ptr<AstNode> left, 
                                    std::shared_ptr<AstNode> right);

    std::shared_ptr<AstNode> SemaVariableAccessNode(const llvm::StringRef& name);

    std::shared_ptr<AstNode> SemaBinaryExprNode(
                                    std::shared_ptr<AstNode> left, 
                                    std::shared_ptr<AstNode> right, 
                                    OpCode op);

    std::shared_ptr<AstNode> SemaNumberExprNode(int number, CType* ctype);
};

#endif  // SEMA_H_
