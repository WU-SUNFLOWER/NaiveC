// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef AST_H_
#define AST_H_

#include <memory>
#include <vector>

#include "llvm/IR/Value.h"

#include "type.h"
#include "lexer.h"

class Program;
class AstNode;
class BinaryExpr;
class NumberExpr;
class VariableAccessExpr;
class VariableDecl;
class AssignExpr;
class IfStmt;
class DeclStmt;
class BlockStmt;

class Visitor {
 public:
    virtual ~Visitor() {}

    virtual llvm::Value* VisitProgram(Program*) = 0;

    virtual llvm::Value* VisitDeclStmt(DeclStmt*) = 0;
    virtual llvm::Value* VisitBlockStmt(BlockStmt*) = 0;
    virtual llvm::Value* VisitIfStmt(IfStmt*) = 0;

    virtual llvm::Value* VisitNumberExpr(NumberExpr*) = 0;
    virtual llvm::Value* VisitBinaryExpr(BinaryExpr*) = 0;
    virtual llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) = 0;
    virtual llvm::Value* VisitVariableDecl(VariableDecl*) = 0;
    virtual llvm::Value* VisitAssignExpr(AssignExpr*) = 0;
};

class AstNode {
 public:
    enum class AstNodeKind {
        kDeclStmt,
        kBlockStmt,
        kIfStmt,

        kVariableDecl,
        kBinaryExpr,
        kNumberExpr,
        kVariableAccessExpr,
        kAssignExpr,
    };

 private:
    const AstNodeKind node_kind_;
    CType* ctype_ { nullptr };
    Token bound_token_ {};

 public:
    explicit AstNode(AstNodeKind node_kind) : node_kind_(node_kind) {}

    virtual ~AstNode() {}

    void SetBoundToken(Token& token) {
        bound_token_ = token;
    }

    const Token& GetBoundToken() const {
        return bound_token_;
    }

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

class DeclStmt : public AstNode {
 public:
    std::vector<std::shared_ptr<AstNode>> nodes_;

    DeclStmt() : AstNode(AstNodeKind::kDeclStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitDeclStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kDeclStmt;
    }
};

class BlockStmt : public AstNode {
 public:
    std::vector<std::shared_ptr<AstNode>> nodes_;

    BlockStmt() : AstNode(AstNodeKind::kBlockStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitBlockStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kBlockStmt;
    }
};

class VariableDecl : public AstNode {
 public:
    VariableDecl() : AstNode(AstNodeKind::kVariableDecl) {}

    llvm::StringRef GetVariableName() const {
        return GetBoundToken().GetContent();
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
    NumberExpr() : AstNode(AstNodeKind::kNumberExpr) {}

    int GetNumber() const {
        return GetBoundToken().GetValue();
    }

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
    VariableAccessExpr() : AstNode(AstNodeKind::kVariableAccessExpr) {}

    llvm::StringRef GetVariableName() const {
        return GetBoundToken().GetContent();
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

    std::shared_ptr<AstNode> GetLeftChild() {
        return left_;
    }

    std::shared_ptr<AstNode> GetRightChild() {
        return right_;
    }

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitAssignExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kAssignExpr;
    }
};

class IfStmt : public AstNode {
 public:
    std::shared_ptr<AstNode> cond_node_;
    std::shared_ptr<AstNode> then_node_;
    std::shared_ptr<AstNode> else_node_;

    IfStmt() : AstNode(AstNodeKind::kIfStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitIfStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kIfStmt;
    }
};

class Program {
 public:
    std::vector<std::shared_ptr<AstNode>> nodes_;
};

#endif  // AST_H_
