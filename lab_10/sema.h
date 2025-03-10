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
 public:
    enum class Mode {
        kNormal,
        kSkip,
    };

 private:
    Mode mode_;
    Scope scope_;
    DiagEngine& diag_engine_;

 public:
    explicit Sema(DiagEngine& diag_engine) : diag_engine_(diag_engine), mode_(Mode::kNormal) {}

    void EnterScope();
    void ExitScope();

    void SetMode(Mode mode);

    std::shared_ptr<AstNode> SemaVariableDeclNode(Token& token, std::shared_ptr<CType> ctype);

    std::shared_ptr<AstNode> SemaVariableAccessNode(Token& token);

    std::shared_ptr<AstNode> SemaBinaryExprNode(
                                    std::shared_ptr<AstNode> left, 
                                    std::shared_ptr<AstNode> right, 
                                    BinaryOpCode op);

    std::shared_ptr<AstNode> SemaUnaryExprNode(
                                    std::shared_ptr<AstNode> sub, 
                                    UnaryOpCode op,
                                    Token& token);

    std::shared_ptr<AstNode> SemaTernaryExprNode(
                                    std::shared_ptr<AstNode> cond,
                                    std::shared_ptr<AstNode> then,
                                    std::shared_ptr<AstNode> els,
                                    Token& token);

    std::shared_ptr<AstNode> SemaSizeofExprNode(
                                    std::shared_ptr<AstNode> sub, 
                                    std::shared_ptr<CType> ctype);

    std::shared_ptr<AstNode> SemaPostIncExprNode(std::shared_ptr<AstNode> sub, Token& token);

    std::shared_ptr<AstNode> SemaPostDecExprNode(std::shared_ptr<AstNode> sub, Token& token);

    std::shared_ptr<VariableDecl::InitValue> SemaDeclInitValueStruct(
                                                   std::shared_ptr<CType> decl_type,
                                                   std::shared_ptr<AstNode> init_node,
                                                   std::vector<int>& index_list,
                                                   Token& token);

    std::shared_ptr<AstNode> SemaPostSubscriptExprNode(
                                       std::shared_ptr<AstNode> sub_node,
                                       std::shared_ptr<AstNode> index_node,
                                       Token& token);

    std::shared_ptr<AstNode> SemaNumberExprNode(Token& token, std::shared_ptr<CType> ctype);

    std::shared_ptr<AstNode> SemaIfStmtNode(
                                    std::shared_ptr<AstNode> cond_node, 
                                    std::shared_ptr<AstNode> then_node,
                                    std::shared_ptr<AstNode> else_node);

    std::shared_ptr<CType> SemaTagDecl(Token& token, 
                                       const std::vector<CRecordType::Member> members, 
                                       CType::TagKind tag_kind);
    std::shared_ptr<CType> SemaTagAnonymousDecl(
                                       const std::vector<CRecordType::Member> members, 
                                       CType::TagKind tag_kind); 
    std::shared_ptr<CType> SemaTagAccess(Token& token);

    std::shared_ptr<AstNode> SemaPostMemberDotExprNode(
                                       std::shared_ptr<AstNode> struct_node, 
                                       Token& op_token, 
                                       Token& token);
    std::shared_ptr<AstNode> SemaPostMemberArrowExprNode(
                                       std::shared_ptr<AstNode> struct_node,
                                       Token& op_token, 
                                       Token& token);
};

#endif  // SEMA_H_
