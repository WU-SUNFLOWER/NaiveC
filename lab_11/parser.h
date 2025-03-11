// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef PARSER_H_
#define PARSER_H_

#include <memory>
#include <vector>

#include "lexer.h"
#include "ast.h"
#include "sema.h"

class Parser {
 private:
    Lexer& lexer_;
    Sema& sema_;
    Token token_ {};

    std::vector<std::shared_ptr<AstNode>> breaked_able_nodes_;
    std::vector<std::shared_ptr<AstNode>> continued_able_nodes_;

    void AddBreakedAbleNode(std::shared_ptr<AstNode> node) {
        breaked_able_nodes_.emplace_back(node);
    }

    void AddContinuedAbleNode(std::shared_ptr<AstNode> node) {
        continued_able_nodes_.emplace_back(node);
    }

    void RemoveBreakedAbleNode(std::shared_ptr<AstNode> node) {
        assert(!breaked_able_nodes_.empty() && breaked_able_nodes_.back() == node);
        breaked_able_nodes_.pop_back();
    }

    void RemoveContinuedAbleNode(std::shared_ptr<AstNode> node) {
        assert(!continued_able_nodes_.empty() && continued_able_nodes_.back() == node);
        continued_able_nodes_.pop_back();
    }

 public:
    explicit Parser(Lexer& lexer, Sema& sema);

    std::shared_ptr<Program> ParseProgram();

 private:
    bool IsFuncDecl();
    std::shared_ptr<AstNode> ParseFuncDecl();

    std::shared_ptr<AstNode> ParseStmt();
    std::shared_ptr<AstNode> ParseBlockStmt();
    std::shared_ptr<AstNode> ParseReturnStmt();
    
    std::shared_ptr<AstNode> ParseDeclStmt(bool is_global);
    std::shared_ptr<CType> ParseDeclSpec();
    std::shared_ptr<CType> ParseStructOrUnionSpec();

    std::shared_ptr<AstNode> ParseDeclarator(std::shared_ptr<CType>, bool is_global);
    std::shared_ptr<AstNode> ParseDirectDeclarator(std::shared_ptr<CType>, bool is_global);

    std::shared_ptr<CType> ParseDirectDeclaratorSuffix(const Token& identifier, 
                                                       std::shared_ptr<CType>, 
                                                       bool is_global);
    std::shared_ptr<CType> ParseDirectDeclaratorArraySuffix(std::shared_ptr<CType>, 
                                                            bool is_global);
    std::shared_ptr<CType> ParseDirectDeclaratorFuncSuffix(const Token& identifier, 
                                                           std::shared_ptr<CType>, 
                                                           bool is_global);

    bool ParseInitializer(std::vector<std::shared_ptr<VariableDecl::InitValue>>& init_values,
                          std::shared_ptr<CType> decl_type,
                          std::vector<int>& index_list,
                          bool has_lbrace);

    std::shared_ptr<AstNode> ParseExprStmt();
    std::shared_ptr<AstNode> ParseIfStmt();
    std::shared_ptr<AstNode> ParseForStmt();
    std::shared_ptr<AstNode> ParseBreakStmt();
    std::shared_ptr<AstNode> ParseContinueStmt();

    std::shared_ptr<AstNode> ParseExpr();
    std::shared_ptr<AstNode> ParseAssignExpr();
    std::shared_ptr<AstNode> ParseConditionalExpr();
    std::shared_ptr<AstNode> ParseEqualExpr();
    std::shared_ptr<AstNode> ParseRelationalExpr();

    std::shared_ptr<AstNode> ParseAddExpr();
    // Including `*`, `/`, `%`
    std::shared_ptr<AstNode> ParseMultiExpr();
    std::shared_ptr<AstNode> ParseUnaryExpr();
    std::shared_ptr<AstNode> ParsePostFixExpr();

    // Since `||` and `&&` have different priorities, 
    // we should define different functions to process them separately.
    std::shared_ptr<AstNode> ParseLogOrExpr();
    std::shared_ptr<AstNode> ParseLogAndExpr();

    std::shared_ptr<AstNode> ParseBitOrExpr();
    std::shared_ptr<AstNode> ParseBitXorExpr();
    std::shared_ptr<AstNode> ParseBitAndExpr();
    std::shared_ptr<AstNode> ParseBitShiftExpr();

    std::shared_ptr<AstNode> ParsePrimaryExpr();

    std::shared_ptr<CType> ParseTypeName();

    bool Expect(TokenType token_type);
    bool Consume(TokenType token_type);
    void Advance();

    DiagEngine& GetDiagEngine() const {
      return lexer_.GetDiagEngine();
    }

    bool CurrentTokenIsAssignOperator() const;
    bool CurrentTokenIsUnaryOperator() const;
};

#endif  // PARSER_H_
