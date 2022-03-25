#include <cstdio>
#include <cassert>
#include "syruntime.h"

int printInt(int a) {
    return printf("%d", a);
}

Value printIntExecCallBack(char* function, AstNodePtr args, InterpreterAPI* interpreter) {
    assert(args->ebnf_type_ == SyEbnfType::FuncRParams);
    int a = interpreter->expDispatcher(args->a_).i32;
    using printIntType = int (*)(int);
    Value ret;
    ret.i32 = ((printIntType) function)(a);
    return ret;
}

int printHex(int a) {
    return printf("0x%x", a);
}

Value printHexExecCallBack(char* function, AstNodePtr args, InterpreterAPI* interpreter) {
    assert(args->ebnf_type_ == SyEbnfType::FuncRParams);
    int a = interpreter->expDispatcher(args->a_).i32;
    using printIntType = int (*)(int);
    Value ret;
    ret.i32 = ((printIntType) function)(a);
    return ret;
}

int printLn() {
    return printf("\n");
}

Value printLnExecCallBack(char* function, AstNodePtr args, InterpreterAPI* interpreter) {
    using printLnType = int (*)();
    Value ret;
    ret.i32 = ((printLnType) function)();
    return ret;
}

void syRuntimeInitForAnInterpreter(InterpreterAPI* interpreter) {
    SYFunctionPtr sy_printInt = std::make_shared<SYFunction>((char*)printInt, false, printIntExecCallBack);
    interpreter->addFunction(sy_printInt, "printInt");
    SYFunctionPtr sy_printHex = std::make_shared<SYFunction>((char*)printHex, false, printHexExecCallBack);
    interpreter->addFunction(sy_printHex, "printHex");
    SYFunctionPtr sy_printLn = std::make_shared<SYFunction>((char*)printLn, false, printLnExecCallBack);
    interpreter->addFunction(sy_printLn, "printLn");
}