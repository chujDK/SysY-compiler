// tend to deprecate this
#ifndef _SYINTERPRET_H_
#define _SYINTERPRET_H_
#include <llvm/IR/Function.h>

#include <map>
#include <string>
#include <tuple>

#include "sy_interpreter_symbol_table.h"
#include "syparse.h"
#include "sytype.h"

class InterpreterAPI;
class SYFunction {
   private:
    AstNodePtr func_;  // func_->getAstType() == SyAstType::FuncDef
    char* func_exec_mem_;
    Value (*exec_call_back_)(
        char*, AstNodePtr,
        InterpreterAPI*);  // this function should handle the args
    bool jited_;
    bool no_fail_;
    unsigned int called_times_;
    llvm::Function* functionDefinitionLLVMIrGen();
    void addToLLVMSymbolTable();

   public:
    Value exec(AstNodePtr args, InterpreterAPI* interpreter);
    bool isJited() { return jited_; }
    AstNodePtr getFuncAst();
    SYFunction(AstNodePtr func);
    SYFunction(void* func, bool no_fail,
               Value (*exec_call_back)(char*, AstNodePtr, InterpreterAPI*));
    // currently only support function level jit
    void compile();
};
using SYFunctionPtr = std::shared_ptr<SYFunction>;

class InterpreterAPI {
   public:
    virtual ~InterpreterAPI(){};
    virtual int exec()                                                 = 0;
    virtual Value execFunction(AstNodePtr func_ast, AstNodePtr args)   = 0;
    virtual Value expDispatcher(AstNodePtr exp)                        = 0;
    virtual void addFunction(SYFunctionPtr function, std::string name) = 0;
};

class FunctionTable {
   private:
    std::map<std::string, SYFunctionPtr> func_table_;

   public:
    SYFunctionPtr getFunc(std::string func_name);
    SYFunctionPtr addFunc(AstNodePtr func_ast);
    SYFunctionPtr addFunc(SYFunctionPtr function, std::string name);
};
using FunctionTalbePtr = std::shared_ptr<FunctionTable>;

enum class StmtState { BREAK, CONTINUE, RETURN, END_OF_ENUM };

// this interpreter is a simple tree walking and executing interpreter
class Interpreter : public InterpreterAPI, public AstNodeBase::AstNodeVisitor {
   private:
    ParserAPI* parser_;
    interpreter::SymbolTablePtr symbol_table_;
    FunctionTalbePtr func_table_;
    bool error_occured_;

// define all the visit method
#define DEF_VISIT_FUNC(type) void visit##type(type##AstNode& node) override{};
    SY_EBNF_TYPE_LIST(DEF_VISIT_FUNC)
#undef DEF_VISIT_FUNC

    void interpretError(std::string msg, int line);
    void interpretWarning(std::string msg, int line);

    int execOneCompUnit(AstNodePtr comp_unit);
    void declHandler(AstNodePtr decl, bool is_global);  // need test
    void initValHandler(AstNodePtr init_val, AstNodePtr const_exp,
                        char* mem_raw, int demention,
                        int size_delta);  // need test
    AstNodePtr initValValidater(AstNodePtr init_val, AstNodePtr const_exp,
                                int demention);  // need test

    inline Value constExpHandler(AstNodePtr exp);
    inline Value expHandler(AstNodePtr exp);
    Value addExpHandler(AstNodePtr exp);
    Value mulExpHandler(AstNodePtr exp);
    Value relExpHandler(AstNodePtr exp);
    Value eqExpHandler(AstNodePtr exp);
    Value lAndExpHandler(AstNodePtr exp);
    Value lOrExpHandler(AstNodePtr exp);
    Value unaryExpHandler(AstNodePtr exp);
    Value primaryExpHandler(AstNodePtr exp);
    Value lValRightHandler(AstNodePtr exp);
    Value numberHandler(AstNodePtr exp);
    Value subExpHandler(AstNodePtr exp);
    void variableIdentTyper(AstNodePtr ident, SyEbnfType type_enum);
    void variableIdentTyper(AstNodePtr ident, AstNodePtr type);

    void setInitValMemoryLayout(int dimension,
                                std::vector<unsigned int> size_for_dimension,
                                Value* mem_raw, AstNodePtr init_val);

    std::tuple<StmtState, Value> blockHandler(AstNodePtr block);
    std::tuple<StmtState, Value> stmtHandler(AstNodePtr stmt);
    std::tuple<char*, SyAstType> lValLeftHandler(AstNodePtr l_val);

   public:
    int exec() override;
    Value execFunction(AstNodePtr func_ast, AstNodePtr args) override;
    Value expDispatcher(AstNodePtr exp) override;
    void addFunction(SYFunctionPtr function, std::string name) override;
    Interpreter(ParserAPI* parser)
        : parser_(parser),
          symbol_table_(
              (interpreter::SymbolTableAPI*)new interpreter::SymbolTable()),
          func_table_(new FunctionTable()) {}
    ~Interpreter() { delete parser_; }
};

#endif
