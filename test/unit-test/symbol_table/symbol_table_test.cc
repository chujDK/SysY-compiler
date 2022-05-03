// a symbol table should be able:
// 1. add a symbol
// 2. delete a symbol
// 3. search a symbol
// 4. enter a scope
// 5. exit a scope
// 6. search a symbol in the current scope
// 7. search a symbol in the global scope
// 8. get a symbol's memory (which means its inital value)
#include <gtest/gtest.h>

#include <cstring>
#include <memory>

#include "syparse.h"
#include "sysymbol_table.h"

TEST(SymbolTableTest, ident_test) {
    SymbolTableAPI* symbol_table = (SymbolTableAPI*)new SymbolTable();

    auto ident = AstNodePool::get(SyAstType::IDENT, 0, "a");  // int a;
    symbol_table->addSymbol(ident->getLiteral(), SyAstType::VAL_TYPE_INT,
                            nullptr, 0, false);
    EXPECT_NE(symbol_table->searchTable("a"), nullptr);         // searched
    EXPECT_NE(symbol_table->searchCurrentScope("a"), nullptr);  // searched
    EXPECT_EQ(symbol_table->searchTable("a")->getType(),
              SyAstType::VAL_TYPE_INT);
    EXPECT_EQ(symbol_table->searchCurrentScope("a")->getMem(),
              nullptr);  // no init val

    EXPECT_EQ(symbol_table->searchTable("b"), nullptr);  // no searched
    ident      = AstNodePool::get(SyAstType::IDENT, 0, "b");
    Value* val = new Value();
    val->i32   = 0xDEADBEEF;  // int b = 0xDEADBEEF;
    symbol_table->enterScope();
    symbol_table->addSymbol(ident->getLiteral(), SyAstType::VAL_TYPE_INT, val,
                            0, false);
    // remember this
    val = nullptr;

    EXPECT_NE(symbol_table->searchTable("b"), nullptr);  // searched
    EXPECT_EQ(symbol_table->searchTable("b")->getType(),
              SyAstType::VAL_TYPE_INT);
    ASSERT_NE(symbol_table->searchTable("b")->getMem(), nullptr);
    EXPECT_EQ(symbol_table->searchTable("b")->getMem()->i32, 0xDEADBEEF);

    ident = AstNodePool::get(SyAstType::IDENT, 0, "c");
    val   = new Value[0x100];
    memset(val, 0, 0x100 * sizeof(Value));
    val[0].i32     = 0xDEADBEEF;
    val[0x100].i32 = 0x13377331;  // int c[0x10][0x10]; c[0][0] = 0xDEADBEEF;
                                  // c[0xF][0xF] = 0x13377331;
    symbol_table->addGlobalSymbol(ident->getLiteral(),
                                  SyAstType::VAL_TYPE_INT_ARRAY, val, 0x100, 0);

    ArrayMemoryPtr array_c = std::dynamic_pointer_cast<ArrayMemoryAPI>(
        symbol_table->searchTable("c"));
    val = nullptr;

    ASSERT_NE(array_c, nullptr);
}

// a function table should be able:
// 1. add a function
// 2. search a function
// 3. get a function's argument list