#include <gtest/gtest.h>

#include <string>

#include "sy.h"
#include "sydebug.h"

class NumberVisitor : public AstNodeBase::AstNodeVisitor {
   public:
    SyAstType type_return_;
    Value value_return_;

    void visitNumber(NumberAstNode& node) {
        auto token = node.token();
        if (token->getAstType() == SyAstType::INT_IMM) {
            type_return_      = SyAstType::INT_IMM;
            value_return_.i32 = std::atoi(token->getLiteral().c_str());
            return;
        }
        FAIL() << "should not reach here.";
    }
};

class ParserTest : public Parser {
   public:
    using Parser::Parser;
    void testNumber() {
        NumberVisitor visitor;

        auto number = Number();
        number->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::INT_IMM);  // "0x1337BEEF"
        EXPECT_EQ(visitor.value_return_.i32, 0x1337BEEF);

        number = Number();
        number->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::INT_IMM);  // "1234567"
        EXPECT_EQ(visitor.value_return_.i32, 1234567);

        number = Number();
        number->accept(visitor);
        EXPECT_EQ(visitor.type_return_, SyAstType::INT_IMM);  // "04321"
        EXPECT_EQ(visitor.value_return_.i32, 04321);
    }
};

TEST(NumberTest, number_test) {
    const char* test_input = "0x1337BEEF 1234567 04321 ";
    // here we don't test the negative number, because the negative number is
    // represented by unary minus operator with a number in the grammar.
    // out of the int bound is not tested, because it should be an undefined
    // behavior.

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserTest parser(cs);
    parser.testNumber();
}