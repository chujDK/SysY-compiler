#include <gtest/gtest.h>

#include <memory>

#include "sy.h"
#include "sydebug.h"
#include "syparse.h"

class ParserGenUnaryExpTest : public Parser {
   public:
    using Parser::Parser;
    void testMulExp() {
        auto mul_exp   = MulExp();
        auto unary_exp = mul_exp->getChildAt(0);
        EXPECT_EQ(unary_exp->getEbnfType(), SyEbnfType::UnaryExp);
        EXPECT_EQ(unary_exp->getChildAt(0)->getEbnfType(),
                  SyEbnfType::PrimaryExp);
        EXPECT_EQ(unary_exp->getChildAt(0)->getChildAt(0)->getEbnfType(),
                  SyEbnfType::LVal);
        EXPECT_EQ(unary_exp->getChildAt(0)
                      ->getChildAt(0)
                      ->getChildAt(0)
                      ->getLiteral(),
                  "a_c");
    }
};

TEST(MulExpTest, gen_unaray_exp_test) {
    const char* test_input = "a_c ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserGenUnaryExpTest parser(cs);
    parser.testMulExp();
}

class ParserGenMulExpTimesUnaryExpTest : public Parser {
   public:
    using Parser::Parser;
    void testMulExp() {
        auto mul_exp   = MulExp();
        auto unary_exp = mul_exp->getChildAt(0)->getChildAt(0);
        EXPECT_EQ(unary_exp->getEbnfType(), SyEbnfType::UnaryExp);
        EXPECT_EQ(unary_exp->getChildAt(0)->getEbnfType(),
                  SyEbnfType::PrimaryExp);
        EXPECT_EQ(unary_exp->getChildAt(0)->getChildAt(0)->getEbnfType(),
                  SyEbnfType::LVal);
        EXPECT_EQ(unary_exp->getChildAt(0)
                      ->getChildAt(0)
                      ->getChildAt(0)
                      ->getLiteral(),
                  "a_c");
        EXPECT_EQ(mul_exp->getChildAt(1)->getAstType(), SyAstType::ALU_MUL);
        EXPECT_EQ(mul_exp->getChildAt(2)
                      ->getChildAt(0)
                      ->getChildAt(0)
                      ->getChildAt(0)
                      ->getLiteral(),
                  "10");
    }
};

TEST(MulExpTest, gen_mul_exp_times_unaray_exp_test) {
    const char* test_input = "a_c * 10 ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserGenUnaryExpTest parser(cs);
    parser.testMulExp();
}