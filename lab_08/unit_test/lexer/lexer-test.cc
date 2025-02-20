#include <gtest/gtest.h>
#include "lexer.h"
#include <functional>

bool TestLexerWithContent(llvm::StringRef content, std::function<std::vector<Token>()> callback) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> buf = llvm::MemoryBuffer::getMemBuffer(content, "stdin");
     if (!buf) {
        llvm::errs() << "open file failed!!!\n";
        return false;
    }
    llvm::SourceMgr mgr;
    DiagEngine diagEngine(mgr);
    mgr.AddNewSourceBuffer(std::move(*buf), llvm::SMLoc());
    Lexer lexer(mgr, diagEngine);

    std::vector<Token> expectedVec = callback(), curVec;

    Token tok;
    while (true) {
        lexer.GetNextToken(tok);
        if (tok.GetType() == TokenType::kEOF)
            break;
        curVec.push_back(tok);
    }

    EXPECT_EQ(expectedVec.size(), curVec.size());
    for (int i = 0; i < expectedVec.size(); i++) {
        const auto &expected_tok = expectedVec[i];
        const auto &cur_tok = curVec[i];

        EXPECT_EQ(static_cast<uint8_t>(expected_tok.GetType()), static_cast<uint8_t>(cur_tok.GetType()));
        EXPECT_EQ(expected_tok.GetRow(), cur_tok.GetRow());
        EXPECT_EQ(expected_tok.GetCol(), cur_tok.GetCol());
    }
    return true;
}



TEST(LexerTest, identifier) {
    bool res = TestLexerWithContent("aaaa aA_ aA0_", []()->std::vector<Token> {
        std::vector<Token> expectedVec;
        expectedVec.push_back(Token{TokenType::kIdentifier, 1, 1});
        expectedVec.push_back(Token{TokenType::kIdentifier, 1, 6});
        expectedVec.push_back(Token{TokenType::kIdentifier, 1, 10});
        return expectedVec;
    });
    ASSERT_EQ(res, true);
}

TEST(LexerTest, keyword) {
    bool res = TestLexerWithContent("  int if else for break continue sizeof", []()->std::vector<Token> {
        std::vector<Token> expectedVec;
        expectedVec.push_back(Token{TokenType::kInt, 1, 3});
        expectedVec.push_back(Token{TokenType::kIf, 1, 7});
        expectedVec.push_back(Token{TokenType::kElse, 1, 10});
        expectedVec.push_back(Token{TokenType::kFor, 1, 15});
        expectedVec.push_back(Token{TokenType::kBreak, 1, 19});
        expectedVec.push_back(Token{TokenType::kContinue, 1, 25});
        expectedVec.push_back(Token{TokenType::kSizeof, 1, 34});
        return expectedVec;
    });
    ASSERT_EQ(res, true);
}

TEST(LexerTest, number) {
    bool res = TestLexerWithContent(" 0123 1234 1234222 \n0" , []()->std::vector<Token> {
        std::vector<Token> expectedVec;
        expectedVec.push_back(Token{TokenType::kNumber, 1, 2});
        expectedVec.push_back(Token{TokenType::kNumber, 1, 7});
        expectedVec.push_back(Token{TokenType::kNumber, 1, 12});
        expectedVec.push_back(Token{TokenType::kNumber, 2, 1});
        return expectedVec;
    });
    ASSERT_EQ(res, true);
}

TEST(LexerTest, punctuation) {
    bool res = TestLexerWithContent("+-*/%();,={}==!=< <=> >= || | & && >><<^", []()->std::vector<Token> {
        std::vector<Token> expectedVec;
        expectedVec.push_back(Token{TokenType::kPlus, 1, 1});
        expectedVec.push_back(Token{TokenType::kMinus, 1, 2});
        expectedVec.push_back(Token{TokenType::kStar, 1, 3});
        expectedVec.push_back(Token{TokenType::kSlash, 1, 4});
        expectedVec.push_back(Token{TokenType::kPercent, 1, 5});
        expectedVec.push_back(Token{TokenType::kLParent, 1, 6});
        expectedVec.push_back(Token{TokenType::kRParent, 1, 7});
        expectedVec.push_back(Token{TokenType::kSemi, 1, 8});
        expectedVec.push_back(Token{TokenType::kComma, 1, 9});
        expectedVec.push_back(Token{TokenType::kEqual, 1, 10});
        expectedVec.push_back(Token{TokenType::kLBrace, 1, 11});
        expectedVec.push_back(Token{TokenType::kRBrace, 1, 12});
        expectedVec.push_back(Token{TokenType::kEqualEqual, 1, 13});
        expectedVec.push_back(Token{TokenType::kNotEqual, 1, 15});
        expectedVec.push_back(Token{TokenType::kLess, 1, 17});
        expectedVec.push_back(Token{TokenType::kLessEqual, 1, 19});
        expectedVec.push_back(Token{TokenType::kGreater, 1, 21});
        expectedVec.push_back(Token{TokenType::kGreaterEqual, 1, 23});
        expectedVec.push_back(Token{TokenType::kPipePipe, 1, 26});
        expectedVec.push_back(Token{TokenType::kPipe, 1, 29});
        expectedVec.push_back(Token{TokenType::kAmp, 1, 31});
        expectedVec.push_back(Token{TokenType::kAmpAmp, 1, 33});
        expectedVec.push_back(Token{TokenType::kGreaterGreater, 1, 36});
        expectedVec.push_back(Token{TokenType::kLessLess, 1, 38});
        expectedVec.push_back(Token{TokenType::kCaret, 1, 40});
        return expectedVec;
    });
    ASSERT_EQ(res, true);
}

TEST(LexerTest, unary) {
    bool res = TestLexerWithContent("+-*&++--!~+=-=*=/=%=<<=>>=&=^=|=?:", []()->std::vector<Token> {
        std::vector<Token> expectedVec;
        expectedVec.push_back(Token{TokenType::kPlus, 1, 1});
        expectedVec.push_back(Token{TokenType::kMinus, 1, 2});
        expectedVec.push_back(Token{TokenType::kStar, 1, 3});
        expectedVec.push_back(Token{TokenType::kAmp, 1, 4});
        expectedVec.push_back(Token{TokenType::kPlusPlus, 1, 5});
        expectedVec.push_back(Token{TokenType::kMinusMinus, 1, 7});
        expectedVec.push_back(Token{TokenType::kNot, 1, 9});
        expectedVec.push_back(Token{TokenType::kTilde, 1, 10});

        expectedVec.push_back(Token{TokenType::kPlusEqual, 1, 11});
        expectedVec.push_back(Token{TokenType::kMinusEqual, 1, 13});
        expectedVec.push_back(Token{TokenType::kStarEqual, 1, 15});
        expectedVec.push_back(Token{TokenType::kSlashEqual, 1, 17});
        expectedVec.push_back(Token{TokenType::kPercentEqual, 1, 19});
        expectedVec.push_back(Token{TokenType::kLessLessEqual, 1, 21});
        expectedVec.push_back(Token{TokenType::kGreaterGreaterEqual, 1, 24});
        expectedVec.push_back(Token{TokenType::kAmpEqual, 1, 27});
        expectedVec.push_back(Token{TokenType::kCaretEqual, 1, 29});
        expectedVec.push_back(Token{TokenType::kPipeEqual, 1, 31});
        expectedVec.push_back(Token{TokenType::kQuestion, 1, 33});
        expectedVec.push_back(Token{TokenType::kColon, 1, 34});
        return expectedVec;
    });
}
