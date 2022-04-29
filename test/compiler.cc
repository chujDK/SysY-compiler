#include <cstdlib>
#include <cstring>
#include <iostream>

#include "sy.h"
#include "sydebug.h"

int main(int argc, char* argv[]) {
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
    // let's do a sample test for ir gen
    ParserAPI* parser           = (ParserAPI*)new Parser(fileStream);
    InterpreterAPI* interpreter = new Interpreter(parser);
    // no runtime init here

    codeGenInit();
    interpreter->exec();
    return 0;
}