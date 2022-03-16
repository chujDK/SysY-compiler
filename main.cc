#include <iostream>
#include <cstring>
#include <cstdlib>
#include "sy.h"
#include "sydebug.h"

int main(int argc, char* argv[])
{
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
    // this is the code for lexer
    Lexer lexer(fileStream);
    LexerIterator iter(lexer.getNextToken(), &lexer);
    while (iter->ast_type_ != SyAstType::EOF_TYPE) {
        std::cout << "\033[1m\033[34m" << SyAstTypeDebugInfo[(int)iter->ast_type_] << "\033[0m" << " \""
        "\033[32m" << iter->literal_ << "\033[0m" << "\"" << std::endl;
        ++iter;
    } 
    return 0;
}
