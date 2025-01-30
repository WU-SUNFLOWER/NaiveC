// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef AST_H_
#define AST_H_

#include <memory>
#include <vector>

#include "llvm/IR/Value.h"

#include "type.h"

class Program;
class AstNode;
class BinaryExpr;
class NumberExpr;
class VariableAccessExpr;
class VariableDecl;
class AssignExpr;

class Visitor {
 public:
    virtual ~Visitor() {}

    virtual llvm::Value* VisitProgram(Program*) = 0;

    virtual llvm::Value* VisitNumberExpr(NumberExpr*) = 0;
    virtual llvm::Value* VisitBinaryExpr(BinaryExpr*) = 0;
    virtual llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) = 0;
    virtual llvm::Value* VisitVariableDecl(VariableDecl*) = 0;
    virtual llvm::Value* VisitAssignExpr(AssignExpr*) = 0;
};

class AstNode {
 public:
    enum class AstNodeKind {
        kVariableDecl,
        kBinaryExpr,
        kNumberExpr,
        kVariableAccessExpr,
        kAssignExpr,
    };

 private:
    const AstNodeKind node_kind_;
    CType* ctype_ { nullptr };

 public:
    explicit AstNode(AstNodeKind node_kind) : node_kind_(node_kind) {}

    virtual ~AstNode() {}

    void SetCType(CType* ctype) {
        ctype_ = ctype;
    }

    CType* GetCType() const {
        return ctype_;
    }

    AstNodeKind GetNodeKind() const {
        return node_kind_;
    }

    virtual llvm::Value* Accept(Visitor* vis) = 0;
};

class VariableDecl : public AstNode {
 public:
    llvm::StringRef name_;

    VariableDecl() : AstNode(AstNodeKind::kVariableDecl) {}

    const llvm::StringRef& GetName() const {
        return name_;
    }

    void SetName(const llvm::StringRef& name) {
        name_ = name;
    }

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitVariableDecl(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kVariableDecl;
    }
};

enum class OpCode {
    kAdd,
    kSub,
    kMul,
    kDiv,
};

class BinaryExpr : public AstNode {
 public:
    OpCode op_;
    std::shared_ptr<AstNode> left_;
    std::shared_ptr<AstNode> right_;

    BinaryExpr() : AstNode(AstNodeKind::kBinaryExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitBinaryExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kBinaryExpr;
    }
};

class NumberExpr : public AstNode {
 public:
    int number_;

    NumberExpr() : AstNode(AstNodeKind::kNumberExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitNumberExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kNumberExpr;
    }
};

class VariableAccessExpr : public AstNode {
 public:
    llvm::StringRef name_;

    VariableAccessExpr() : AstNode(AstNodeKind::kVariableAccessExpr) {}

    void SetName(const llvm::StringRef& name) {
        name_ = name;
    }

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitVariableAccessExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kVariableAccessExpr;
    }
};

class AssignExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> left_;
    std::shared_ptr<AstNode> right_;

    AssignExpr() : AstNode(AstNodeKind::kAssignExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitAssignExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kAssignExpr;
    }
};

class Program {
 public:
    std::vector<std::shared_ptr<AstNode>> expr_vec_;
};

#endif  // AST_H_
