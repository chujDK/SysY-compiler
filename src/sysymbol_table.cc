#include <cassert>
#include "sysymbol_table.h"

IdentMemoryPtr SymbolTable::searchTable(TokenPtr ident) {
    for (int i = current_scope_; i >= 0; i--) {
        if (symbol_table_[i].find(ident->literal_) != symbol_table_[i].end()) {
            return symbol_table_[i][ident->literal_];
        }
    }
    return nullptr;
}

IdentMemoryPtr SymbolTable::searchCurrentScope(TokenPtr ident) {
    if (symbol_table_[current_scope_].find(ident->literal_) != symbol_table_[current_scope_].end()) {
        return symbol_table_[current_scope_][ident->literal_];
    }
    return nullptr;
}

IdentMemoryPtr IdentMemory::AllocMemoryForIdent(TokenPtr ident) {
    std::shared_ptr<IdentMemoryAPI> mem(nullptr);
    switch (ident->ebnf_type_)
    {
    case SyEbnfType::TYPE_INT:
        mem.reset((IdentMemoryAPI*) new IdentMemory(sizeof(int)));
        return mem;
    case SyEbnfType::TYPE_INT_ARRAY:
        mem.reset((IdentMemoryAPI*) new IdentMemory(sizeof(int) * ident->u.array_size_));
        return mem;
    default:
        // shouldn't reach here
        // cause a bug
        assert(1!=1);
        return nullptr;
    }
}

inline IdentMemoryPtr SymbolTable::addSymbol(TokenPtr ident) {
    // if this ident isn't exist in the current scope, delete won't hurt
    deleteSymbol(ident);
    IdentMemoryPtr mem = IdentMemory::AllocMemoryForIdent(ident);
    symbol_table_[current_scope_][ident->literal_] = mem;
    return mem;
}

inline IdentMemoryPtr SymbolTable::addGlobalSymbol(TokenPtr ident) {
    // if this ident isn't exist in the current scope, delete won't hurt
    symbol_table_[0].erase(ident->literal_);
    IdentMemoryPtr mem = IdentMemory::AllocMemoryForIdent(ident);
    symbol_table_[0][ident->literal_] = mem;
    return mem;
}

// note that this function can still work when ident isn't exist in the current scope
inline void SymbolTable::deleteSymbol(TokenPtr ident) {
    symbol_table_[current_scope_].erase(ident->literal_);
}

inline void SymbolTable::enterScope() {
    symbol_table_.push_back(std::map<std::string, IdentMemoryPtr>());
    current_scope_++;
}

inline void SymbolTable::exitScope() {
    symbol_table_.pop_back();
    current_scope_--;
}