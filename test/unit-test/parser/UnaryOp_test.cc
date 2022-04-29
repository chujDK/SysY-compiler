#include <gtest/gtest.h>

#include "sy.h"
#include "sydebug.h"
#include "syparse.h"

class UnaryOpVisitor : public AstNodeBase::AstNodeVisitor {
   public:
    SyAstType type_return_;
    void visitUnaryOp(UnaryOpAstNode& node) override {
        auto op      = node.op();
        type_return_ = op->getAstType();
    }
};
class UnaryOpTest : public Parser {
   public:
    using Parser::Parser;
    void testUnaryOp() {
        auto unary_op = UnaryOp();
        UnaryOpVisitor visitor;
        unary_op->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::ALU_ADD);  // '+'

        unary_op = UnaryOp();
        unary_op->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::ALU_SUB);  // '-'

        unary_op = UnaryOp();
        unary_op->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::LOGIC_NOT);  // '!'
    }
};

TEST(UnaryOpTest, unary_op_test) {
    const char* test_input = "+-! ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
}