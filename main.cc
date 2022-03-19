#include <iostream>
#include <cstring>
#include <cstdlib>
#include "sy.h"
#include "sydebug.h"

void astWalkThrough(AstNodePtr node, int level) {
    if (node == nullptr) {
        return;
    }
    for (int i = 0; i < level; i++) {
        std::cout << "    ";
    }
    if (node->ebnf_type_ != SyEbnfType::END_OF_ENUM) {
        std::cout << SyEbnfTypeDebugInfo[(int) node->ebnf_type_] << " \"" << node->literal_ << "\"" << std::endl;
    } else {
        std::cout << SyAstTypeDebugInfo[(int) node->ast_type_] << " \"" << node->literal_ << "\"" << std::endl;
    }
    astWalkThrough(node->a_, level + 1);
    astWalkThrough(node->b_, level + 1);
    astWalkThrough(node->c_, level + 1);
    astWalkThrough(node->d_, level + 1);
    return;
}

int main(int argc, char* argv[])
{
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
    #ifdef LEXER
    // this is the code for lexer
    Lexer lexer(fileStream);
    LexerIterator iter(lexer.getNextToken(), &lexer);
    while (iter->ast_type_ != SyAstType::EOF_TYPE) {
        printSyToken(*iter);
        ++iter;
    } 
    #endif
    #ifdef PARSER
    // this is the code for parser
    Parser parser(fileStream);
    while (1)
    {
        auto root = parser.parse();
        astWalkThrough(root, 0);
        if (root == nullptr) {
            break;
        }
    }
    #endif
    
    return 0;
}
