// Copyright 2024 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#ifndef AST_H_
#define AST_H_

#include <memory>
#include <vector>

class Expression {
 public:
    virtual ~Expression() {}
};

enum class OpCode {
    kAdd,
    kSub,
    kMul,
    kDiv,
};

class BinaryExpression {
 public:
    OpCode op_;
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
};

class FactorExpression {
 public:
    int number_;
};

class Program {
 public:
    std::vector<std::shared_ptr<Expression>> expr_vec_;
};

#endif  // AST_H_
