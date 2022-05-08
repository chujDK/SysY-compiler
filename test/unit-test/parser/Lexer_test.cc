#include <gtest/gtest.h>

#include "sydebug.h"
#include "syparse.h"

TEST(LexerTest, test_float) {
    const char* test_input = "float int float1234 1234. ";

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
}