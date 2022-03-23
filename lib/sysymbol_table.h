#ifndef _SYSYMBOL_TABLE_H_
#define _SYSYMBOL_TABLE_H_
#include <vector>
#include <map>
#include <memory>
#include "syparse.h"

struct AstNode;
using TokenPtr = std::shared_ptr<AstNode>; 
using AstNodePtr = std::shared_ptr<AstNode>;

class IdentMemoryAPI {
public:
    virtual char* getMem() = 0;
    virtual size_t getSize() = 0;
};

using IdentMemoryPtr = std::shared_ptr<IdentMemoryAPI>;
class IdentMemory: IdentMemoryAPI {
private:
    char* mem;
    size_t size;
public:
    // this is a factory method
    static IdentMemoryPtr AllocMemoryForIdent(TokenPtr ident);
    IdentMemory(size_t size) {
        this->size = size;
        // TODO: this can cause a serious memory waste
        // when too much small memory (< 0x18 with glibc) is allocated
        // consider to make a pool
        mem = new char[size];
    }
    ~IdentMemory() {
        delete[] mem;
    }
    char* getMem() {
        return mem;
    }
    size_t getSize() {
        return size;
    }
};

class SymbolTableAPI {
public:
    virtual IdentMemoryPtr searchTable(TokenPtr ident) = 0;
    virtual IdentMemoryPtr searchCurrentScope(TokenPtr ident) = 0;
    virtual IdentMemoryPtr addGlobalSymbol(TokenPtr ident) = 0;
    virtual IdentMemoryPtr addSymbol(TokenPtr ident) = 0;
    virtual void deleteSymbol(TokenPtr ident) = 0;
    virtual void enterScope() = 0;
    virtual void exitScope() = 0;
};
using SymbolTablePtr = std::shared_ptr<SymbolTableAPI>;

class SymbolTable: SymbolTableAPI {
private:
    using Scope = std::map<std::string, IdentMemoryPtr>;
    
    std::vector<Scope> symbol_table_;
    // symbol_table_[0] is the global table
    int current_scope_;
public:
    IdentMemoryPtr searchTable(TokenPtr ident);
    IdentMemoryPtr searchCurrentScope(TokenPtr ident);
    inline IdentMemoryPtr addSymbol(TokenPtr ident);
    inline IdentMemoryPtr addGlobalSymbol(TokenPtr ident);
    inline void deleteSymbol(TokenPtr ident);
    inline void enterScope();
    inline void exitScope();
    SymbolTable() {
        current_scope_ = 0;
        symbol_table_.push_back(Scope());
    }
};

#endif