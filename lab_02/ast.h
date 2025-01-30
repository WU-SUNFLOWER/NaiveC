// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef AST_H_
#define AST_H_

#include <memory>
#include <vector>

class Program;
class Expression;
class BinaryExpression;
class FactorExpression;

class Visitor {
 public:
    virtual ~Visitor() {}

    virtual void VisitProgram(Program*) = 0;
    virtual void VisitBinaryExpr(BinaryExpression*) = 0;
    virtual void VisitFactorExpr(FactorExpression*) = 0;
};

class Expression {
 public:
    virtual ~Expression() {}

    virtual void Accept(Visitor* vis) = 0;
};

enum class OpCode {
    kAdd,
    kSub,
    kMul,
    kDiv,
};

class BinaryExpression : public Expression {
 public:
    OpCode op_;
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;

    void Accept(Visitor* vis) override {
        vis->VisitBinaryExpr(this);
    }
};

class FactorExpression : public Expression{
 public:
    int number_;

    void Accept(Visitor* vis) override {
        vis->VisitFactorExpr(this);
    }
};

class Program {
 public:
    std::vector<std::shared_ptr<Expression>> expr_vec_;
};

#endif  // AST_H_
