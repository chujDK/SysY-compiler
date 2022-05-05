#include <iostream>

#include "sy_interpreter_symbol_table.h"
#include "sydebug.h"
#include "syinterpret.h"
#include "syruntime.h"

int main(int argc, char* argv[]) {
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
    // this is the code for interpreter
    ParserAPI* parser           = (ParserAPI*)new Parser(fileStream);
    InterpreterAPI* interpreter = new Interpreter(parser);
    syRuntimeInitForAnInterpreter(interpreter);
    interpreter->exec();
    return 0;
}