#ifndef _SYTYPE_H_
#define _SYTYPE_H_
#include <cstdint>

union Value {
    int32_t i32;
//    uint32_t u32;
} ;

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

#endif