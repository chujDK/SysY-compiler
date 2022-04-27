#include "syruntime.h"

#include <cassert>
#include <cstdio>

int RTprintInt(int a) { return printf("%d", a); }

Value printIntExecCallBack(char* function, AstNodePtr args,
                           InterpreterAPI* interpreter) {
    assert(args->getEbnfType() == SyEbnfType::FuncRParams);
    int a              = interpreter->expDispatcher(args->a_).i32;
    using printIntType = int (*)(int);
    Value ret;
    ret.i32 = ((printIntType)function)(a);
    return ret;
}

int RTprintHex(int a) { return printf("0x%x", a); }

Value printHexExecCallBack(char* function, AstNodePtr args,
                           InterpreterAPI* interpreter) {
    assert(args->getEbnfType() == SyEbnfType::FuncRParams);
    int a              = interpreter->expDispatcher(args->a_).i32;
    using printIntType = int (*)(int);
    Value ret;
    ret.i32 = ((printIntType)function)(a);
    return ret;
}

int printLn() { return printf("\n"); }

Value printLnExecCallBack(char* function, AstNodePtr args,
                          InterpreterAPI* interpreter) {
    using printLnType = int (*)();
    Value ret;
    ret.i32 = ((printLnType)function)();
    return ret;
}

int RTputchar(int a) { return std::putchar(a); }

Value putcharExecCallBack(char* function, AstNodePtr args,
                          InterpreterAPI* interpreter) {
    using putcharType = int (*)(int);
    Value ret;
    int a   = interpreter->expDispatcher(args->a_).i32;
    ret.i32 = ((putcharType)function)(a);
    return ret;
}

void syRuntimeInitForAnInterpreter(InterpreterAPI* interpreter) {
    SYFunctionPtr sy_printInt = std::make_shared<SYFunction>(
        (char*)RTprintInt, false, printIntExecCallBack);
    interpreter->addFunction(sy_printInt, "printInt");
    SYFunctionPtr sy_printHex = std::make_shared<SYFunction>(
        (char*)RTprintHex, false, printHexExecCallBack);
    interpreter->addFunction(sy_printHex, "printHex");
    SYFunctionPtr sy_printLn = std::make_shared<SYFunction>(
        (char*)printLn, false, printLnExecCallBack);
    interpreter->addFunction(sy_printLn, "printLn");
    SYFunctionPtr sy_putchar = std::make_shared<SYFunction>(
        (char*)putchar, false, putcharExecCallBack);
    interpreter->addFunction(sy_putchar, "putchar");
}