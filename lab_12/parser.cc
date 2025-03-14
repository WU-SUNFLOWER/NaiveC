// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "parser.h"

#include <cassert>
#include <vector>
#include <memory>
#include <utility>

bool IsTypeName(TokenType token_type) {
    return (token_type == TokenType::kInt ||
            token_type == TokenType::kStruct ||
            token_type == TokenType::kUnion ||
            token_type == TokenType::kVoid);
}

bool IsTypeName(const Token& token) {
    return IsTypeName(token.GetType());
}

Parser::Parser(Lexer& lexer, Sema& sema) : lexer_(lexer), sema_(sema) {
    Advance();
}

bool Parser::IsFuncDecl() {
    int is_func_decl = false;

    Token begin = token_;
    lexer_.SaveState();
    sema_.SetMode(Sema::Mode::kSkip);
    {
        auto base_type = ParseDeclSpec();
        if (token_.GetType() != TokenType::kSemi) {
            auto decl_node = ParseDeclarator(base_type, true);
            if (decl_node->GetCType()->GetKind() == CType::TypeKind::kFunc) {
                is_func_decl = true;
            }
        }
    }
    sema_.SetMode(Sema::Mode::kNormal);
    lexer_.RestoreState();
    token_ = begin;

    return is_func_decl;
}

std::shared_ptr<Program> Parser::ParseProgram() {
    auto prog = std::make_shared<Program>();
    prog->file_name_ = lexer_.GetFileName();

    while (token_.GetType() != TokenType::kEOF) {
        auto node = IsFuncDecl() ? ParseFuncDecl() : 
                                   ParseDeclStmt(true);
        if (node) {
            prog->nodes_.emplace_back(node);
        }
    }
    Consume(TokenType::kEOF);

    return prog;
}

std::shared_ptr<AstNode> Parser::ParseFuncDecl() {
    auto base_type = ParseDeclSpec();

    Token func_name_token;
    std::shared_ptr<CType> func_type = nullptr;
    std::shared_ptr<AstNode> func_body_node = nullptr;

    // NOTE:
    // Since the variables in function's parameter list are in
    // the same scope with the variables in function's body,
    // we must enter this scope before we parse the function's
    // parameter list!!!
    sema_.EnterScope();
    {
        auto decl_node = ParseDeclarator(base_type, true);
        
        func_name_token = decl_node->GetBoundToken();
        func_type = decl_node->GetCType();

        if (token_.GetType() == TokenType::kLBrace) {
            func_body_node = ParseBlockStmt();
        }
    }
    sema_.ExitScope();

    // Create function declare node, 
    // and add the function's name to symbol table.
    auto func_decl_node = sema_.SemaFuncDecl(func_name_token, func_type, func_body_node);

    // Eliminate potential redundant semicolons.
    while (token_.GetType() == TokenType::kSemi) {
        Advance();
    }

    return func_decl_node;
}

std::shared_ptr<AstNode> Parser::ParseStmt() {
#define TOKEN_TYPE_IS(type) (token_.GetType() == type)

    if (TOKEN_TYPE_IS(TokenType::kSemi)) {
        Advance();
        return nullptr;
    }
    else if (IsTypeName(token_)) {
        return ParseDeclStmt(false);
    }
    else if (TOKEN_TYPE_IS(TokenType::kIf)) {
        return ParseIfStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kLBrace)) {
        return ParseBlockStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kFor)) {
        return ParseForStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kBreak)) {
        return ParseBreakStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kContinue)) {
        return ParseContinueStmt();
    }
    else if (TOKEN_TYPE_IS(TokenType::kReturn)) {
        return ParseReturnStmt();
    }

#undef TOKEN_TYPE_IS

    return ParseExprStmt();
}

std::shared_ptr<CType> Parser::ParseDeclSpec() {
    switch (token_.GetType()) {
        case TokenType::kVoid: {
            Advance();
            return CType::kVoidType;
        }
        case TokenType::kInt: {
            Advance();
            return CType::kIntType;
        }
        case TokenType::kStruct:
        case TokenType::kUnion: {
            return ParseStructOrUnionSpec();
        }
        default: {
            GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()), 
                                   Diag::kErrType);
        }
    }
    
    return nullptr;
}

std::shared_ptr<CType> Parser::ParseStructOrUnionSpec() {
    CType::TagKind tag_kind;
    switch (token_.GetType()) {
        case TokenType::kStruct:
            tag_kind = CType::TagKind::kStruct;
            break;
        case TokenType::kUnion:
            tag_kind = CType::TagKind::kUnion;
            break;
        default:
            assert(0);
    }
    // Consume `struct` or `union` keyword.
    Advance();  

    Token tag_symbol_token;
    bool is_anonymous_struct = token_.GetType() != TokenType::kIdentifier;

    if (!is_anonymous_struct) {
        tag_symbol_token = token_;
        Consume(TokenType::kIdentifier);
    }

    // Case 1: user want to define new struct or union.
    // struct A {
    //     int a, b;
    //     int* p;
    // };
    if (token_.GetType() == TokenType::kLBrace) {
        Consume(TokenType::kLBrace);

        std::shared_ptr<CType> record_type = nullptr;
        if (is_anonymous_struct) {
            record_type = sema_.SemaTagAnonymousDecl(tag_kind);
        } else {
            record_type = sema_.SemaTagDecl(tag_symbol_token, tag_kind);
        }

        std::vector<CRecordType::Member> members;
        sema_.EnterScope();
        {
            while (token_.GetType() != TokenType::kRBrace) {
                auto decl_stmt = ParseDeclStmt(false);
                auto raw_decl_stmt = llvm::dyn_cast<DeclStmt>(decl_stmt.get());
                for (const auto& decl_node : raw_decl_stmt->nodes_) {
                    auto raw_decl_node = llvm::dyn_cast<VariableDecl>(decl_node.get());
                    members.emplace_back(raw_decl_node->GetCType(), raw_decl_node->GetVariableName());
                }
            }
        }
        sema_.ExitScope();

        // Now we have known all the members of this struct.
        // Let's update the struct type's member list here.
        auto raw_record_type = llvm::dyn_cast<CRecordType>(record_type.get());
        raw_record_type->SetMembers(std::move(members));

        Consume(TokenType::kRBrace);
        return record_type;
    }
    // Case 2: user want to use defined struct or union.
    else if (!is_anonymous_struct) {
        return sema_.SemaTagAccess(tag_symbol_token);
    }
    else {
        assert(0);
    }

    return nullptr;
}

std::shared_ptr<AstNode> Parser::ParseDeclarator(std::shared_ptr<CType> base_type, bool is_global) {
    // Process pointer variable.
    // For example:
    //     1. `int** ptr = &something` => `int**`
    //     2. `int* ar[10];` => `int*`
    while (token_.GetType() == TokenType::kStar) {
        Consume(TokenType::kStar);
        base_type = std::make_shared<CPointerType>(base_type);
    }

    return ParseDirectDeclarator(base_type, is_global);
}

std::shared_ptr<AstNode> Parser::ParseDirectDeclarator(std::shared_ptr<CType> base_type, bool is_global) {
    std::shared_ptr<AstNode> variable_decl_node = nullptr;
    
    // 1. Create the declarator node.
    switch (token_.GetType()) {
        // 1.1 Process the common situation, e.g. `int x, y=100;` or `int ar[3] = { 1, 2, 3 }`.
        case TokenType::kIdentifier: {
            Token identifier_tok = token_;
            Consume(TokenType::kIdentifier);
            // Process declarator, just like `A[1][2][3]...` or `sum(int a, int b)`.
            base_type = ParseDirectDeclaratorSuffix(identifier_tok, base_type, is_global);
            // Create AST node.
            variable_decl_node = sema_.SemaVariableDeclNode(identifier_tok, base_type, is_global);
            break;
        }
        // 1.2 Process the special situation, e.g. `int (*p)[3][4] = &a;`.
        case TokenType::kLParent: {
            Token dummy;
            // At first time, let's look forward.
            Token history_tok = token_;
            lexer_.SaveState();
            sema_.SetMode(Sema::Mode::kSkip);
            {
                Consume(TokenType::kLParent);
                // In this time while calling `ParseDeclarator`, 
                // we don't care about what you filled in the parentheses.
                // So, we just pass a dummy argument here.
                auto tmp_variable_decl_node = ParseDeclarator(CType::kIntType, is_global);
                Consume(TokenType::kRParent);
                // However, we are interested in what you wrote after the right parenthesis.
                base_type = ParseDirectDeclaratorSuffix(dummy, base_type, is_global);
            }
            sema_.SetMode(Sema::Mode::kNormal);
            lexer_.RestoreState();
            token_ = history_tok;

            // At second time, let's read what you wrote in the parentheses,
            // and combine it with what you wrote after the right parenthesis.
            Consume(TokenType::kLParent);
            variable_decl_node = ParseDeclarator(base_type, is_global);
            Consume(TokenType::kRParent);
            // Note that in second time, we just need to ignore
            // what you wrote after the right parenthesis.
            // So, we just pass a dummy argument here.
            sema_.SetMode(Sema::Mode::kSkip); 
            {
                ParseDirectDeclaratorSuffix(dummy, CType::kIntType, is_global);
            }
            sema_.SetMode(Sema::Mode::kNormal);
            break;
        }
        default: {
            GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()),
                                Diag::kErrExpectedDeclare,
                                "identifier or '('");     
        }
    }

    // 2. Create tne initializer node.
    if (token_.GetType() == TokenType::kEqual) {
        Advance();
        VariableDecl* raw_decl_node = llvm::dyn_cast<VariableDecl>(variable_decl_node.get());
        // NOTE: Why we have to add a zero in each `index_list`?
        // That's because when generating LLVM IR for initialize the member of an array, struct or union, 
        // we have to complete a dereference operation at first, which is the meaning of this zero.
        // For example, just like in C language, when we have an array pointer(`int (*p)[10];`),
        // we have to deref this pointer(`*p`) at first,
        // before we try to write or read an element from the array(`(*p)[idx]`).
        std::vector<int> index_list = { 0 };
        ParseInitializer(raw_decl_node->init_values_, raw_decl_node->GetCType(), index_list, false);
    }

    return variable_decl_node;
}

std::shared_ptr<CType> Parser::ParseDirectDeclaratorSuffix(
    const Token& iden, 
    std::shared_ptr<CType> base_type, 
    bool is_global) 
{
    switch (token_.GetType()) {
        case TokenType::kLBracket:
            base_type = ParseDirectDeclaratorArraySuffix(base_type, is_global);
            break;
        case TokenType::kLParent:
            base_type = ParseDirectDeclaratorFuncSuffix(iden, base_type, is_global);
            break;
    }

    return base_type;
}

// NOTE: 
// For example, if we have an array `int ar[12][34][56];`, 
// then its `element_type` is CType::kIntType
std::shared_ptr<CType> Parser::ParseDirectDeclaratorArraySuffix(
    std::shared_ptr<CType> element_type, 
    bool is_global) 
{
    // NOTE: 
    // Don't delete this if statement, 
    // since it is the exit of recursion.
    if (token_.GetType() != TokenType::kLBracket) {
        return element_type;
    }

    int array_element_cnt = -1;
    Consume(TokenType::kLBracket);
    {
        if (token_.GetType() != TokenType::kRBracket) {
            array_element_cnt = token_.GetValue();
            Consume(TokenType::kNumber);
        }
    }
    Consume(TokenType::kRBracket);

    auto sub_array_type = ParseDirectDeclaratorArraySuffix(element_type, is_global);
    return std::make_shared<CArrayType>(sub_array_type, array_element_cnt);
}

std::shared_ptr<CType> Parser::ParseDirectDeclaratorFuncSuffix(
    const Token& iden,
    std::shared_ptr<CType> ret_type, 
    bool is_global) 
{
    Consume(TokenType::kLParent);

    int i = 0;
    std::vector<CFuncType::Param> params;
    while (token_.GetType() != TokenType::kRParent) {
        if (0 < i && token_.GetType() == TokenType::kComma) {
            Consume(TokenType::kComma);
        }
        ++i;

        auto param_base_type = ParseDeclSpec();
        auto param_decl_node = ParseDeclarator(param_base_type, false);
        auto param_final_type = param_decl_node->GetCType();

        if (param_final_type->GetKind() == CType::TypeKind::kArray) {
            auto array_type = llvm::dyn_cast<CArrayType>(param_final_type.get());
            auto pointer_type = std::make_shared<CPointerType>(array_type->GetElementType());
            param_decl_node->SetCType(pointer_type);
        }

        params.emplace_back(param_decl_node->GetCType(), 
                            param_decl_node->GetBoundToken().GetContent());
    }

    Consume(TokenType::kRParent);

    return std::make_shared<CFuncType>(iden.GetContent(), ret_type, std::move(params));
}

bool Parser::ParseInitializer(
    std::vector<std::shared_ptr<VariableDecl::InitValue>> &init_values, 
    std::shared_ptr<CType> decl_type, 
    std::vector<int> &index_list,
    bool has_lbrace)
{
    // NOTE: This is the exit of recursion.
    if (token_.GetType() == TokenType::kRBrace) {
        if (!has_lbrace) {
            GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()),
                                   Diag::kErrMiss,
                                   "{");
        }
        return true;
    }

    if (token_.GetType() == TokenType::kLBrace) {
        Consume(TokenType::kLBrace);

        auto type_kind = decl_type->GetKind();
        switch (type_kind) {
            case CType::TypeKind::kArray: {
                auto array_type = llvm::dyn_cast<CArrayType>(decl_type.get());
                int total_element_count = array_type->GetElementCount();
                bool is_flex_array = total_element_count < 0;

                int initialized_element_count = 0;
                while (is_flex_array || initialized_element_count < total_element_count) {
                    index_list.push_back(initialized_element_count);
                    bool end = ParseInitializer(init_values, 
                                                array_type->GetElementType(), 
                                                index_list, 
                                                true);
                    index_list.pop_back();
                    if (end) {
                        break;
                    }
                    if (token_.GetType() == TokenType::kComma) {
                        Advance();
                    }
                    ++initialized_element_count;
                }

                if (is_flex_array) {
                    array_type->SetElementCount(initialized_element_count);
                }

                break;
            }
            case CType::TypeKind::kRecord: {
                CRecordType* record_type = llvm::dyn_cast<CRecordType>(decl_type.get());
                const auto& members = record_type->GetMembers();
                int member_count = members.size();

                switch (record_type->GetTagKind()) {
                    case CType::TagKind::kStruct: {
                        for (int i = 0; i < member_count; ++i) {
                            index_list.push_back(i);
                            bool end = ParseInitializer(init_values, 
                                                        members[i].type, 
                                                        index_list, 
                                                        true);
                            index_list.pop_back();
                            if (end) {
                                break;
                            }
                            if (token_.GetType() == TokenType::kComma) {
                                Advance();
                            }
                        }                       
                        break;
                    }
                    case CType::TagKind::kUnion: {
                        if (member_count > 0) {
                            index_list.push_back(0);
                            ParseInitializer(init_values, members[0].type, index_list, true);
                            index_list.pop_back();
                        }
                        break;
                    }
                    default: {
                        // TO DO: throw error for invaild tag kind.
                    }
                }
                break;
            }
            default: {
                // TO DO: throw error for invalid type kind.
            }   
        }

        Consume(TokenType::kRBrace);
    }
    else {
        Token tmp = token_;
        auto init_node = ParseAssignExpr();
        auto init_value_struct = sema_.SemaDeclInitValueStruct(decl_type, init_node, index_list, tmp);
        init_values.emplace_back(init_value_struct);
    }

    return false;
}

std::shared_ptr<AstNode> Parser::ParseReturnStmt() {
    Consume(TokenType::kReturn);

    std::shared_ptr<AstNode> ret_value_expr = nullptr;
    if (token_.GetType() != TokenType::kSemi) {
        ret_value_expr = ParseExpr();
        Consume(TokenType::kSemi);        
    }

    auto ret_stmt_node = std::make_shared<ReturnStmt>();
    ret_stmt_node->value_node_ = ret_value_expr;

    return ret_stmt_node;
}

std::shared_ptr<AstNode> Parser::ParseDeclStmt(bool is_global) {
    // Extract the BASE TYPE of variable declared.
    // For example:
    //     1. `int x, y, z;` => `int`
    //     2. `int** p;` => `int`
    //     3. `int ar[1][2][3];` => `int`
    std::shared_ptr<CType> variable_base_type = ParseDeclSpec();

    if (token_.GetType() == TokenType::kSemi) {
        Consume(TokenType::kSemi);
        return nullptr;
    }

    auto decl_stmt = std::make_shared<DeclStmt>();

    while (token_.GetType() != TokenType::kSemi) {
        decl_stmt->nodes_.emplace_back(ParseDeclarator(variable_base_type, is_global));
        if (token_.GetType() == TokenType::kComma) {
            Advance();
        }
    }

    // Don't forget me!
    Consume(TokenType::kSemi);

    return decl_stmt;
}

std::shared_ptr<AstNode> Parser::ParseIfStmt() {
    Consume(TokenType::kIf);

    Consume(TokenType::kLParent);
    auto cond_expr = ParseExpr();
    Consume(TokenType::kRParent);

    auto then_stmt = ParseStmt();

    std::shared_ptr<AstNode> else_stmt = nullptr;
    if (token_.GetType() == TokenType::kElse) {
        Consume(TokenType::kElse);
        else_stmt = ParseStmt();
    }

    return sema_.SemaIfStmtNode(cond_expr, then_stmt, else_stmt);
}

std::shared_ptr<AstNode> Parser::ParseBlockStmt() {
    auto block_stmt = std::make_shared<BlockStmt>();

    Consume(TokenType::kLBrace);
    sema_.EnterScope();
    
    while (token_.GetType() != TokenType::kRBrace) {
        auto stmt = ParseStmt();
        if (stmt != nullptr) {
            block_stmt->nodes_.emplace_back(stmt);
        }
    }
    
    sema_.ExitScope();
    Consume(TokenType::kRBrace);

    return block_stmt;
}

std::shared_ptr<AstNode> Parser::ParseForStmt() {
    Consume(TokenType::kFor);
    Consume(TokenType::kLParent);

    // For statement has its own scope.
    sema_.EnterScope();

    // Create forstmt node.
    auto for_stmt_node = std::make_shared<ForStmt>();
    // Record loop statement node, 
    // so that it can be breaked or continued by code in its body.
    AddBreakedAbleNode(for_stmt_node);
    AddContinuedAbleNode(for_stmt_node);

    std::shared_ptr<AstNode> init_node = nullptr;
    std::shared_ptr<AstNode> cond_node = nullptr;
    std::shared_ptr<AstNode> inc_node = nullptr;
    std::shared_ptr<AstNode> body_node = nullptr;

    // Build all the sub nodes of forstmt node.
    if (IsTypeName(token_)) {
        init_node = ParseDeclStmt(false);
    } else {
        init_node = ParseExprStmt();
    }

    cond_node = ParseExprStmt();

    if (token_.GetType() != TokenType::kRParent) {
        inc_node = ParseExpr();
    }
    Consume(TokenType::kRParent);

    body_node = ParseStmt();

    // Finish forstmt node.
    for_stmt_node->init_node_ = init_node;
    for_stmt_node->cond_node_ = cond_node;
    for_stmt_node->inc_node_ = inc_node;
    for_stmt_node->body_node_ = body_node;

    // Don't forget to exit the scope of for statement!
    sema_.ExitScope();
    // Don't forget to remove the record of loop statement node!
    RemoveBreakedAbleNode(for_stmt_node);
    RemoveContinuedAbleNode(for_stmt_node);

    return for_stmt_node;
}

std::shared_ptr<AstNode> Parser::ParseBreakStmt() {
    if (breaked_able_nodes_.empty()) {
        GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()),
                               Diag::kErrBreakStmt);
    }

    Consume(TokenType::kBreak);
    auto node = std::make_shared<BreakStmt>();
    node->target_ = breaked_able_nodes_.back();
    Consume(TokenType::kSemi);
    return node;
}

std::shared_ptr<AstNode> Parser::ParseContinueStmt() {
    if (continued_able_nodes_.empty()) {
        GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()),
                               Diag::kErrContinueStmt);
    }

    Consume(TokenType::kContinue);
    auto node = std::make_shared<ContinueStmt>();
    node->target_ = continued_able_nodes_.back();
    Consume(TokenType::kSemi);
    return node;
}

std::shared_ptr<AstNode> Parser::ParseExprStmt() {
    if (token_.GetType() == TokenType::kSemi) {
        Advance();
        return nullptr;
    }

    auto expr = ParseExpr();
    Consume(TokenType::kSemi);
    return expr;
}

std::shared_ptr<AstNode> Parser::ParseExpr() {
    auto left = ParseAssignExpr();
    while (token_.GetType() == TokenType::kComma) {
        Consume(TokenType::kComma);
        auto right = ParseAssignExpr();
        left = sema_.SemaBinaryExprNode(left, right, BinaryOpCode::kComma);
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseAssignExpr() {
    auto left = ParseConditionalExpr();
    if (!CurrentTokenIsAssignOperator()) {
        return left;
    }

    BinaryOpCode op;
    switch (token_.GetType()) {
        case TokenType::kEqual:
            op = BinaryOpCode::kAssign;
            break;
        case TokenType::kPlusEqual:
            op = BinaryOpCode::kAddAssign;
            break;
        case TokenType::kMinusEqual:
            op = BinaryOpCode::kSubAssign;
            break;
        case TokenType::kStarEqual:
            op = BinaryOpCode::kMulAssign;
            break;
        case TokenType::kSlashEqual:
            op = BinaryOpCode::kDivAssign;
            break;
        case TokenType::kPercentEqual:
            op = BinaryOpCode::kModAssign;
            break;
        case TokenType::kLessLessEqual:
            op = BinaryOpCode::kLeftShiftAssign;
            break;
        case TokenType::kGreaterGreaterEqual:
            op = BinaryOpCode::kRightShiftAssign;
            break;
        case TokenType::kAmpEqual:
            op = BinaryOpCode::kBitwiseAndAssign;
            break;
        case TokenType::kPipeEqual:
            op = BinaryOpCode::kBitwiseOrAssign;
            break;
        case TokenType::kCaretEqual:
            op = BinaryOpCode::kBitwiseXorAssign;
            break;
    }
    Advance();

    auto right = ParseAssignExpr();
    return sema_.SemaBinaryExprNode(left, right, op);
}

// Process something like `a ? b : c`
std::shared_ptr<AstNode> Parser::ParseConditionalExpr() {
    auto cond_node = ParseLogOrExpr();
    if (token_.GetType() != TokenType::kQuestion) {
        return cond_node;
    }
    Token tmp = token_;
    Consume(TokenType::kQuestion);

    auto then_node = ParseExpr();
    Consume(TokenType::kColon);
    auto els_node = ParseConditionalExpr();

    return sema_.SemaTernaryExprNode(cond_node, then_node, els_node, tmp);
}

// Process "==" or "!=" expression.
std::shared_ptr<AstNode> Parser::ParseEqualExpr() {
    auto left_expr = ParseRelationalExpr();
    while (token_.GetType() == TokenType::kEqualEqual 
           || token_.GetType() == TokenType::kNotEqual) {
        BinaryOpCode op;
        if (token_.GetType() == TokenType::kEqualEqual) {
            op = BinaryOpCode::kEqualEqual;
        } else {
            op = BinaryOpCode::kNotEqual;
        }

        Advance();

        auto right_expr = ParseRelationalExpr();
        left_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);
    }

    return left_expr;
}

// Process "<", ">", "<=" or ">=" expression.
std::shared_ptr<AstNode> Parser::ParseRelationalExpr() {
    auto left_expr = ParseBitShiftExpr();

#define TOKEN_TYPE_IS(type) (token_.GetType() == type)

    while (TOKEN_TYPE_IS(TokenType::kLess) 
           || TOKEN_TYPE_IS(TokenType::kGreater)
           || TOKEN_TYPE_IS(TokenType::kLessEqual)
           || TOKEN_TYPE_IS(TokenType::kGreaterEqual))
    {
        BinaryOpCode op;
        switch (token_.GetType()) {
            case TokenType::kLess:
                op = BinaryOpCode::kLess;
                break;
            case TokenType::kGreater:
                op = BinaryOpCode::kGreater;
                break;
            case TokenType::kLessEqual:
                op = BinaryOpCode::kLessEqual;
                break;
            case TokenType::kGreaterEqual:
                op = BinaryOpCode::kGreaterEqual;
                break;
            default:
                assert(0);
        }

        Advance();

        auto right_expr = ParseBitShiftExpr();
        left_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);
    }

    return left_expr;

#undef TOKEN_TYPE_IS
}

// Process "+" or "-" expression.
std::shared_ptr<AstNode> Parser::ParseAddExpr() {
    auto left_expr = ParseMultiExpr();
    while (token_.GetType() == TokenType::kPlus 
           || token_.GetType() == TokenType::kMinus) {
        BinaryOpCode op;
        if (token_.GetType() == TokenType::kPlus) {
            op = BinaryOpCode::kAdd;
        } else {
            op = BinaryOpCode::kSub;
        }

        Advance();

        auto right_expr = ParseMultiExpr();
        left_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);
    }

    return left_expr;
}

// Process "*" or "/" expression.
std::shared_ptr<AstNode> Parser::ParseMultiExpr() {
    auto left_expr = ParseUnaryExpr();
    while (token_.GetType() == TokenType::kStar 
           || token_.GetType() == TokenType::kSlash
           || token_.GetType() == TokenType::kPercent) 
    {
        BinaryOpCode op;
        switch (token_.GetType()) {
            case TokenType::kStar:
                op = BinaryOpCode::kMul;
                break;
            case TokenType::kSlash:
                op = BinaryOpCode::kDiv;
                break;
            case TokenType::kPercent:
                op = BinaryOpCode::kMod;
                break;
        }

        Advance();

        auto right_expr = ParseUnaryExpr();
        left_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseUnaryExpr() {
    if (!CurrentTokenIsUnaryOperator()) {
        return ParsePostFixExpr();
    }

    if (token_.GetType() == TokenType::kSizeof) {
        bool is_type_name = false;
        Consume(TokenType::kSizeof);

        if (token_.GetType() == TokenType::kLParent) {
            lexer_.SaveState();
            {
                Token next;
                lexer_.GetNextToken(next);
                if (IsTypeName(next)) {
                    is_type_name = true;
                }
            }
            lexer_.RestoreState();
        }

        auto node = std::make_shared<SizeofExpr>();
        if (is_type_name) {
            Consume(TokenType::kLParent);
            auto ctype = ParseTypeName();
            Consume(TokenType::kRParent);
            return sema_.SemaSizeofExprNode(nullptr, ctype);
        } else {
            return sema_.SemaSizeofExprNode(ParseUnaryExpr(), nullptr);
        }

        return node;
    }

    UnaryOpCode op;
    switch (token_.GetType()) {
        case TokenType::kPlus:
            op = UnaryOpCode::kPositive;
            break;
        case TokenType::kMinus:
            op = UnaryOpCode::kNegative;
            break;
        case TokenType::kPlusPlus:
            op = UnaryOpCode::kSelfIncreasing;
            break;
        case TokenType::kMinusMinus:
            op = UnaryOpCode::kSelfDecreasing;
            break;
        case TokenType::kAmp:
            op = UnaryOpCode::kAddress;
            break;
        case TokenType::kStar:
            op = UnaryOpCode::kDereference;
            break;
        case TokenType::kTilde:
            op = UnaryOpCode::kBitwiseNot;
            break;
        case TokenType::kNot:
            op = UnaryOpCode::kLogicalNot;
            break;
        default:
            llvm::errs() << "Unknown token type: " 
                         << static_cast<int>(token_.GetType()) 
                         << "\n";
    }

    // Consume operator token
    Advance();

    Token tmp = token_;
    auto sub_node = ParseUnaryExpr();

    return sema_.SemaUnaryExprNode(sub_node, op, tmp);
}

std::shared_ptr<AstNode> Parser::ParsePostFixExpr() {
    auto left = ParsePrimaryExpr();
    while (true) {
        Token tmp = token_;
        switch (token_.GetType()) {
            case TokenType::kPlusPlus: {
                Advance();
                left = sema_.SemaPostIncExprNode(left, tmp);
                continue;                
            }
            case TokenType::kMinusMinus: {
                Advance();
                left = sema_.SemaPostDecExprNode(left, tmp);
                continue;
            }
            case TokenType::kLBracket: {
                Token tmp = token_;
                Consume(TokenType::kLBracket);
                auto index_node = ParseExpr();
                Consume(TokenType::kRBracket);
                left = sema_.SemaPostSubscriptExprNode(left, index_node, tmp);
                continue;
            }
            case TokenType::kDot : {
                Token op_token = token_;
                Consume(TokenType::kDot);
                Token member_token = token_;
                Consume(TokenType::kIdentifier);
                left = sema_.SemaPostMemberDotExprNode(left, op_token, member_token);
                continue;
            }
            case TokenType::kArrow: {
                Token op_token = token_;
                Consume(TokenType::kArrow);
                Token member_token = token_;
                Consume(TokenType::kIdentifier);
                left = sema_.SemaPostMemberArrowExprNode(left, op_token, member_token);
                continue;
            }
            case TokenType::kLParent: {
                Consume(TokenType::kLParent);

                int i = 0;
                std::vector<std::shared_ptr<AstNode>> arg_nodes;
                while (token_.GetType() != TokenType::kRParent) {
                    if (0 < i && token_.GetType() == TokenType::kComma) {
                        Consume(TokenType::kComma);
                    }
                    ++i;
                    // NOTE:
                    // Here we use `ParseAssignExpr` to compute the value of single expression,
                    // instead of `ParseExpr`, which will process the dot(.) operator.
                    arg_nodes.emplace_back(ParseAssignExpr());
                }

                left = sema_.SemaPostFuncCallExprNode(left, arg_nodes);
                Consume(TokenType::kRParent);
                continue;
            }
        }
        break;
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseLogOrExpr() {
    auto left_expr = ParseLogAndExpr();
    while (token_.GetType() == TokenType::kPipePipe)  {
        Advance();

        auto right_expr = ParseLogAndExpr();
        left_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kLogicalOr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseLogAndExpr() {
    auto left_expr = ParseBitOrExpr();
    while (token_.GetType() == TokenType::kAmpAmp)  {
        Advance();

        auto right_expr = ParseBitOrExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kLogicalAnd);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseBitOrExpr() {
    auto left_expr = ParseBitXorExpr();
    while (token_.GetType() == TokenType::kPipe)  {
        Advance();

        auto right_expr = ParseBitXorExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kBitwiseOr);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseBitXorExpr() {
    auto left_expr = ParseBitAndExpr();
    while (token_.GetType() == TokenType::kCaret)  {
        Advance();

        auto right_expr = ParseBitAndExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kBitwiseXor);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseBitAndExpr() {
    auto left_expr = ParseEqualExpr();
    while (token_.GetType() == TokenType::kAmp)  {
        Advance();

        auto right_expr = ParseEqualExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kBitwiseAnd);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseBitShiftExpr() {
    auto left_expr = ParseAddExpr();
    while (token_.GetType() == TokenType::kLessLess 
           || token_.GetType() == TokenType::kGreaterGreater) 
    {
        BinaryOpCode op;
        switch (token_.GetType()) {
            case TokenType::kLessLess:
                op = BinaryOpCode::kLeftShift;
                break;
            case TokenType::kGreaterGreater:
                op = BinaryOpCode::kRightShift;
                break;
        }

        Advance();

        auto right_expr = ParseAddExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, op);

        left_expr = std::move(binary_expr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParsePrimaryExpr() {
    if (token_.GetType() == TokenType::kLParent) {
        Consume(TokenType::kLParent);
        auto sub_expr = ParseExpr();
        Consume(TokenType::kRParent);
        return sub_expr;
    }

    if (token_.GetType() == TokenType::kIdentifier) {
        auto access_expr = sema_.SemaVariableAccessNode(token_);
        Advance();
        return access_expr;
    }
    
    Expect(TokenType::kNumber);
    auto number_expr = sema_.SemaNumberExprNode(token_, token_.GetCType());
    Advance();

    return number_expr;
}

// Some examples:
//  1. sizeof(int);
//  2. sizeof(int[5][6]);
//  3. sizeof(int**);
//  4. sizeof(int*[5][6]);
std::shared_ptr<CType> Parser::ParseTypeName() {
    // Get the base type and consume the corresponding token.
    auto base_type = ParseDeclSpec();
    
    // Process pointer typename
    while (token_.GetType() == TokenType::kStar) {
        base_type = std::make_shared<CPointerType>(base_type);
        Consume(TokenType::kStar);
    }

    Token dummy = token_;
    base_type = ParseDirectDeclaratorSuffix(dummy, base_type, false);

    return base_type;
}

bool Parser::Expect(TokenType token_type) {
    if (token_.GetType() == token_type) {
        return true;
    }
    GetDiagEngine().Report(
            llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()),
            Diag::kErrExpected,
            Token::GetSpellingText(token_type),
            token_.GetContent());
    return token_.GetType() == token_type;
}

bool Parser::Consume(TokenType token_type) {
    if (Expect(token_type)) {
        Advance();
        return true;
    }
    return false;
}

void Parser::Advance() {
    lexer_.GetNextToken(token_);
}

bool Parser::CurrentTokenIsAssignOperator() const {
    return (token_.GetType() == TokenType::kEqual ||
            token_.GetType() == TokenType::kPlusEqual ||
            token_.GetType() == TokenType::kMinusEqual ||
            token_.GetType() == TokenType::kLessLessEqual ||
            token_.GetType() == TokenType::kGreaterGreaterEqual ||
            token_.GetType() == TokenType::kAmpEqual ||
            token_.GetType() == TokenType::kPipeEqual ||
            token_.GetType() == TokenType::kCaretEqual ||
            token_.GetType() == TokenType::kStarEqual ||
            token_.GetType() == TokenType::kPercentEqual ||
            token_.GetType() == TokenType::kSlashEqual);
}

bool Parser::CurrentTokenIsUnaryOperator() const {
    return (token_.GetType() == TokenType::kPlusPlus ||
            token_.GetType() == TokenType::kMinusMinus ||
            token_.GetType() == TokenType::kAmp ||
            token_.GetType() == TokenType::kStar ||
            token_.GetType() == TokenType::kPlus ||
            token_.GetType() == TokenType::kMinus ||
            token_.GetType() == TokenType::kTilde ||
            token_.GetType() == TokenType::kNot ||
            token_.GetType() == TokenType::kSizeof);
}
