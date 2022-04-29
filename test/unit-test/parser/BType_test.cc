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

class ParserTest : private Parser {
   public:
    using Parser::Parser;
    void testBType() {
        auto btype = BType();
        BTypeVisitor visitor;
        btype->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::TYPE_INT);
    }
};

TEST(BTypeTest, type_test) {
    const char* test_input = "int ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserTest parser(cs);
    parser.testBType();
}