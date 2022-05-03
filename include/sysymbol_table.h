#ifndef _SYSYMBOL_TABLE_H_
#define _SYSYMBOL_TABLE_H_
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "syparse.h"

struct AstNodeBase;
using AstNodePtr = std::shared_ptr<AstNodeBase>;

class IdentMemoryAPI {
   public:
    virtual Value* getMem()     = 0;
    virtual SyAstType getType() = 0;
    virtual size_t getSize()    = 0;
    virtual bool isConst()      = 0;
    virtual ~IdentMemoryAPI() {}
};

class ArrayMemoryAPI : public IdentMemoryAPI {
   public:
    virtual unsigned int getDimension()                              = 0;
    virtual unsigned int getSizeForDimension(unsigned int dimension) = 0;
    virtual void setDimension(unsigned int dimension)                = 0;
    virtual void setSizeForDimension(unsigned int dimension,
                                     unsigned int size)              = 0;
    virtual ~ArrayMemoryAPI() {}
};

using IdentMemoryPtr = std::shared_ptr<IdentMemoryAPI>;
using ArrayMemoryPtr = std::shared_ptr<ArrayMemoryAPI>;
class IdentMemory : IdentMemoryAPI {
   private:
    Value* mem_;
    SyAstType type_;
    size_t size_;
    bool is_const_;

   public:
    // this is a factory method
    static IdentMemoryPtr AllocMemoryForIdent(std::string ident, SyAstType type,
                                              Value* init_mem, int n_elem,
                                              bool is_const);
    IdentMemory(SyAstType type, bool is_const, Value* init_mem) {
        // this can cause a serious memory waste when too much
        // small memory (< 0x18 with glibc) is allocated
        // consider to make a pool
        type_     = type;
        mem_      = init_mem;
        is_const_ = is_const;
        size_     = 1;
    }
    IdentMemory(size_t size, SyAstType type, bool is_const, Value* init_mem) {
        type_     = type;
        mem_      = init_mem;
        is_const_ = is_const;
        size_     = size;
    }
    ~IdentMemory() { delete[] mem_; }
    Value* getMem() override { return mem_; }
    SyAstType getType() override { return type_; }
    size_t getSize() override { return size_; }
    bool isConst() override { return is_const_; }
};

class ArrayMemory : public IdentMemory, public ArrayMemoryAPI {
   private:
    unsigned int dimension_;
    std::vector<unsigned int> size_for_dimension_;

   public:
    void setDimension(unsigned int dimension) override {
        size_for_dimension_.resize(dimension);
        dimension_ = dimension;
    }
    void setSizeForDimension(unsigned int dimension,
                             unsigned int size) override {
        size_for_dimension_[dimension] = size;
    }
    unsigned int getDimension() override { return dimension_; }
    unsigned int getSizeForDimension(unsigned int dimension) override {
        return size_for_dimension_[dimension];
    }
    ArrayMemory(size_t size, SyAstType type, bool is_const, Value* init_mem)
        : IdentMemory(size, type, is_const, init_mem) {
        dimension_ = 0;
    }

    Value* getMem() override { return IdentMemory::getMem(); }
    SyAstType getType() override { return IdentMemory::getType(); }
    size_t getSize() override { return IdentMemory::getSize(); }
    bool isConst() override { return IdentMemory::isConst(); }
};

class SymbolTableAPI {
   public:
    virtual IdentMemoryPtr searchTable(std::string ident)        = 0;
    virtual IdentMemoryPtr searchCurrentScope(std::string ident) = 0;
    virtual IdentMemoryPtr addGlobalSymbol(std::string ident, SyAstType type,
                                           Value* init_mem, int n_elem,
                                           bool is_const)        = 0;
    virtual IdentMemoryPtr addSymbol(std::string ident, SyAstType type,
                                     Value* init_mem, int n_elem,
                                     bool is_const)              = 0;
    virtual void deleteSymbol(std::string ident)                 = 0;
    virtual void enterScope()                                    = 0;
    virtual void exitScope()                                     = 0;
    virtual ~SymbolTableAPI() {}
};
using SymbolTablePtr = std::shared_ptr<SymbolTableAPI>;

class SymbolTable : SymbolTableAPI {
   private:
    using Scope = std::unordered_map<std::string, IdentMemoryPtr>;

    std::vector<Scope> symbol_table_;
    // symbol_table_[0] is the global table
    int current_scope_;

    IdentMemoryPtr addSymbolInternal(std::string ident, SyAstType type,
                                     Value* init_mem, int n_elem, bool is_const,
                                     int scope);

   public:
    IdentMemoryPtr searchTable(std::string ident);
    IdentMemoryPtr searchCurrentScope(std::string ident);

    // @n_elem is the number of elements in the *array*, only used when type
    // is array
    IdentMemoryPtr addSymbol(std::string ident, SyAstType type, Value* init_mem,
                             int n_elem, bool is_const);
    // @n_elem is the number of elements in the *array*, only used when type is
    // array
    IdentMemoryPtr addGlobalSymbol(std::string ident, SyAstType type,
                                   Value* init_mem, int n_elem, bool is_const);
    void deleteSymbol(std::string ident);
    void enterScope();
    void exitScope();
    SymbolTable() {
        current_scope_ = 0;
        symbol_table_.push_back(Scope());
    }
};

#endif