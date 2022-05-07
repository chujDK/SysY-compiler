#ifndef _SYSEMANTIC_H_
#define _SYSEMANTIC_H_
#include <string>
#include <tuple>

#include "syparse.h"
#include "sysymbol_table.h"
#include "sytype.h"

// note: to this analysis, a const exp can only constains const lval with init
// val, like "const int a = 1;" is valid, but "const int a;" or
// "int a; const int b = a;" is not valid. containing such a invaild lval can
// cause a undefined behavior. (in Debug build, symbol_table will assert to
// abort)

class SemanticAnalysisVisitor : public AstNodeBase::AstNodeVisitor {
   private:
    bool error_;
    Value const_exp_val_;
    SymbolTableAPI* symbol_table_;
    ArrayMemoryPtr array_mem_;
    IdentMemoryPtr ident_mem_;
    // context for the tour of b_type
    SyAstType type_;

    // context for the exp computing, when const_flag_ = 1, it means we need to
    // compute the result eagerly. also, if in the exp, some ident isn't const,
    // we need to report an error.
    bool const_exp_flag_;

    // context for the typing pass
    SyAstType val_type_;

    //    SymbolTablePtr symbol_table_;
    void defHelper(bool is_const, SyAstType type, AstNodeBase* def_iter);
    void defListHelper(bool is_const, SyAstType type, AstNodePtr def_iter);

    void semanticError(std::string msg, int line);
    std::tuple<Value*, uint32_t> arrayIndexToValueMemoryChecker(
        IdentMemoryPtr array_mem, AstNodePtr const_exp_chain);

   public:
    // nessary getter
    SymbolTableAPI* symbol_table() { return symbol_table_; }

    SemanticAnalysisVisitor()
        : symbol_table_((SymbolTableAPI*)new SymbolTable()) {
        error_          = false;
        const_exp_flag_ = false;
        const_exp_val_  = Value(0);
    }

#define DEF_VISIT_FUNC(type) void visit##type(type##AstNode& node) override;
    SY_EBNF_TYPE_LIST(DEF_VISIT_FUNC)
#undef DEF_VISIT_FUNC
};

#endif