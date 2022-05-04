// a symbol table should be able:
// 1. add a symbol
// 2. delete a symbol
// 3. search a symbol
// 4. enter a scope
// 5. exit a scope
// 6. search a symbol in the current scope
// 7. search a symbol in the global scope
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

    EXPECT_EQ(symbol_table->searchTable("b"), nullptr);  // no searched
    ident = AstNodePool::get(SyAstType::IDENT, 0, "b");
    symbol_table->enterScope();
    symbol_table->addSymbol(ident->getLiteral(), SyAstType::VAL_TYPE_INT, 0,
                            false);
    EXPECT_NE(symbol_table->searchTable("b"), nullptr);  // searched
    EXPECT_EQ(symbol_table->searchTable("b")->getType(),
              SyAstType::VAL_TYPE_INT);

    ident = AstNodePool::get(SyAstType::IDENT, 0, "c");
    symbol_table->addGlobalSymbol(ident->getLiteral(),
                                  SyAstType::VAL_TYPE_INT_ARRAY, 0x100, 0);

    ArrayMemoryPtr array_c = std::dynamic_pointer_cast<ArrayMemoryAPI>(
        symbol_table->searchTable("c"));

    array_c->setDimension(2);
    array_c->setSizeForDimension(0, 0x8);
    array_c->setSizeForDimension(
        1, 0x20);  // int c[0x8][0x20]; c[0][0] = 0xDEADBEEF;
                   // c[0x7][0x1F] = 0x13377331;

    EXPECT_EQ(array_c->getType(), SyAstType::VAL_TYPE_INT_ARRAY);
    ASSERT_NE(array_c, nullptr);
    EXPECT_EQ(array_c->getDimension(), 2);
    EXPECT_EQ(array_c->getSizeForDimension(0), 0x8);
    EXPECT_EQ(array_c->getSizeForDimension(1), 0x20);

    symbol_table->exitScope();
    EXPECT_EQ(symbol_table->searchTable("b"), nullptr);  // no searched
}

// a function table should be able:
// 1. add a function
// 2. search a function
// 3. get a function's argument list