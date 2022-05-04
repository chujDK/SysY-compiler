#include <gtest/gtest.h>

#include <cstring>
#include <memory>

#include "sydebug.h"
#include "sysemantic.h"

TEST(SemanticAnalysisVisitorTest, DeclTest) {
    const char* test_str =
        "int a, b, c, d;\n\
        const int b_c = 1;\
        int arr_a[0x10][0x20];\
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

    auto a               = symbol_table->searchTable("a");
    auto b               = symbol_table->searchTable("b");
    auto c               = symbol_table->searchTable("c");
    auto d               = symbol_table->searchTable("d");
    auto b_c             = symbol_table->searchTable("b_c");
    ArrayMemoryPtr arr_a = std::dynamic_pointer_cast<ArrayMemoryAPI>(
        symbol_table->searchTable("arr_a"));
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    ASSERT_NE(c, nullptr);
    ASSERT_NE(d, nullptr);
    ASSERT_NE(b_c, nullptr);
    ASSERT_NE(arr_a, nullptr);

    EXPECT_EQ(a->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(a->isConst(), false);
    EXPECT_EQ(b->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(b->isConst(), false);
    EXPECT_EQ(c->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(c->isConst(), false);
    EXPECT_EQ(d->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(d->isConst(), false);

    EXPECT_EQ(b_c->getType(), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(b_c->isConst(), true);

    EXPECT_EQ(arr_a->getType(), SyAstType::VAL_TYPE_INT_ARRAY);
    EXPECT_EQ(arr_a->isConst(), false);
    EXPECT_EQ(arr_a->getDimension(), 2);
    EXPECT_EQ(arr_a->getSizeForDimension(0), 0x10);
    EXPECT_EQ(arr_a->getSizeForDimension(1), 0x20);

    ASSERT_NE(b_c->getInitVal(), nullptr);
    EXPECT_EQ(b_c->getInitVal()->i32, 1);
}