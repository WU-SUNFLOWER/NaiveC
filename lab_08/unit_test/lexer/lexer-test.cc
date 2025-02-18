// Copyright 2025 WU-SUNFLOWER. All rights reserved.
// Use of this source code is governed by a GPL-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "lexer.h"

class LexerTest : public ::testing::Test {
 public:
    void SetUp() override {
        static auto buf = llvm::MemoryBuffer::getFile("./test_set/lexer-01.txt");
        if (!buf) {
            llvm::errs() << "can't open file!!!\n";
            return;
        }

        llvm::SourceMgr mgr;
        DiagEngine diagEngine(mgr);

        mgr.AddNewSourceBuffer(std::move(*buf), llvm::SMLoc());

        lexer = new Lexer(mgr, diagEngine);
    }

    void TearDown() override {
        delete lexer;
    }
    Lexer *lexer;
};

/*
int aa, b = 4;
aa=1 ;
*/

TEST_F(LexerTest, NextToken) {
    std::vector<Token> expectedVec, curVec;

    expectedVec.push_back(Token{TokenType::kInt, 1, 1});
    expectedVec.push_back(Token{TokenType::kIdentifier, 1, 5});
    expectedVec.push_back(Token{TokenType::kComma, 1, 7});
    expectedVec.push_back(Token{TokenType::kIdentifier, 1, 9});
    expectedVec.push_back(Token{TokenType::kEqual, 1, 11});
    expectedVec.push_back(Token{TokenType::kNumber, 1, 13});
    expectedVec.push_back(Token{TokenType::kSemi, 1, 14});
    expectedVec.push_back(Token{TokenType::kIdentifier, 2, 1});
    expectedVec.push_back(Token{TokenType::kEqual, 2, 3});
    expectedVec.push_back(Token{TokenType::kNumber, 2, 4});
    expectedVec.push_back(Token{TokenType::kSemi, 2, 6});

    Token tok;
    while (true) {
        lexer->GetNextToken(tok);
        if (tok.GetType() == TokenType::kEOF)
            break;
        curVec.push_back(tok);
    }

    ASSERT_EQ(expectedVec.size(), curVec.size());
    for (int i = 0; i < expectedVec.size(); i++) {
        const auto &expected_tok = expectedVec[i];
        const auto &cur_tok = curVec[i];

        EXPECT_EQ(expected_tok.GetType(), cur_tok.GetType());
        EXPECT_EQ(expected_tok.GetRow(), cur_tok.GetRow());
        EXPECT_EQ(expected_tok.GetCol(), cur_tok.GetCol());
    }
}
