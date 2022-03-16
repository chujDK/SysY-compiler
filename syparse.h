#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <memory>
#include "utils.h"
#include "sy.h"

class InputStream {
public:
    // this class is made to read the input "file"
    virtual char getChar() = 0; // get the current char
    virtual char peakChar() = 0; // get the current char
    virtual char peakNextChar() = 0; // get the next char
    virtual void ungetChar() = 0; // unget the current char
    virtual std::string getLine() = 0; // get the current line
    virtual ~InputStream() {}
};

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

enum class SyEbnfType {
    CompUnit, // CompUnit -> [ CompUnit ] ( Decl | FuncDef ) 
    Decl, // Decl -> ConstDecl | VarDecl
    ConstDecl, // ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    BType, // BType -> 'int'
    ConstDef, // ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal 
    ConstInitVal, // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    VarDecl, // VarDecl -> BType VarDef { ',' VarDef } ';'
    VarDef, // VarDef -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal 
    InitVal, // InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    FuncDef, // FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block 
    FuncType, // FuncType -> 'void' | 'int'
    FuncFParams, // FuncFParams -> FuncFParam { ',' FuncFParam } 
    FuncFParam, // FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }] 
    Block, // Block -> '{' { BlockItem } '}' 
    BlockItem, // BlockItem -> Decl | Stmt
    Stmt, // Stmt -> LVal '=' Exp ';' | [Exp] ';' | Block
                                   //| 'if' '( Cond ')' Stmt [ 'else' Stmt ]
                                   //| 'while' '(' Cond ')' Stmt
                                   //| 'break' ';' | 'continue' ';'
                                   //| 'return' [Exp] ';'
    Exp, // Exp -> AddExp
    Cond, // Cond -> LOrExp
    LVal, // LVal -> Ident {'[' Exp ']'} 
    PrimaryExp, // PrimaryExp -> '(' Exp ')' | LVal | Number
    Number, // Number -> IntConst 
    UnaryExp, // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    UnaryOp, // UnaryOp -> '+' | '-' | '!' 
    FuncRParams, // FuncRParams -> Exp { ',' Exp } 
    MulExp, // MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    AddExp, // AddExp -> MulExp | AddExp ('+' | '−') MulExp 
    RelExp, // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    EqExp, // EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    LAndExp, // LAndExp -> EqExp | LAndExp '&&' EqExp
    LOrExp, // LOrExp -> LAndExp | LOrExp '||' LAndExp
    ConstExp, // ConstExp -> AddExp
    E, // epsilon
    END_OF_ENUM
};

struct AstNode;
using TokenPtr = std::shared_ptr<AstNode>; 
using AstNodePtr = std::shared_ptr<AstNode>;
struct AstNode {
    enum SyAstType ast_type_;
    enum SyEbnfType ebnf_type_;
    int line_;
    std::string literal_;
    TokenPtr next_token_;
    TokenPtr prev_token_;
    // in the ast, the meaning is self-explained;
    // if the nodes are in a list, like FuncFParam to FuncFParams,
    // parent_ and a_ link up a double linked list
    AstNodePtr parent_, a_, b_, c_, d_;
    AstNode(enum SyAstType ast_type, int line, std::string&& literal):
        ast_type_(ast_type), ebnf_type_(SyEbnfType::END_OF_ENUM), line_(line), literal_(literal), 
        next_token_(nullptr), parent_(nullptr), a_(nullptr), b_(nullptr), 
        c_(nullptr), d_(nullptr) {}

    AstNode(enum SyEbnfType ebnf_type, int line):
        ast_type_(SyAstType::END_OF_ENUM), ebnf_type_(ebnf_type), line_(line), literal_(nullptr), 
        next_token_(nullptr), parent_(nullptr), a_(nullptr), b_(nullptr), 
        c_(nullptr), d_(nullptr) {}
};

class Lexer {
private:
    InputStream* input_stream_;
    int line_;
    bool error_occured_;
    TokenPtr current_token_;

    void lexError(std::string msg);
    std::string getString();
    std::string getNumber();
    TokenPtr getIdent();
    TokenPtr getNextTokenInternal();


public:

    TokenPtr getNextToken();
    TokenPtr getNextToken(TokenPtr token);
    TokenPtr getPrevToken(TokenPtr token);
    Lexer(InputStream* input_stream): line_(1), input_stream_(input_stream) {}
    ~Lexer() {delete input_stream_;}
};

class LexerIterator {
private:
    TokenPtr current_token_;
    Lexer* lexer_;
public:
    LexerIterator(TokenPtr token, Lexer* lexer): lexer_(lexer) {
        if (token == nullptr) {
            throw "token is nullptr";
        } else {
            current_token_ = token;
        }
    }
    TokenPtr operator*() { return current_token_; }
    TokenPtr operator->() { return current_token_; }
    LexerIterator& operator++() { current_token_ = lexer_->getNextToken(current_token_); return *this; }
    LexerIterator& operator--() { current_token_ = lexer_->getPrevToken(current_token_); return *this; }
    bool operator!=(const LexerIterator& other) { return current_token_ != other.current_token_; }
};

class Parser {
private:
    Lexer* lexer_;
    LexerIterator* token_iter_;
    bool error_occured_;

    void parseError(std::string msg, int line);
    AstNodePtr BType();
    AstNodePtr CompUnit();
    AstNodePtr Decl();
    AstNodePtr FuncDef();
    AstNodePtr FuncType();
    AstNodePtr ConstDecl();
    AstNodePtr ConstDef();
    AstNodePtr ConstInitVal();
    AstNodePtr VarDecl();
    AstNodePtr VarDef();
    AstNodePtr InitVal();
    AstNodePtr FuncFParams();
    AstNodePtr FuncFParam();
    AstNodePtr Block();
    AstNodePtr BlockItem();
    AstNodePtr Stmt();
    AstNodePtr Exp();
    AstNodePtr Cond();
    AstNodePtr LVal();
    AstNodePtr PrimaryExp();
    AstNodePtr Number();
    AstNodePtr UnaryExp();
    AstNodePtr UnaryOp();
    AstNodePtr FuncRParams();
    AstNodePtr MulExp();
    AstNodePtr MulExpL();
    AstNodePtr AddExp();
    AstNodePtr AddExpL(); // to eliminate the left recursion
    AstNodePtr RelExp();
    AstNodePtr EqExp();
    AstNodePtr LAndExp();
    AstNodePtr LOrExp();
    AstNodePtr ConstExp();
    AstNodePtr Ident();

public:
    AstNodePtr parse();
    Parser(InputStream* InputStream): error_occured_(false) {
        lexer_ = new Lexer(InputStream);
        token_iter_ = new LexerIterator(lexer_->getNextToken(), lexer_);
    }
    ~Parser() {delete lexer_; delete token_iter_;}
};

#endif
