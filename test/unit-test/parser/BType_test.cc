#include <gtest/gtest.h>

#include "sy.h"
#include "sydebug.h"
#include "syparse.h"

class BTypeVisitor : public AstNodeBase::AstNodeVisitor {
   public:
    SyAstType type_return_;

    void visitBType(BTypeAstNode& node) override {
        auto type    = node.type();
        type_return_ = type->getAstType();
    }
};

class ParserIntTest : public Parser {
   public:
    using Parser::Parser;
    void testBType() {
        auto btype = BType();
        BTypeVisitor visitor;
        btype->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::TYPE_INT);
    }
};

TEST(BTypeTest, int_type_test) {
    const char* test_input = "int ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserIntTest parser(cs);
    parser.testBType();
}

class ParserFloatTest : public Parser {
   public:
    using Parser::Parser;
    void testBType() {
        auto btype = BType();
        BTypeVisitor visitor;
        btype->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::TYPE_FLOAT);
    }
};

TEST(BTypeTest, float_type_test) {
    const char* test_input = "float ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserFloatTest parser(cs);
    parser.testBType();
}