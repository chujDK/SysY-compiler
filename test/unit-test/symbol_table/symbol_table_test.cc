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

#include "syparse.h"
#include "sysymbol_table.h"

TEST(SymbolTableTest, ident_test) {
    SymbolTableAPI* symbol_table = (SymbolTableAPI*)new SymbolTable();

    auto ident = AstNodePool::get(SyAstType::IDENT, 0, "a");
    symbol_table->addSymbol(ident->getLiteral(), SyAstType::VAL_TYPE_INT,
                            nullptr, 0, false);
}

// a function table should be able:
// 1. add a function
// 2. search a function
// 3. get a function's argument list