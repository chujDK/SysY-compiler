#include <gtest/gtest.h>

#include "sydebug.h"
#include "syparse.h"

TEST(LexerTest, test_float) {
    const char* test_input =
        "float int float1234 1234 0x1234 0X42 01713 1234.42 42.42 ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    Lexer lexer(cs);
    auto iter = lexer.begin();
    EXPECT_EQ(iter->getLiteral(), "float");
    EXPECT_EQ(iter->getAstType(), SyAstType::TYPE_FLOAT);
    ++iter;
    EXPECT_EQ(iter->getLiteral(), "int");
    EXPECT_EQ(iter->getAstType(), SyAstType::TYPE_INT);
    ++iter;
    EXPECT_EQ(iter->getLiteral(), "float1234");
    EXPECT_EQ(iter->getAstType(), SyAstType::IDENT);
    ++iter;
    EXPECT_EQ(strtod(iter->getLiteral().c_str(), nullptr), 1234);
    EXPECT_EQ(iter->getAstType(), SyAstType::INT_IMM);
    ++iter;
    EXPECT_EQ(strtod(iter->getLiteral().c_str(), nullptr), 0x1234);
    EXPECT_EQ(iter->getAstType(), SyAstType::INT_IMM);
    ++iter;
    EXPECT_EQ(strtod(iter->getLiteral().c_str(), nullptr), 0x42);
    EXPECT_EQ(iter->getAstType(), SyAstType::INT_IMM);
    ++iter;
    EXPECT_EQ(strtod(iter->getLiteral().c_str(), nullptr), 01713);
    EXPECT_EQ(iter->getAstType(), SyAstType::INT_IMM);
    ++iter;
    EXPECT_EQ(strtod(iter->getLiteral().c_str(), nullptr), 1234.42);
    EXPECT_EQ(iter->getAstType(), SyAstType::FLOAT_IMM);
    ++iter;
    EXPECT_EQ(strtod(iter->getLiteral().c_str(), nullptr), 42.42);
    EXPECT_EQ(iter->getAstType(), SyAstType::FLOAT_IMM);
}