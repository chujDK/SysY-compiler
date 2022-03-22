#ifndef _SYINTERPRET_H_
#define _SYINTERPRET_H_
#include "sy.h"

// this interpreter is a simple tree walking and executing interpreter



class Interpreter {
private:
    ParserAPI* parser_;
    SymbolTablePtr symbol_table_;
public:
    Interpreter(ParserAPI* parser): parser_(parser) {}
    ~Interpreter() {
        delete parser_;
    }
};

#endif