#ifndef _SYINTERPRET_H_
#define _SYINTERPRET_H_
#include "sy.h"

union Value {
    int32_t i32;
} ;

class SYFunction {
private:
    AstNodePtr func_;
    char* func_exec_mem_;
    bool jited_;
    unsigned int called_times_;

public:
    Value exec();
    AstNodePtr getFuncAst();
};

class FunctionTalbe {
private:
    std::map<std::string, SYFunction> func_table_;
public:
    SYFunction* getFunc(std::string func_name);
    SYFunction* addFunc(AstNodePtr func_ast);
};
using FunctionTalbePtr = std::shared_ptr<FunctionTalbe>;

enum class StmtState {
    BREAK,
    CONTINUE,
    RETURN,
    END_OF_ENUM
};

// this interpreter is a simple tree walking and executing interpreter
class Interpreter {
private:
    ParserAPI* parser_;
    SymbolTablePtr symbol_table_;
    FunctionTalbePtr func_table_;
    bool error_occured_;

    void interpretError(std::string msg, int line);
    void interpretWarning(std::string msg, int line);

    int execOneCompUnit(AstNodePtr comp_unit);
    void declHandler(AstNodePtr decl, bool is_global); // need test
    void initValHandler(AstNodePtr init_val, AstNodePtr const_exp, \
     char* mem_raw, int demention, int size_delta); // need test
    AstNodePtr initValValidater(AstNodePtr init_val, AstNodePtr const_exp, \
     int demention); // need test

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
    Value expDispatcher(AstNodePtr exp);

    std::pair<StmtState, Value> blockHandler(AstNodePtr block);
    std::pair<StmtState, Value> stmtHandler(AstNodePtr stmt);
    std::pair<char*, SyEbnfType> lValLeftHandler(AstNodePtr l_val);
public:
    int exec();
    Value exec(AstNodePtr func_ast, AstNodePtr args);
    Interpreter(ParserAPI* parser): parser_(parser), symbol_table_((SymbolTableAPI*)new SymbolTable()) {}
    ~Interpreter() {
        delete parser_;
    }
};

#endif