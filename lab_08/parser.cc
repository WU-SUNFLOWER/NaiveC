// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include "parser.h"

#include <cassert>
#include <vector>
#include <memory>
#include <utility>

bool IsTypeName(TokenType token_type) {
    if (token_type == TokenType::kInt) {
        return true;
    }
    return false;
}

bool IsTypeName(Token token) {
    return IsTypeName(token.GetType());
}

Parser::Parser(Lexer& lexer, Sema& sema) : lexer_(lexer), sema_(sema) {
    Advance();
}

std::shared_ptr<Program> Parser::ParseProgram() {
    auto prog = std::make_shared<Program>();
    if (token_.GetType() != TokenType::kEOF) {
        prog->node_ = ParseBlockStmt();
    }
    Consume(TokenType::kEOF);
    return prog;
}

std::shared_ptr<AstNode> Parser::ParseStmt() {
#define TOKEN_TYPE_IS(type) (token_.GetType() == type)

    if (TOKEN_TYPE_IS(TokenType::kSemi)) {
        Advance();
        return nullptr;
    }
    else if (IsTypeName(token_)) {
        return ParseDeclStmt();
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

#undef TOKEN_TYPE_IS

    return ParseExprStmt();
}

std::shared_ptr<CType> Parser::ParseDeclSpec() {
    if (token_.GetType() == TokenType::kInt) {
        Consume(TokenType::kInt);
        return CType::kIntType;
    }
    GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()), Diag::kErrType);
    return nullptr;
}

std::shared_ptr<AstNode> Parser::ParseDeclarator(std::shared_ptr<CType> variable_ctype) {
    // Process pointer variable, like `char** ptr = 1234`.
    while (token_.GetType() == TokenType::kStar) {
        Consume(TokenType::kStar);
        variable_ctype = std::make_shared<CPointerType>(variable_ctype);
    }

    // Now we have `ptr = 1234`.
    auto variable_decl_node = sema_.SemaVariableDeclNode(token_, variable_ctype);
    Consume(TokenType::kIdentifier);

    if (token_.GetType() == TokenType::kEqual) {
        Advance();
        VariableDecl* raw_decl_node = llvm::dyn_cast<VariableDecl>(variable_decl_node.get());
        raw_decl_node->init_node_ = ParseAssignExpr();
    }

    return variable_decl_node;
}

std::shared_ptr<AstNode> Parser::ParseDeclStmt() {
    // Extract the type of variable declared.
    // For example, `int x, y, z;` => `int`
    std::shared_ptr<CType> variable_ctype = ParseDeclSpec();

    if (token_.GetType() == TokenType::kSemi) {
        Consume(TokenType::kSemi);
        return nullptr;
    }

    auto decl_stmt = std::make_shared<DeclStmt>();

    while (token_.GetType() != TokenType::kSemi) {
        decl_stmt->nodes_.emplace_back(ParseDeclarator(variable_ctype));
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
        block_stmt->nodes_.emplace_back(ParseStmt());
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
        init_node = ParseDeclStmt();
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
    /*
    bool is_assign_expr = false;
    bool is_logic_expr = false;

    lexer_.SaveState();
    {
        if (token_.GetType() == TokenType::kIdentifier) {
            Token tmp;
            lexer_.GetNextToken(tmp);
            if (tmp.GetType() == TokenType::kEqual) {
                is_assign_expr = true;
            }
        }
    }
    lexer_.RestoreState();

    // Process "=" assignment expression.
    if (is_assign_expr) {
        return ParseAssignExpr();
    }

    return ParseLogOrExpr();    
    */
    auto left = ParseAssignExpr();
    while (token_.GetType() == TokenType::kComma) {
        Consume(TokenType::kComma);
        auto right = ParseAssignExpr();
        left = sema_.SemaBinaryExprNode(left, right, BinaryOpCode::kComma);
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseAssignExpr() {
    /*
    auto access_node = sema_.SemaVariableAccessNode(token_);
    Consume(TokenType::kIdentifier);

    Token equal_op_token = token_;
    Consume(TokenType::kEqual);

    auto right_node = ParseExpr();
    return sema_.SemaAssignExprNode(equal_op_token, access_node, right_node);    
    */
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
                left = sema_.SemaPostIncExpr(left, tmp);
                continue;                
            }
            case TokenType::kMinusMinus: {
                Advance();
                left = sema_.SemaPostDecExpr(left, tmp);
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
        left_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kLogicOr);
    }
    return left_expr;
}

std::shared_ptr<AstNode> Parser::ParseLogAndExpr() {
    auto left_expr = ParseBitOrExpr();
    while (token_.GetType() == TokenType::kAmpAmp)  {
        Advance();

        auto right_expr = ParseBitOrExpr();
        auto binary_expr = sema_.SemaBinaryExprNode(left_expr, right_expr, BinaryOpCode::kLogicAnd);

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

std::shared_ptr<CType> Parser::ParseTypeName() {
    std::shared_ptr<CType> base_type = nullptr;
    if (token_.GetType() == TokenType::kInt) {
        base_type = CType::kIntType;
    }
    else {
        GetDiagEngine().Report(llvm::SMLoc::getFromPointer(token_.GetRawContentPtr()), Diag::kErrType);
        return nullptr;
    }

    // Consume typename
    Advance();
    
    // Process pointer typename
    while (token_.GetType() == TokenType::kStar) {
        base_type = std::make_shared<CPointerType>(base_type);
        Consume(TokenType::kStar);
    }

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
