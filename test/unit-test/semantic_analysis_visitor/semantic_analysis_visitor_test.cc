#include <gtest/gtest.h>

#include <cstring>
#include <memory>

#include "sydebug.h"
#include "sysemantic.h"

TEST(SemanticAnalysisVisitorTest, DeclTest) {
    const char* test_str =
        "int a;\n\
        const int b = 1;\
        int c[0x10][0x20];\
        \xFF";
    CharStream* char_stream = new CharStream(test_str, strlen(test_str));

    Parser parser(char_stream);
    SemanticAnalysisVisitor analyzer;
    while (1) {
        auto root = parser.parse();
        if (root == nullptr) {
            break;
        }
        root->accept(analyzer);
    }

    auto symbol_table = analyzer.symbol_table();

    auto a           = symbol_table->searchTable("a");
    auto b           = symbol_table->searchTable("b");
    ArrayMemoryPtr c = std::dynamic_pointer_cast<ArrayMemoryAPI>(
        symbol_table->searchTable("c"));
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);

    EXPECT_EQ(a->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(a->isConst(), false);
    EXPECT_EQ(b->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(b->isConst(), true);
    EXPECT_EQ(c->getType(), SyAstType::VAL_TYPE_INT_ARRAY);
    EXPECT_EQ(c->isConst(), false);
    EXPECT_EQ(c->getDimension(), 2);
    EXPECT_EQ(c->getSizeForDimension(0), 0x10);
    EXPECT_EQ(c->getSizeForDimension(1), 0x20);
}