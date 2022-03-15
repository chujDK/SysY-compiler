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
        std::cout << "\033[1m\033[34m" << SyAstTypeDebugInfo[(int)token->ast_type_] << "\033[0m" << " \""
        "\033[32m" << token->literal_ << "\033[0m" << "\"" << std::endl;
        if (token->ast_type_ == SyAstType::EOF_TYPE) {
            break;
        }
    }
    return 0;
}
