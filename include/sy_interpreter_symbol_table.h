#ifndef _SY_INTERPRETER_SYMBOL_TABLE_H_
#define _SY_INTERPRETER_SYMBOL_TABLE_H_
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

#include "syparse.h"

class AstNode;
using AstNodePtr = std::shared_ptr<AstNodeBase>;

namespace interpreter {
class IdentMemoryAPI {
   public:
    virtual char* getMem()   = 0;
    virtual size_t getSize() = 0;
    virtual bool isConst()   = 0;
    virtual ~IdentMemoryAPI() {}
};

class ArrayMemoryAPI : public IdentMemoryAPI {
   public:
    virtual unsigned int getDimension()                              = 0;
    virtual unsigned int getSizeForDimension(unsigned int dimension) = 0;
    virtual unsigned int getArraySize()                              = 0;
    virtual void setDimension(unsigned int dimension)                = 0;
    virtual void setSizeForDimension(unsigned int dimension,
                                     unsigned int size)              = 0;
    virtual ~ArrayMemoryAPI() {}
};

using IdentMemoryPtr = std::shared_ptr<IdentMemoryAPI>;
using ArrayMemoryPtr = std::shared_ptr<ArrayMemoryAPI>;
class IdentMemory : IdentMemoryAPI {
   private:
    char* mem_;
    size_t size_;
    bool is_const_;

   public:
    // this is a factory method
    static IdentMemoryPtr AllocMemoryForIdent(AstNodePtr ident, bool is_const);
    IdentMemory(size_t size, bool is_const) {
        this->size_ = size;
        // this can cause a serious memory waste when too much
        // small memory (< 0x18 with glibc) is allocated
        // consider to make a pool
        mem_      = new char[size];
        is_const_ = is_const;
    }
    ~IdentMemory() { delete[] mem_; }
    char* getMem() { return mem_; }
    size_t getSize() { return size_; }
    bool isConst() { return is_const_; }
};

class ArrayMemory : public IdentMemory, public ArrayMemoryAPI {
   private:
    unsigned int dimension_;
    std::vector<unsigned int> size_for_dimension_;
    unsigned int array_size_;

   public:
    void setDimension(unsigned int dimension) {
        size_for_dimension_.resize(dimension);
        dimension_ = dimension;
    }
    void setSizeForDimension(unsigned int dimension, unsigned int size) {
        size_for_dimension_[dimension] = size;
        array_size_ *= size;
    }
    unsigned int getDimension() { return dimension_; }
    unsigned int getSizeForDimension(unsigned int dimension) {
        return size_for_dimension_[dimension];
    }
    unsigned int getArraySize() { return array_size_; }
    char* getMem() { return IdentMemory::getMem(); }
    size_t getSize() { return IdentMemory::getSize(); }
    ArrayMemory(size_t size, bool is_const) : IdentMemory(size, is_const) {
        dimension_  = 0;
        array_size_ = 1;
    }
    bool isConst() { return IdentMemory::isConst(); }
};

class SymbolTableAPI {
   public:
    virtual IdentMemoryPtr searchTable(AstNodePtr ident)                    = 0;
    virtual IdentMemoryPtr searchCurrentScope(AstNodePtr ident)             = 0;
    virtual IdentMemoryPtr addGlobalSymbol(AstNodePtr ident, bool is_const) = 0;
    virtual IdentMemoryPtr addSymbol(AstNodePtr ident, bool is_const)       = 0;
    virtual void deleteSymbol(AstNodePtr ident)                             = 0;
    virtual void enterScope()                                               = 0;
    virtual void exitScope()                                                = 0;
    virtual ~SymbolTableAPI() {}
};
using SymbolTablePtr = std::shared_ptr<SymbolTableAPI>;

class SymbolTable : SymbolTableAPI {
   private:
    using Scope = std::unordered_map<std::string, IdentMemoryPtr>;

    std::vector<Scope> symbol_table_;
    // symbol_table_[0] is the global table
    int current_scope_;

   public:
    IdentMemoryPtr searchTable(AstNodePtr ident);
    IdentMemoryPtr searchCurrentScope(AstNodePtr ident);
    inline IdentMemoryPtr addSymbol(AstNodePtr ident, bool is_const);
    inline IdentMemoryPtr addGlobalSymbol(AstNodePtr ident, bool is_const);
    inline void deleteSymbol(AstNodePtr ident);
    inline void enterScope();
    inline void exitScope();
    SymbolTable() {
        current_scope_ = 0;
        symbol_table_.push_back(Scope());
    }
};
}  // namespace interpreter

#endif