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

class IfStmt;
class DeclStmt;
class BlockStmt;
class ForStmt;
class BreakStmt;
class ContinueStmt;

class UnaryExpr;
class BinaryExpr;
class TernaryExpr;

class NumberExpr;
class VariableAccessExpr;
class VariableDecl;
class SizeofExpr;

class PostIncExpr;
class PostDecExpr;

class PostSubscriptExpr;

class PostMemberDotExpr;
class PostMemberArrowExpr;

class FuncDecl;
class PostFuncCallExpr;
class ReturnStmt;

class Visitor {
 public:
    virtual ~Visitor() {}

    virtual llvm::Value* VisitProgram(Program*) = 0;

    virtual llvm::Value* VisitDeclStmt(DeclStmt*) = 0;
    virtual llvm::Value* VisitBlockStmt(BlockStmt*) = 0;
    virtual llvm::Value* VisitIfStmt(IfStmt*) = 0;
    virtual llvm::Value* VisitForStmt(ForStmt*) = 0;
    virtual llvm::Value* VisitBreakStmt(BreakStmt*) = 0;
    virtual llvm::Value* VisitContinueStmt(ContinueStmt*) = 0;

    virtual llvm::Value* VisitUnaryExpr(UnaryExpr*) = 0;
    virtual llvm::Value* VisitBinaryExpr(BinaryExpr*) = 0;
    virtual llvm::Value* VisitTernaryExpr(TernaryExpr*) = 0;

    virtual llvm::Value* VisitNumberExpr(NumberExpr*) = 0;
    virtual llvm::Value* VisitVariableAccessExpr(VariableAccessExpr*) = 0;
    virtual llvm::Value* VisitVariableDecl(VariableDecl*) = 0;
    virtual llvm::Value* VisitSizeofExpr(SizeofExpr*) = 0;

    virtual llvm::Value* VisitPostIncExpr(PostIncExpr*) = 0;
    virtual llvm::Value* VisitPostDecExpr(PostDecExpr*) = 0;

    virtual llvm::Value* VisitPostSubscript(PostSubscriptExpr*) = 0;
    
    virtual llvm::Value* VisitPostMemberDotExpr(PostMemberDotExpr*) = 0;
    virtual llvm::Value* VisitPostMemberArrowExpr(PostMemberArrowExpr*) = 0;

    virtual llvm::Value* VisitFuncDecl(FuncDecl*) = 0;
    virtual llvm::Value* VisitPostFuncCallExpr(PostFuncCallExpr*) = 0;
    virtual llvm::Value* VisitReturnStmt(ReturnStmt*) = 0;
};

class AstNode {
 public:
    enum class AstNodeKind {
        kDeclStmt,
        kBlockStmt,
        kIfStmt,
        kForStmt,
        kBreakStmt,
        kContinueStmt,

        kUnaryExpr,
        kBinaryExpr,
        kTernaryExpr,

        kVariableDecl,
        kNumberExpr,
        kVariableAccessExpr,
        kSizeof,

        kPostIncExpr,
        kPostDecExpr,

        kPostSubscriptExpr,

        kPostMemberDotExpr,
        kPostMemberArrowExpr,

        kFuncDecl,
        kPostFuncCallExpr,
        kReturnStmt,
    };

 private:
    const AstNodeKind node_kind_;
    std::shared_ptr<CType> ctype_ { nullptr };
    Token bound_token_ {};

    // An lvalue can be both evaluated (or have its address taken) 
    // and assigned to.
    // An rvalue can only be evaluated.
    bool is_lvalue_ { false };

 public:
    explicit AstNode(AstNodeKind node_kind) : node_kind_(node_kind) {}

    virtual ~AstNode() {}

    void SetBoundToken(const Token& token) {
        bound_token_ = token;
    }

    const Token& GetBoundToken() const {
        return bound_token_;
    }

    void SetCType(std::shared_ptr<CType> ctype) {
        ctype_ = ctype;
    }

    std::shared_ptr<CType> GetCType() const {
        return ctype_;
    }

    AstNodeKind GetNodeKind() const {
        return node_kind_;
    }

    bool IsLValue() const {
        return is_lvalue_;
    }

    void SetLValue(bool flag) {
        is_lvalue_ = flag;
    }

    virtual llvm::Value* Accept(Visitor* vis) = 0;
};

class Program {
 public:
    llvm::StringRef file_name_;

    // A program consists of serveral external declaration,
    // including functions, global variables...
    std::vector<std::shared_ptr<AstNode>> nodes_;
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

class ForStmt : public AstNode {
 public:
    std::shared_ptr<AstNode> init_node_;
    std::shared_ptr<AstNode> cond_node_;
    std::shared_ptr<AstNode> inc_node_;
    std::shared_ptr<AstNode> body_node_;

    ForStmt() : AstNode(AstNodeKind::kForStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitForStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kForStmt;
    }
};

class BreakStmt : public AstNode {
 public:
    std::shared_ptr<AstNode> target_ { nullptr };

    BreakStmt() : AstNode(AstNodeKind::kBreakStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitBreakStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kBreakStmt;
    }
};

class ContinueStmt : public AstNode {
 public:
    std::shared_ptr<AstNode> target_ { nullptr };

    ContinueStmt() : AstNode(AstNodeKind::kContinueStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitContinueStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kContinueStmt;
    }
};

class VariableDecl : public AstNode {
 public:
    struct InitValue {
        std::shared_ptr<CType> decl_type;
        std::shared_ptr<AstNode> init_node;
        // For example, 
        // `int ar[][][] = {
        //     {
        //         {1, 2},
        //         {3, 4}
        //     },
        //     {
        //         {5, 6}
        //     }
        // };`
        //
        // Then, the `index_list` of element `6` is `[0, 1, 0, 1]`.
        //
        // The trick thing that you should forget out is that why
        // we have to add a zero in the head of `index_list`.
        //
        // Please read the comment in `Parser::ParseDirectDeclarator` method
        // to learn about the reason.
        std::vector<int> index_list;
    };

    std::vector<std::shared_ptr<InitValue>> init_values_;

    bool is_global_ { false };

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

enum class UnaryOpCode {
    // +i
    kPositive,
    // -i
    kNegative,
    // ++i
    kSelfIncreasing,
    // --i
    kSelfDecreasing,
    // *i
    kDereference,
    // &i
    kAddress,
    // !i
    kLogicalNot,
    // ~i
    kBitwiseNot,
};

class UnaryExpr : public AstNode {
 public:
    UnaryOpCode op_;
    std::shared_ptr<AstNode> sub_node_;

    UnaryExpr() : AstNode(AstNodeKind::kUnaryExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitUnaryExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kBinaryExpr;
    }
};

class SizeofExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> sub_node_ { nullptr };
    std::shared_ptr<CType> sub_ctype_ { nullptr };

    SizeofExpr() : AstNode(AstNodeKind::kSizeof) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitSizeofExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kSizeof;
    }
};

enum class BinaryOpCode {
    kEqualEqual,
    kNotEqual,

    kLess,
    kLessEqual,
    kGreater,
    kGreaterEqual,

    kAdd,
    kSub,
    kMul,
    kDiv,
    kMod,

    kLogicalOr,
    kLogicalAnd,

    kBitwiseOr,
    kBitwiseAnd,
    kBitwiseXor,
    kLeftShift,
    kRightShift,

    kAssign,
    kAddAssign,
    kSubAssign,
    kMulAssign,
    kDivAssign,
    kModAssign,

    kLeftShiftAssign,
    kRightShiftAssign,
    kBitwiseAndAssign,
    kBitwiseOrAssign,
    kBitwiseXorAssign,

    kComma,
};

class BinaryExpr : public AstNode {
 public:
    BinaryOpCode op_;
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

class TernaryExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> cond_;
    std::shared_ptr<AstNode> then_;
    std::shared_ptr<AstNode> els_;

    TernaryExpr() : AstNode(AstNodeKind::kTernaryExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitTernaryExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kTernaryExpr;
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

class PostIncExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> sub_node_;

    PostIncExpr() : AstNode(AstNodeKind::kPostIncExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitPostIncExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kPostIncExpr;
    }
};

class PostDecExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> sub_node_;

    PostDecExpr() : AstNode(AstNodeKind::kPostDecExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitPostDecExpr(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kPostDecExpr;
    }
};

class PostSubscriptExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> sub_node_;
    std::shared_ptr<AstNode> index_node_;

    PostSubscriptExpr() : AstNode(AstNodeKind::kPostSubscriptExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitPostSubscript(this);
    }

    // Provide support for LLVM RTTI
    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kPostSubscriptExpr;
    }
};

class PostMemberDotExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> struct_node_;
    CRecordType::Member target_member_;
    
    PostMemberDotExpr() : AstNode(AstNodeKind::kPostMemberDotExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitPostMemberDotExpr(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kPostMemberDotExpr;
    }
};

class PostMemberArrowExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> struct_pointer_node_;
    CRecordType::Member target_member_;
    
    PostMemberArrowExpr() : AstNode(AstNodeKind::kPostMemberArrowExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitPostMemberArrowExpr(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kPostMemberArrowExpr;
    }
};

class FuncDecl : public AstNode {
 public:
    std::shared_ptr<AstNode> block_stmt_ { nullptr };

    FuncDecl() : AstNode(AstNodeKind::kFuncDecl) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitFuncDecl(this);
    }

    static bool classof(const AstNode* node)  {
        return node->GetNodeKind() == AstNodeKind::kFuncDecl;
    }
};

class PostFuncCallExpr : public AstNode {
 public:
    std::shared_ptr<AstNode> func_node_;
    std::vector<std::shared_ptr<AstNode>> arg_nodes_;

    PostFuncCallExpr() : AstNode(AstNodeKind::kPostFuncCallExpr) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitPostFuncCallExpr(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kPostFuncCallExpr;
    }
};

class ReturnStmt : public AstNode {
 public:
    std::shared_ptr<AstNode> value_node_ { nullptr };

    ReturnStmt() : AstNode(AstNodeKind::kReturnStmt) {}

    llvm::Value* Accept(Visitor* vis) override {
        return vis->VisitReturnStmt(this);
    }

    static bool classof(const AstNode* node) {
        return node->GetNodeKind() == AstNodeKind::kReturnStmt;
    }
};

#endif  // AST_H_
