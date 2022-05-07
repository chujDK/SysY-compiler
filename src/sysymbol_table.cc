#include "sysymbol_table.h"

#include <cassert>
#include <string>
#include <tuple>
#include <unordered_map>

#include "syparse.h"
#include "utils.h"

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
                                                SyAstType type, int n_elem,
                                                bool is_const) {
    std::shared_ptr<IdentMemoryAPI> mem(nullptr);
    switch (type) {
        case SyAstType::VAL_TYPE_INT:
            mem.reset((IdentMemoryAPI *)new IdentMemory(type, is_const));
            return mem;
        case SyAstType::VAL_TYPE_INT_ARRAY:
            mem.reset(
                (ArrayMemoryAPI *)new ArrayMemory(n_elem, type, is_const));
            return mem;
        default:
            // shouldn't reach here
            // cause a bug
            DEBUG_ASSERT_NOT_REACH
            return nullptr;
    }
}

IdentMemoryPtr SymbolTable::addSymbolInternal(std::string ident, SyAstType type,
                                              int n_elem, bool is_const,
                                              int scope) {
    // no need to delete the ident, just write through it
    IdentMemoryPtr mem =
        IdentMemory::AllocMemoryForIdent(ident, type, n_elem, is_const);
    symbol_table_[scope][ident] = mem;
    return mem;
}

IdentMemoryPtr SymbolTable::addSymbol(std::string ident, SyAstType type,
                                      int n_elem, bool is_const) {
    return addSymbolInternal(ident, type, n_elem, is_const, current_scope_);
}

IdentMemoryPtr SymbolTable::addGlobalSymbol(std::string ident, SyAstType type,
                                            int n_elem, bool is_const) {
    return addSymbolInternal(ident, type, n_elem, is_const, 0);
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

std::tuple<bool, FunctionTableAPI::Function> FunctionTable::searchFunction(
    std::string ident) {
    if (function_table_.find(ident) != function_table_.end()) {
        return std::make_tuple(true, function_table_[ident]);
    }
    return std::make_tuple(false, std::make_tuple(SyAstType::VAL_TYPE_VOID, "",
                                                  FunctionArgs(), nullptr));
}

FunctionTable::Function FunctionTable::addFunction(std::string ident,
                                                   SyAstType type) {
    auto funciton = std::make_tuple(type, ident, FunctionArgs(), nullptr);
    function_table_[ident] = funciton;
    return funciton;
}

int FunctionTable::addFunctionArg(std::string ident, SyAstType type,
                                  std::string name) {
    // A. search the function
    auto func_iter = function_table_.find(ident);
    if (func_iter == function_table_.end()) {
        return -1;
    }
    auto &[func_type, func_name, func_args, body] = func_iter->second;
    func_args.push_back(std::make_tuple(type, name));
    return 0;
}

AstNodePtr FunctionTable::function_body(std::string ident) {
    auto func_iter = function_table_.find(ident);
    if (func_iter == function_table_.end()) {
        return nullptr;
    }
    auto &[func_type, func_name, func_args, body] = func_iter->second;
    return body;
}

void FunctionTable::set_function_body(std::string name,
                                      AstNodePtr function_body) {
    auto func_iter = function_table_.find(name);
    if (func_iter == function_table_.end()) {
        DEBUG_ASSERT_NOT_REACH
        return;
    }
    auto &[func_type, func_name, func_args, body] = func_iter->second;
    body                                          = function_body;
}