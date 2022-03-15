#include <iostream>
#include <cstring>
#include <cstdlib>
#include "sy.h"
#include "sydebug.h"

int main(int argc, char* argv[])
{
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
    Lexer lexer(fileStream);
    TokenPtr token;
    while (token = lexer.getNextToken()) {
        std::cout << SyAstTypeDebugInfo[(int)token->ast_type_] << " \"" << token->literal_ << "\"" << std::endl;
        if (token->ast_type_ == SyAstType::EOF_TYPE) {
            break;
        }
    }
    return 0;
}
