#ifndef _SYPARSE_H_
#define _SYPARSE_H_
#include <memory>
#include <cassert>
#include <string>
#include "utils.h"


struct AstNode;
class TokenAstNode;
using TokenPtr = std::shared_ptr<TokenAstNode>; 
using AstNodePtr = std::shared_ptr<AstNode>;
enum class SyAstType;
enum class SyEbnfType;

class AstNodePool {
public:
    static TokenPtr get(SyAstType type, int line, std::string && literal);
    static AstNodePtr get(SyEbnfType type, int line);
};

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

// SyEbnfType 同样存储类型信息 ( 虽然只有 int, int[] 和 void :( )
// 如果需要修改语法，需要注意：
// 1. 如果修改了语法，需要修改SyEbnfTypeDebugInfo
// 2. 需要注意修改语法后，对链表类型的影响，链表类型假设 d_ 不会被使用
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
                                   //| 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
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

    // typing
    TYPE_INT, // 'int'
    TYPE_CONST_INT, // 'const int'
    TYPE_VOID, // 'void'
    TYPE_INT_ARRAY, // 'int[]'
    TYPE_CONST_INT_ARRAY, // 'const int[]'
    END_OF_ENUM
};

#ifdef DEBUG
#define DEBUG_ASSERT_NOT_REACH \
    assert(false && "should not reach here");
#else
#define DEBUG_ASSERT_NOT_REACH
#endif


struct AstNode {
    enum SyAstType ast_type_; // delete this field in the future
    enum SyEbnfType ebnf_type_; // delete this field in the future
    unsigned int line_;
    // only to the EBnfType::TYPE_INT_ARRAY, array_size_ can be used
    // this also means that this complier can only support array size up to 2^32-1
    // only to the EBnfType::ConstDef, array_size_ can be used
    // it's tempting to move the line_ into this union, but giveup
    // delete this field in the future
    union {
        unsigned int array_size_;
        unsigned int const_val_;
    } u_;
//    std::string literal_; // delete this field in the future
    // in the ast, the meaning is self-explained;
    // if the nodes are in a list, like FuncFParam to FuncFParams,
    // parent_ and d_ link up a double linked list
    // be careful when using the d_, make sure it's not in a list
    std::weak_ptr<AstNode> parent_; // delete this field in the future
    AstNodePtr a_, b_, c_, d_; // delete this field in the future
    AstNode(enum SyAstType ast_type, int line):
        ast_type_(ast_type), ebnf_type_(SyEbnfType::END_OF_ENUM), line_(line),
        a_(nullptr), b_(nullptr), 
        c_(nullptr), d_(nullptr) { u_.const_val_ = 0xFFFFFFFF; }

    AstNode(enum SyEbnfType ebnf_type, int line):
        ast_type_(SyAstType::END_OF_ENUM), ebnf_type_(ebnf_type), line_(line),
        a_(nullptr), b_(nullptr), 
        c_(nullptr), d_(nullptr) { u_.const_val_ = 0xFFFFFFFF; }

    // following the virtual functions for the children getters
    virtual std::string const& getLiteral() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getDecl() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getConstDecl() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getBType() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getConstDefListFirst() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getConstDef() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getConstInitVal() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getVarDecl() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getVarDef() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getInitVal() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getFuncDef() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getFuncType() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getFuncFParams() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getFuncFParam() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getBlock() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getBlockItem() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getStmt() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getCond() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getLVal() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getPrimaryExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getNumber() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getUnaryExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getUnaryOp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getFuncRParams() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getMulExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getAddExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getRelExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getEqExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getLAndExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getLOrExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getConstExp() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getConstExpList() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getListNext() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getIdent() { DEBUG_ASSERT_NOT_REACH return nullptr; }
    virtual AstNodePtr getType() { DEBUG_ASSERT_NOT_REACH return nullptr; }

    virtual ~AstNode() {}
};

class TokenAstNode : public AstNode {
private:
    std::string literal_;
    // TODO: make this private
public:
    TokenPtr next_token_;
    TokenPtr prev_token_;

public:
    TokenAstNode(enum SyAstType ast_type, int line, std::string&& literal):
        AstNode(ast_type, line), literal_(literal), next_token_(nullptr), prev_token_(nullptr) {}

    std::string const& getLiteral() { return literal_; }
};

class CompUnitAstNode : public AstNode {
    AstNodePtr getDecl() { return a_; }
    AstNodePtr getFuncDef() { return a_; }
};

class DeclAstNode : public AstNode {
    AstNodePtr getConstDecl() { return a_; }
    AstNodePtr getDecl() { return a_; }
};

class ConstDeclAstNode : public AstNode {
    AstNodePtr getBType() { return a_; }
    AstNodePtr getConstDefListFirst() { return b_; }
};

class BTypeAstNode : public AstNode {
    AstNodePtr getType() { return a_; }
};

class ConstDefAstNode : public AstNode {
    AstNodePtr getListNext() { return d_; }
    AstNodePtr getIdent() { return a_; }
    AstNodePtr getConstExpList() { return b_; }
    AstNodePtr getConstInitVal() { return c_; }
    AstNodePtr getInitVal() { return getConstInitVal(); }
};

class ConstInitValAstNode : public AstNode {
};

class VarDeclAstNode : public AstNode {
};

class VarDefAstNode : public AstNode {
};

class InitValAstNode : public AstNode {
};

class FuncDefAstNode : public AstNode {
};

class FuncTypeAstNode : public AstNode {
};

class FuncFParamsAstNode : public AstNode {
};

class FuncFParamAstNode : public AstNode {
};

class BlockAstNode : public AstNode {
};

class BlockItemAstNode : public AstNode {
};

class StmtAstNode : public AstNode {
};

class ExpAstNode : public AstNode {
};

class CondAstNode : public AstNode {
};

class LValAstNode : public AstNode {
};

class PrimaryExpAstNode : public AstNode {
};

class NumberAstNode : public AstNode {
};

class UnaryExpAstNode : public AstNode {
};

class UnaryOpAstNode : public AstNode {
};

class FuncRParamsAstNode : public AstNode {
};

class MulExpAstNode : public AstNode {
};

class AddExpAstNode : public AstNode {
};

class RelExpAstNode : public AstNode {
};

class EqExpAstNode : public AstNode {
};

class LAndExpAstNode : public AstNode {
};

class LOrExpAstNode : public AstNode {
};

class ConstExpAstNode : public AstNode {
};

class ConstExpAstNode : public AstNode {
};

class Lexer {
private:
    InputStream* input_stream_;
    int line_;
    bool error_occured_;
    TokenPtr current_token_;

    void lexError(std::string msg);
    void lexWarning(std::string msg);
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
    std::shared_ptr<Lexer> lexer_;
public:
    LexerIterator(TokenPtr token, Lexer* lexer): lexer_(lexer) {
        if (token == nullptr) {
            fprintf(stderr, "LexerIterator: token is nullptr\n");
            exit(-1);
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

class ParserAPI {
public:
    virtual AstNodePtr parse() = 0;
    virtual bool end() = 0;
    virtual bool error() = 0;
    virtual ~ParserAPI() {};
};

class Parser : ParserAPI {
private:
    LexerIterator* token_iter_;
    bool error_occured_;
    bool end_parse_;

    void parseError(std::string msg, int line);
    AstNodePtr BType(); // no error handling
    AstNodePtr CompUnit(); // no error handling
    AstNodePtr Decl(); // no error handling
    AstNodePtr FuncDef(); // error handling: DONE
    AstNodePtr FuncType(); // no error handling
    AstNodePtr ConstDecl(); // error handling: DONE
    AstNodePtr ConstDef(); // error handling: DONE
    AstNodePtr ConstInitVal(); // error handling: DONE
    AstNodePtr VarDecl(); // error handling: DONE
    AstNodePtr VarDef(); // error handling: DONE 
    AstNodePtr InitVal(); // error handling: DONE
    AstNodePtr FuncFParams(); // error handling: DONE
    AstNodePtr FuncFParam(); // no error handling
    AstNodePtr Block(); // error handling: DONE
    AstNodePtr BlockItem(); // error handling: DONE
    AstNodePtr Stmt(); // error handling: DONE
    AstNodePtr Exp(); // no error handling
    AstNodePtr Cond(); // no errror handling
    AstNodePtr LVal(); // no error handling TODO: check if it really needn't error handling
    AstNodePtr PrimaryExp(); // no error handling
    AstNodePtr Number(); // no error handling
    AstNodePtr UnaryExp(); // no error handling
    AstNodePtr UnaryOp(); // no error handling
    AstNodePtr FuncRParams(); // error handling: DONE
    // following function with the L suffix means left recursion elimination
    AstNodePtr MulExp(); // no error handling
    AstNodePtr MulExpL(); // won't error
    AstNodePtr AddExp(); // no error handling
    AstNodePtr AddExpL(); // won't error
    AstNodePtr RelExp(); // no error handling
    AstNodePtr RelExpL(); // won't error
    AstNodePtr EqExp(); // no error handling
    AstNodePtr EqExpL(); // won't error
    AstNodePtr LAndExp(); // no error handling
    AstNodePtr LAndExpL(); // won't error
    AstNodePtr LOrExp(); // no error handling
    AstNodePtr LOrExpL(); // won't error
    AstNodePtr ConstExp(); // no error handling
    AstNodePtr Ident(); // no error handling

public:
    AstNodePtr parse();
    bool end() { return end_parse_; }
    bool error() { return error_occured_; }
    Parser(InputStream* InputStream): error_occured_(false), end_parse_(false) {
        auto lexer = new Lexer(InputStream);
        token_iter_ = new LexerIterator(lexer->getNextToken(), lexer);
    }
    ~Parser() {delete token_iter_;}
};


#endif
