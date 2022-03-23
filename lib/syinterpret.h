#ifndef _SYINTERPRET_H_
#define _SYINTERPRET_H_
#include "sy.h"

union Value {
    int32_t i32;
} ;

// this interpreter is a simple tree walking and executing interpreter
class Interpreter {
private:
    ParserAPI* parser_;
    SymbolTablePtr symbol_table_;
    bool error_occured_;

    void interpretError(std::string msg, int line);
    void interpretWarning(std::string msg, int line);

    int execOneCompUnit(AstNodePtr comp_unit);
    void declHandler(AstNodePtr decl, bool is_global); // need test
    void initValHandler(AstNodePtr init_val, AstNodePtr const_exp, \
     char* mem_raw, int demention, int size_delta); // need test

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
    Value lValHandler(AstNodePtr exp);
    Value numberHandler(AstNodePtr exp);
    Value expDispatcher(AstNodePtr exp);

public:
    int exec();
    Interpreter(ParserAPI* parser): parser_(parser), symbol_table_((SymbolTableAPI*)new SymbolTable()) {}
    ~Interpreter() {
        delete parser_;
    }
};

#endif