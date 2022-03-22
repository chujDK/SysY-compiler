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
        std::cout << "  ";
    }
    if (node->literal_.empty()) {
        if (node->ebnf_type_ != SyEbnfType::END_OF_ENUM) {
            std::cout << "\033[1m\033[33m" << SyEbnfTypeDebugInfo[(int) node->ebnf_type_] << "\033[0m";
        } else {
            std::cout << "\033[1m\033[33m" << SyAstTypeDebugInfo[(int) node->ast_type_] << "\033[0m";
        }
    }
    else {
        if (node->ebnf_type_ != SyEbnfType::END_OF_ENUM) {
            std::cout << "\033[1m\033[34m" << SyEbnfTypeDebugInfo[(int) node->ebnf_type_] << "\033[0m \"\033[32m" << node->literal_ << "\033[0m\"";
        } else {
            std::cout << "\033[1m\033[34m" << SyAstTypeDebugInfo[(int) node->ast_type_] << "\033[0m \"\033[32m" << node->literal_ << "\033[0m\"";
        }
    }
    #ifdef AST_WALK_SHOW_CHILDERN
    if (node->a_ != nullptr) {
        std::cout << "\033[35m\ta\033[0m";
    }
    if (node->b_ != nullptr) {
        std::cout << "\033[35m\tb\033[0m";
    }
    if (node->c_ != nullptr) {
        std::cout << "\033[35m\tc\033[0m";
    }
    if (node->d_ != nullptr) {
        std::cout << "\033[35m\td\033[0m";
    }
    #endif
    std::cout << std::endl;
    astWalkThrough(node->a_, level + 1);
    astWalkThrough(node->b_, level + 1);
    astWalkThrough(node->c_, level + 1);
    astWalkThrough(node->d_, level + 1);
    return;
}

#ifndef UNIT_TEST
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
    #ifdef INTERPRETER
    // this is the code for interpreter
    #endif
    
    return 0;
}
#endif
