#include "sysymbol_table.h"

#include <cassert>
#include <string>
#include <unordered_map>

IdentMemoryPtr SymbolTable::searchTable(std::string ident) {
    for (int i = current_scope_; i >= 0; i--) {
        if (symbol_table_[i].find(ident) != symbol_table_[i].end()) {
            return symbol_table_[i][ident];
        }
    }
    return nullptr;
}

IdentMemoryPtr SymbolTable::searchCurrentScope(std::string ident) {
    if (symbol_table_[current_scope_].find(ident) !=
        symbol_table_[current_scope_].end()) {
        return symbol_table_[current_scope_][ident];
    }
    return nullptr;
}

IdentMemoryPtr IdentMemory::AllocMemoryForIdent(std::string ident,
                                                SyAstType type, Value* init_mem,
                                                int n_elem, bool is_const) {
    std::shared_ptr<IdentMemoryAPI> mem(nullptr);
    switch (type) {
        case SyAstType::VAL_TYPE_INT:
            mem.reset(
                (IdentMemoryAPI*)new IdentMemory(type, is_const, init_mem));
            return mem;
        case SyAstType::VAL_TYPE_INT_ARRAY:
            mem.reset((ArrayMemoryAPI*)new ArrayMemory(n_elem, type, is_const,
                                                       init_mem));
            return mem;
        default:
            // shouldn't reach here
            // cause a bug
            assert(1 != 1);
            return nullptr;
    }
}

IdentMemoryPtr SymbolTable::addSymbolInternal(std::string ident, SyAstType type,
                                              Value* init_mem, int n_elem,
                                              bool is_const, int scope) {
    // no need to delete the ident, just write through it
    IdentMemoryPtr mem = IdentMemory::AllocMemoryForIdent(ident, type, init_mem,
                                                          n_elem, is_const);
    symbol_table_[scope][ident] = mem;
    return mem;
}

IdentMemoryPtr SymbolTable::addSymbol(std::string ident, SyAstType type,
                                      Value* init_mem, int n_elem,
                                      bool is_const) {
    return addSymbolInternal(ident, type, init_mem, n_elem, is_const,
                             current_scope_);
}

IdentMemoryPtr SymbolTable::addGlobalSymbol(std::string ident, SyAstType type,
                                            Value* init_mem, int n_elem,
                                            bool is_const) {
    return addSymbolInternal(ident, type, init_mem, n_elem, is_const, 0);
}

// note that this function can still work when ident isn't exist in the current
// scope
inline void SymbolTable::deleteSymbol(std::string ident) {
    symbol_table_[current_scope_].erase(ident);
}

inline void SymbolTable::enterScope() {
    symbol_table_.push_back(std::unordered_map<std::string, IdentMemoryPtr>());
    current_scope_++;
}

inline void SymbolTable::exitScope() {
    symbol_table_.pop_back();
    current_scope_--;
}