#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <memory>
#include "utils.h"
#include "sy.h"
class InputStream;

enum class SyAstType {
    LEFT_PARENTHESE, // '('
    RIGHT_PARENTHESE, // ')'
    LEFT_BRACKET, // '['
    RIGHT_BRACKET, // ']'
    LEFT_BRACE, // '{'
    RIGHT_BRACE, // '}'

    // ALU op
    ALU_ADD, // '+'
    ALU_SUB, // '-'
    ALU_DIV, // '/'
    ALU_MUL, // '*'
    ALU_MOD, // '%'

    ASSIGN, // '='

    // relation op
    EQ, // '=='
    NEQ, // '!='
    LNE, // '<'
    LE, // '<='
    GNE, // '>'
    GE, // '>='

    // logic op
    LOGIC_NOT, // '!'
    LOGIC_AND, // '&&'
    LOGIC_OR, // '||'

    SEMICOLON, // ';'
    QUOTE, // '"'
    COMMA, // ','
    DOT, // '.'

    // type
    TYPE_INT, // 'int'
    TYPE_VOID, // 'void'

    // statement
    STM_IF, // 'if'
    STM_ELSE, // 'else'
    STM_WHILE, // 'while'
    STM_BREAK, // 'break'
    STM_CONTINUE, // 'continue'
    STM_RETURN, // 'return'
    STM_CONST, // 'const'

    // number
    INT_IMM,

    STRING,
    IDENT, // [_a-zA-Z][_-a-zA-Z0-9]*
    EOF_TYPE,
    END_OF_ENUM
};

struct AstNode;
using TokenPtr = std::shared_ptr<AstNode>; 
using AstNodePtr = std::shared_ptr<AstNode>;
struct AstNode {
    enum SyAstType ast_type_;
    int line_;
    std::string literal_;
    TokenPtr next_token_;
    TokenPtr prev_token_;
    AstNodePtr parent_, a_, b_, c_, d_;
    AstNode(enum SyAstType ast_type, int line, std::string&& literal):
        ast_type_(ast_type), line_(line), literal_(literal), 
        next_token_(nullptr), parent_(nullptr), a_(nullptr), b_(nullptr), 
        c_(nullptr), d_(nullptr) {}
};


class Lexer {
private:
    InputStream* input_stream_;
    int line_;
    bool error_occured_;
    void lexError(std::string msg);
    std::string getString();
    std::string getNumber();
    TokenPtr getIdent();
    TokenPtr current_token_;
    TokenPtr getNextTokenInternal();
public:
    TokenPtr getNextToken();
    TokenPtr getNextToken(TokenPtr token);
    TokenPtr getPrevToken(TokenPtr token);
    Lexer(InputStream* input_stream): line_(1), input_stream_(input_stream) {}
};

class Parser {
private:
    Lexer* lexer_;
public:

};

#endif