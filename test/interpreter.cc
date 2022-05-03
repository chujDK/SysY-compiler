#include <cstdlib>
#include <cstring>
#include <iostream>

#include "sy.h"
#include "sy_interpreter_symbol_table.h"
#include "sydebug.h"

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