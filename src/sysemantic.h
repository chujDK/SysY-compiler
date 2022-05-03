#ifndef _SYSEMANTIC_H_
#define _SYSEMANTIC_H_
#include "syparse.h"
#include "sysymbol_table.h"

class FunctionTable {};

class SemanticAnalysisVisitor : public AstNodeBase::AstNodeVisitor {
   private:
    bool error_;
    //    SymbolTablePtr symbol_table_;

   public:
#define DEF_VISIT_FUNC(type) void visit##type(type##AstNode& node) override;
    SY_EBNF_TYPE_LIST(DEF_VISIT_FUNC)
#undef DEF_VISIT_FUNC
};

#endif