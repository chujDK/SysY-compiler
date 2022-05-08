// a symbol table should be able:
// 1. add a symbol
// 2. delete a symbol
// 3. search a symbol
// 4. enter a scope
// 5. exit a scope
// 6. search a symbol in the current scope
// 7. search a symbol in the global scope
// 8. set the init value of a symbol
// 9. get the init value of a symbol
#include <gtest/gtest.h>

#include <cstring>
#include <memory>

#include "syparse.h"
#include "sysymbol_table.h"
#include "sytype.h"

TEST(SymbolTableTest, ident_test) {
    SymbolTableAPI* symbol_table = (SymbolTableAPI*)new SymbolTable();

    auto ident = AstNodePool::get(SyAstType::IDENT, 0, "a");  // int a;
    symbol_table->addSymbol(ident->getLiteral(), SyAstType::VAL_TYPE_INT, 0,
                            false);
    EXPECT_NE(symbol_table->searchTable("a"), nullptr);         // searched
    EXPECT_NE(symbol_table->searchCurrentScope("a"), nullptr);  // searched
    EXPECT_EQ(symbol_table->searchTable("a")->getType(),
              SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(symbol_table->searchTable("a")->isConst(), false);

    EXPECT_EQ(symbol_table->searchTable("b"), nullptr);  // no searched

    ident      = AstNodePool::get(SyAstType::IDENT, 0, "b");
    Value* val = new Value();
    val->i32   = 0xDEADBEEF;  // int b = 0xDEADBEEF;
    symbol_table->enterScope();
    symbol_table
        ->addSymbol(ident->getLiteral(), SyAstType::VAL_TYPE_INT, 0, false)
        ->setInitVal(val);
    EXPECT_EQ(val, nullptr);

    EXPECT_NE(symbol_table->searchTable("b"), nullptr);  // searched
    EXPECT_EQ(symbol_table->searchTable("b")->getType(),
              SyAstType::VAL_TYPE_INT);
    ASSERT_NE(symbol_table->searchTable("b")->getInitVal(), nullptr);
    EXPECT_EQ(symbol_table->searchTable("b")->getInitVal()->i32, 0xDEADBEEF);
    EXPECT_EQ(symbol_table->searchTable("b")->isConst(), false);

    ident = AstNodePool::get(SyAstType::IDENT, 0, "c");
    symbol_table->addGlobalSymbol(ident->getLiteral(),
                                  SyAstType::VAL_TYPE_INT_ARRAY, 0x100, true);

    ArrayMemoryPtr array_c = std::dynamic_pointer_cast<ArrayMemoryAPI>(
        symbol_table->searchTable("c"));

    ASSERT_NE(array_c, nullptr);

    array_c->setDimension(2);
    array_c->setSizeForDimension(0, 0x8);
    array_c->setSizeForDimension(1, 0x20);
    val = new Value[0x100];
    memset(val, 0, 0x100 * sizeof(Value));
    val[0].i32         = 0xDEADBEEF;
    val[0x100 - 1].i32 = 0x13377331;
    // const int c[0x8][0x20] && c[0][0] = 0xDEADBEEF && c[0x7][0x1F] =
    // 0x13377331;
    array_c->setInitVal(val);
    EXPECT_EQ(val, nullptr);

    EXPECT_EQ(array_c->getType(), SyAstType::VAL_TYPE_INT_ARRAY);
    EXPECT_EQ(array_c->isConst(), true);
    ASSERT_NE(array_c, nullptr);
    EXPECT_EQ(array_c->getDimension(), 2);
    EXPECT_EQ(array_c->getSizeForDimension(0), 0x8);
    EXPECT_EQ(array_c->getSizeForDimension(1), 0x20);
    EXPECT_EQ(array_c->getInitVal()[0].i32, 0xDEADBEEF);
    EXPECT_EQ(array_c->getInitVal()[0x100 - 1].i32, 0x13377331);

    symbol_table->exitScope();
    EXPECT_EQ(symbol_table->searchTable("b"), nullptr);  // no searched
}

// a function table should be able:
// 1. add a function
// 2. search a function
// 3. get a function's argument list
// 4. get a function's body
TEST(SymbolTableTest, function_test) {
    FunctionTableAPI* function_table = (FunctionTableAPI*)new FunctionTable();

    // int foo(int a, float b)
    function_table->addFunction("foo", SyAstType::VAL_TYPE_INT);
    auto ident_a =
        IdentMemory::AllocMemoryForIdent(SyAstType::VAL_TYPE_INT, 0, false);
    function_table->addFunctionArg("foo", SyAstType::VAL_TYPE_INT, "a",
                                   ident_a);

    auto ident_b =
        IdentMemory::AllocMemoryForIdent(SyAstType::VAL_TYPE_FLOAT, 0, false);
    function_table->addFunctionArg("foo", SyAstType::VAL_TYPE_FLOAT, "b",
                                   ident_b);

    auto [searched, foo] = function_table->searchFunction("foo");
    ASSERT_EQ(searched, true);
    auto [foo_type, foo_name, foo_args, body] = foo;
    EXPECT_EQ(foo_type, SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(foo_name, "foo");
    EXPECT_EQ(foo_args.size(), 2);
    EXPECT_EQ(std::get<0>(foo_args[0]), SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(std::get<1>(foo_args[0]), "a");
    EXPECT_EQ(std::get<0>(foo_args[1]), SyAstType::VAL_TYPE_FLOAT);
    EXPECT_EQ(std::get<1>(foo_args[1]), "b");
}