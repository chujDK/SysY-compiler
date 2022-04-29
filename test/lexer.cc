#include <cstdlib>
#include <cstring>
#include <iostream>

#include "sy.h"
#include "sydebug.h"

int main(int argc, char* argv[]) {
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
    // this is the code for lexer
    Lexer* lexer       = new Lexer(fileStream);
    LexerIterator iter = lexer->begin();
    while (iter->getAstType() != SyAstType::EOF_TYPE) {
        printSyToken(*iter);
        ++iter;
    }
    return 0;
}