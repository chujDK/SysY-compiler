#include <gtest/gtest.h>

#include "sydebug.h"
#include "syparse.h"

// EqExp -> RelExp | EqExp ('==' | '!=') RelExp

class EqExpVisitor : public AstNodeBase::AstNodeVisitor {
    void visitEqExp(EqExpAstNode& node) override {}
};

class ParserTest : public Parser {
   public:
    using Parser::Parser;
    void testEqExp() {
        auto eqexp = EqExp();
        EqExpVisitor visitor;
        // eqexp->accept(visitor);
    }
};

TEST(EqExpTest, eq_exp_test) {
    // EqExp ----> EqExp '!=' RelExp
    //               |          |
    //               |          ----> AddExp
    //               |
    //               ----> EqExp '==' RelExp
    //                       |          |
    //                       |          ----> AddExp
    //                       |
    //                       ----> RelExp
    //                               |
    //                               ----> AddExp
    const char* test_input = "1 == 2 != 2 ";

    CharStream* cs = new CharStream(test_input, strlen(test_input));
    ParserTest parser(cs);
    parser.testEqExp();
}