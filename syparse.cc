#include <cstdio>
#include <cstring>
#include "syparse.h"

static inline bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool isIdentStart(char c) {
    return isAlpha(c) || c == '_';
}

static inline bool isIdentPart(char c) {
    return isAlpha(c) || isDigit(c) || c == '_';
}

static inline bool isEndForIdentAndNumber(char c) {
    return c == ' ' || c == '.' || c == ';' || c == ',' 
        || c == '|' || c == '&' || c == '<' || c == '>'
        || c == '=' || c == '!' || c == '*' || c == '+'
        || c == '-' || c == '/' || c == '%' || c == '('
        || c == ')' || c == '[' || c == ']' || c == '{'
        || c == '}' || c == '\t';
}

void Lexer::lexError(std::string msg) {
    fprintf(stderr, "\033[31mError in lexing\033[0m: line %d: %s\n", line_, msg.c_str());
    error_occured_ = 1;
}

void Parser::parseError(std::string msg, int line) {
    fprintf(stderr, "\033[31mError in parser\033[0m: line %d: %s\n", line, msg.c_str());
    error_occured_ = 1;
}

std::string Lexer::getString() {
    std::string str;
    while (true) {
        char c = input_stream_->getChar();
        if (c == '"') {
            break;
        }
        if (c == '%') {
            if (input_stream_->peakNextChar() == '%') {
                str += c;
                input_stream_->getChar(); // "%%" -> "%"
                continue;
            }
            if (input_stream_->peakNextChar() != 'd' && input_stream_->peakNextChar() != 'c') {
                lexError(std::string("invalid format character '") + std::string(1, input_stream_->peakNextChar()) + std::string("'"));
            }
        }
        if (c == '\\') {
            if (input_stream_->peakChar() == 'n') {
                str += "\\n";
                input_stream_->getChar(); // "\\n" -> "\n"
                continue;
            }
            // every thing else is illegal for now
            lexError(std::string("invalid escape character '") + std::string(1, input_stream_->peakChar()) + std::string("'"));
        }
        if (c == EOF) {
            lexError(std::string("unexpected EOF in string literal"));
        }
        str += c;
    }
    return str;
}

std::string Lexer::getNumber() {
    std::string num_str;
    char c;
    if (input_stream_->peakChar() == '0' && input_stream_->peakNextChar() == 'x') {
        // hex number
        input_stream_->getChar();
        input_stream_->getChar();
        while(1) {
            c = input_stream_->getChar();
            if (c == EOF) {
                lexError(std::string("unexpected EOF in hex number"));
                return std::string("0");
            }
            if (c == '_') {
                continue;
            }
            if (isEndForIdentAndNumber(c)) {
                input_stream_->ungetChar();
                break;
            }
            if (!isDigit(c) && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) {
                lexError(std::string("invalid hex number"));
                break;
            }
            num_str += c;
        }
        num_str = "0x" + num_str;
        num_str = std::to_string(strtol(num_str.c_str(), NULL, 16));
    }
    else if (input_stream_->peakChar() == '0' && isDigit(input_stream_->peakNextChar())) {
        // oct number
        input_stream_->getChar();
        while (1)
        {
            c = input_stream_->getChar();
            if (c == EOF) {
                lexError(std::string("unexpected EOF in oct number"));
                return std::string("0");
            }
            if (isEndForIdentAndNumber(c)) {
                input_stream_->ungetChar();
                break;
            }
            if (!isDigit(c)) {
                lexError(std::string("invalid oct number"));
                break;
            }
            num_str += c;
        }
        num_str = std::to_string(strtol(num_str.c_str(), NULL, 8));
    }
    else {
        // decimal number
        while (1)
        {
            c = input_stream_->getChar();
            if (c == EOF) {
                lexError(std::string("unexpected EOF in decimal number"));
                return std::string("0");
            }
            if (isEndForIdentAndNumber(c)) {
                input_stream_->ungetChar();
                break;
            }
            if (!isDigit(c)) {
                lexError(std::string("invalid decimal number"));
                break;
            }
            num_str += c;
        }
    }
    return num_str;
}

TokenPtr Lexer::getIdent() {
    // --- ident_jump is the state change of the state machine ---
    // --- END_OF_ENUM represent the accept state ---
    // --- INIT ONEC ---
    static SyAstType ident_jump[256][(int) SyAstType::END_OF_ENUM];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < (int) SyAstType::END_OF_ENUM; j++) {
                ident_jump[i][j] = SyAstType::END_OF_ENUM;
            }
        }
        // if and int
        ident_jump['i'][(int)SyAstType::END_OF_ENUM] = SyAstType::STM_IF;
        ident_jump['f'][(int)SyAstType::STM_IF] = SyAstType::END_OF_ENUM;
        ident_jump['n'][(int)SyAstType::STM_IF] = SyAstType::TYPE_INT;
        ident_jump['t'][(int)SyAstType::TYPE_INT] = SyAstType::END_OF_ENUM;

        // void
        ident_jump['v'][(int)SyAstType::END_OF_ENUM] = SyAstType::TYPE_VOID;
        ident_jump['o'][(int)SyAstType::TYPE_VOID] = SyAstType::TYPE_VOID;
        ident_jump['i'][(int)SyAstType::TYPE_VOID] = SyAstType::TYPE_VOID;
        ident_jump['d'][(int)SyAstType::TYPE_VOID] = SyAstType::END_OF_ENUM;

        // else
        ident_jump['e'][(int)SyAstType::END_OF_ENUM] = SyAstType::STM_ELSE;
        ident_jump['l'][(int)SyAstType::STM_ELSE] = SyAstType::STM_ELSE;
        ident_jump['s'][(int)SyAstType::STM_ELSE] = SyAstType::STM_ELSE;
        ident_jump['e'][(int)SyAstType::STM_ELSE] = SyAstType::END_OF_ENUM;

        // while
        ident_jump['w'][(int)SyAstType::END_OF_ENUM] = SyAstType::STM_WHILE;
        ident_jump['h'][(int)SyAstType::STM_WHILE] = SyAstType::STM_WHILE;
        ident_jump['i'][(int)SyAstType::STM_WHILE] = SyAstType::STM_WHILE;
        ident_jump['l'][(int)SyAstType::STM_WHILE] = SyAstType::STM_WHILE;
        ident_jump['e'][(int)SyAstType::STM_WHILE] = SyAstType::END_OF_ENUM;

        // break
        ident_jump['b'][(int)SyAstType::END_OF_ENUM] = SyAstType::STM_BREAK;
        ident_jump['r'][(int)SyAstType::STM_BREAK] = SyAstType::STM_BREAK;
        ident_jump['e'][(int)SyAstType::STM_BREAK] = SyAstType::STM_BREAK;
        ident_jump['a'][(int)SyAstType::STM_BREAK] = SyAstType::STM_BREAK;
        ident_jump['k'][(int)SyAstType::STM_BREAK] = SyAstType::END_OF_ENUM;

        // continue and const
        ident_jump['c'][(int)SyAstType::END_OF_ENUM] = SyAstType::STM_CONTINUE;
        ident_jump['o'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONTINUE;
        ident_jump['n'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONTINUE;
        ident_jump['s'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONST;
        ident_jump['t'][(int)SyAstType::STM_CONST] = SyAstType::END_OF_ENUM;
        ident_jump['t'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONTINUE;
        ident_jump['i'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONTINUE;
        ident_jump['n'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONTINUE;
        ident_jump['u'][(int)SyAstType::STM_CONTINUE] = SyAstType::STM_CONTINUE;
        ident_jump['e'][(int)SyAstType::STM_CONTINUE] = SyAstType::END_OF_ENUM;

        // return
        ident_jump['r'][(int)SyAstType::END_OF_ENUM] = SyAstType::STM_RETURN;
        ident_jump['e'][(int)SyAstType::STM_RETURN] = SyAstType::STM_RETURN;
        ident_jump['t'][(int)SyAstType::STM_RETURN] = SyAstType::STM_RETURN;
        ident_jump['u'][(int)SyAstType::STM_RETURN] = SyAstType::STM_RETURN;
        ident_jump['r'][(int)SyAstType::STM_RETURN] = SyAstType::STM_RETURN;
        ident_jump['n'][(int)SyAstType::STM_RETURN] = SyAstType::END_OF_ENUM;
        init = true;
    }
    char current_char = input_stream_->getChar();
    SyAstType current_state = ident_jump[current_char][(int)SyAstType::END_OF_ENUM];
    std::string ident = std::string(1, current_char);
    while (1)
    {
        current_char = input_stream_->getChar();
        if (isIdentPart(current_char)) {
            if (current_state != SyAstType::END_OF_ENUM && 
                ident_jump[current_char][(int)current_state] == SyAstType::END_OF_ENUM &&
                isEndForIdentAndNumber(input_stream_->peakChar())) {
                // this is a STMT or TYPE or so
                ident += current_char;
                return std::make_shared<AstNode>(current_state, line_, std::move(ident));
            }
            current_state = ident_jump[current_char][(int)current_state];
            ident += current_char;
        } 
        else if (isEndForIdentAndNumber(current_char)) {
            input_stream_->ungetChar();
            return std::make_shared<AstNode>(SyAstType::IDENT, line_, std::move(ident));
        }
        else {
            lexError(std::string("invalid character '") + current_char + std::string("'in ident"));
        }
    }
}

TokenPtr Lexer::getNextTokenInternal() {
    char current_char;
    std::string str; // store the string literal
    while ((current_char = input_stream_->peakChar()) != EOF) {
        switch (current_char)
        {
            case '(':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::LEFT_PARENTHESE, line_, std::string("("));
            case ')':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::RIGHT_PARENTHESE, line_, std::string(")"));
            case '[':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::LEFT_BRACKET, line_, std::string("["));
            case ']':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::RIGHT_BRACKET, line_, std::string("]"));
            case '{':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::LEFT_BRACE, line_, std::string("{"));
            case '}':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::RIGHT_BRACE, line_, std::string("}"));
            case '+':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::ALU_ADD, line_, std::string("+"));
            case '-':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::ALU_SUB, line_, std::string("-"));
            case '/':
                if (input_stream_->peakNextChar() == '/') {
                    input_stream_->getLine();
                    line_++;
                    continue;
                }
                else if (input_stream_->peakNextChar() == '*') {
                    while (1) {
                        input_stream_->getChar();
                        if (input_stream_->peakChar() == '*') {
                            input_stream_->getChar();
                            if (input_stream_->peakChar() == '/') {
                                input_stream_->getChar();
                                break;
                            }
                        }
                        if (input_stream_->peakChar() == EOF) {
                            lexError(std::string("unexpected EOF in comment"));
                            break;
                        }
                    }
                    continue;
                }
                else {
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::ALU_DIV, line_, std::string("/"));
                }
            case '*':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::ALU_MUL, line_, std::string("*"));
            case '%':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::ALU_MOD, line_, std::string("%"));
            case '=':
                if ('=' == input_stream_->peakNextChar()) {
                    input_stream_->getChar();
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::EQ, line_, std::string("=="));
                }
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::ASSIGN, line_, std::string("="));
            case '!':
                if ('=' == input_stream_->peakNextChar()) {
                    input_stream_->getChar();
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::NEQ, line_, std::string("!="));
                }
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::LOGIC_NOT, line_, std::string("!"));
            case '<':
                if ('=' == input_stream_->peakNextChar()) {
                    input_stream_->getChar();
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::LE, line_, std::string("<="));
                }
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::LNE, line_, std::string("<"));
            case '>':
                if ('=' == input_stream_->peakNextChar()) {
                    input_stream_->getChar();
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::GE, line_, std::string(">="));
                }
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::GNE, line_, std::string(">"));
            case '&':
                if ('&' == input_stream_->peakNextChar()) {
                    input_stream_->getChar();
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::LOGIC_AND, line_, std::string("&&"));
                }
                else {
                    lexError(std::string("invalid token '&'"));
                    input_stream_->getChar();
                    continue;
                }
            case '|':
                if ('|' == input_stream_->peakNextChar()) {
                    input_stream_->getChar();
                    input_stream_->getChar();
                    return std::make_shared<AstNode>(SyAstType::LOGIC_OR, line_, std::string("||"));
                }
                else {
                    lexError(std::string("invalid token '|'"));
                    input_stream_->getChar();
                    continue;
                }
            case ';':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::SEMICOLON, line_, std::string(";"));
            case '"':
                input_stream_->getChar();
                str = getString();
                return std::make_shared<AstNode>(SyAstType::STRING, line_, std::move(str));
            case ',':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::COMMA, line_, std::string(","));
            case '.':
                input_stream_->getChar();
                return std::make_shared<AstNode>(SyAstType::DOT, line_, std::string("."));
            case '\n':
                input_stream_->getChar();
                line_++;
                continue;
            case '\t':
            case ' ':
                input_stream_->getChar();
                continue;
            case '\b':
                input_stream_->getChar();
                lexError(std::string("invalid token '\\b'"));
                continue;
            case '\r':
                input_stream_->getChar();
                continue;
            default:
                break;
        }
        if (isDigit(current_char)) {
            return std::make_shared<AstNode>(SyAstType::INT_IMM, line_, std::move(getNumber()));
        }
        else if (isIdentStart(current_char)) {
            return getIdent();
        }
        else {
            lexError(std::string("invalid token '") + current_char + std::string("'"));
            input_stream_->getChar();
        }
    };
    return std::make_shared<AstNode>(SyAstType::EOF_TYPE, line_, std::string("EOF"));
}

TokenPtr Lexer::getNextToken() {
    if (current_token_ == nullptr) {
        current_token_ = getNextTokenInternal();
        return current_token_;
    }
    if (current_token_->ast_type_ == SyAstType::EOF_TYPE) {
        return current_token_;
    }
    auto next_token = getNextTokenInternal();
    current_token_->next_token_ = next_token;
    next_token->prev_token_ = current_token_;
    current_token_ = next_token;
    return current_token_;
}

TokenPtr Lexer::getNextToken(TokenPtr token) {
    if (token == current_token_) {
        return getNextToken();
    } 
    else {
        return token->next_token_;
    }
}

TokenPtr Lexer::getPrevToken(TokenPtr token) {
    return token->prev_token_;
}

// start the parser code
// 基本原则：
// 1. 每个生成函数在匹配失败时不考虑返回时迭代器的正确性，此时由调用函数来保证迭代器的正确性
// 2. 每个生成函数在匹配成功时，返回时的迭代器应当指向下一个要匹配的 token
// 3. 进入生成函数时，迭代器应当指向将要被识别的 token，而不是识别过的 token
// 4. 暂时不考虑错误处理
// 5. 行数的维护尽量懒惰，不要每次都更新
// 6. 匹配成功并返回的情况应该尽量在函数的最后

// 可能出现的问题：
// 1. 匹配时修改了某 token 的 parent_, a_... 字段，但是后来匹配失败了，
// 可能会产生无效引用（由于使用了智能指针，应该不会是悬垂指针，但是仍可能有问题）

AstNodePtr Parser::ConstInitVal() {}
AstNodePtr Parser::VarDecl() {}
AstNodePtr Parser::VarDef() {}
AstNodePtr Parser::InitVal() {}
AstNodePtr Parser::Block() {}
AstNodePtr Parser::BlockItem() {}
AstNodePtr Parser::Stmt() {}
AstNodePtr Parser::Cond() {}
AstNodePtr Parser::LVal() {}
AstNodePtr Parser::PrimaryExp() {}
AstNodePtr Parser::Number() {}
AstNodePtr Parser::UnaryExp() {}
AstNodePtr Parser::UnaryOp() {}
AstNodePtr Parser::FuncRParams() {}
AstNodePtr Parser::RelExp() {}
AstNodePtr Parser::EqExp() {}
AstNodePtr Parser::LAndExp() {}
AstNodePtr Parser::LOrExp() {}
AstNodePtr Parser::ConstExp() {}

// this two helper can adjust AddExp and MulExp
// this comment inside uses AddExp as example
// because the node->ebnf_type_ need to be reseted
// so an isAddExp is used to distinguish the two cases
static void adjustAddMulExpLAst(AstNodePtr node, bool isAddExp) {
    // before addjust, node is already an AddExp
    // but node->b_ is nullptr
    // after addjust, node->b_ is an '+' or '-',
    // and node->c_ is an AddExp, and node->c_->b_ is nullptr
    if (node->c_->ebnf_type_ == SyEbnfType::E) {
        // replace node to node->a_, then node turns to a MulExp
        node = node->a_;
        node->ebnf_type_ = isAddExp ? SyEbnfType::MulExp : SyEbnfType::UnaryExp;
        return;
    }
    else {
        auto add_exp_l = node->c_;
        node->b_ = add_exp_l->a_;
        add_exp_l->a_ = add_exp_l->b_;
        add_exp_l->b_ = nullptr;
        node->ebnf_type_ = isAddExp ? SyEbnfType::AddExp : SyEbnfType::MulExp;
        adjustAddMulExpLAst(add_exp_l, isAddExp);
    }
}

static void adjustAddMulExpAst(AstNodePtr node, bool isAddExp) {
    // before addjust, node->b_ is an AddExpL
    // after addjust, node->b_ is an '+' or '-', 
    // and node->c_ is an AddExp
    if (node->b_ != nullptr) {
        auto add_exp_l = node->b_;
        node->c_ = add_exp_l;
        node->b_ = add_exp_l->a_;
        add_exp_l->a_ = add_exp_l->b_;
        add_exp_l->b_ = nullptr;
        adjustAddMulExpLAst(add_exp_l, isAddExp);
    }
}

// this parse won't return nullptr (it success all the time)
AstNodePtr Parser::MulExpL() {
    LexerIterator iter_back = *token_iter_;
    // MulExpL -> ('*' | '/' | '%') UnaryExp MulExpL | e
    if ((*token_iter_)->ast_type_ != SyAstType::ALU_MUL &&
        (*token_iter_)->ast_type_ != SyAstType::ALU_DIV &&
        (*token_iter_)->ast_type_ != SyAstType::ALU_MOD) {
        // this is not an failure
        // just return e
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    auto mul_exp_l = std::make_shared<AstNode>(SyEbnfType::END_OF_ENUM, (*token_iter_)->line_);
    mul_exp_l->a_ = *(*token_iter_);
    ++(*token_iter_);
    auto unary_exp = UnaryExp();
    if (unary_exp == nullptr) {
        // this is no a failure
        // just return e
        *token_iter_ = iter_back;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    mul_exp_l->b_ = unary_exp;
    auto mul_exp_l_ = MulExpL();
    return mul_exp_l_;
}

AstNodePtr Parser::MulExp() {
    // origin: MulExp -> UnaryExp | MulExp ('*' | '/' | '%') UnaryExp
    // rewrite: MulExp -> UnaryExp MulExpL
    //          MulExpL -> ('*' | '/' | '%') UnaryExp MulExpL | e
    auto mul_exp = std::make_shared<AstNode>(SyEbnfType::MulExp, (*token_iter_)->line_);
    auto unary_exp = UnaryExp();
    if (unary_exp == nullptr) {
        return nullptr;
    }
    mul_exp->a_ = unary_exp;
    unary_exp->parent_ = mul_exp;
    auto mul_exp_l = MulExpL();
    // merge mul_exp_l
    if (mul_exp_l->ebnf_type_ != SyEbnfType::E) {
        mul_exp->b_ = mul_exp_l;
        mul_exp_l->parent_ = mul_exp;
        adjustAddMulExpAst(mul_exp, 0);
    }
}

// this parse won't return nullptr (it success all the time)
AstNodePtr Parser::AddExpL() {
    LexerIterator iter_back = *token_iter_;
    // AddExpL -> ('+' | '−') MulExp AddExpL | e
    if ((*token_iter_)->ast_type_ != SyAstType::ALU_ADD && (*token_iter_)->ast_type_ != SyAstType::ALU_SUB) {
        // this is not an failure
        // just return e
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    auto add_exp_l = std::make_shared<AstNode>(SyEbnfType::END_OF_ENUM, (*token_iter_)->line_);
    add_exp_l->a_ = *(*token_iter_);
    ++(*token_iter_);
    auto mul_exp = MulExp();
    if (mul_exp == nullptr) {
        // this is not an failure
        // just return e
        *token_iter_ = iter_back;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    add_exp_l->b_ = mul_exp;
    add_exp_l->c_ = AddExpL();
    return add_exp_l;
}

AstNodePtr Parser::AddExp() {
    // origin: AddExp -> MulExp | AddExp ('+' | '−') MulExp 
    // rewrite: AddExp -> MulExp AddExpL
    //          AddExpL -> ('+' | '−') MulExp AddExpL | e
    auto add_exp = std::make_shared<AstNode>(SyEbnfType::AddExp, (*token_iter_)->line_);
    auto mul_exp = MulExp();
    if (mul_exp == nullptr) {
        return nullptr;
    }
    add_exp->a_ = mul_exp;
    mul_exp->parent_ = add_exp;
    auto add_exp_l = AddExpL();
    // merge add_exp_l
    if (add_exp_l->ebnf_type_ != SyEbnfType::E) {
        add_exp->b_ = add_exp_l;
        add_exp_l->parent_ = add_exp;
        // we need to change the AddExpL to AddExp
        adjustAddMulExpAst(add_exp, 1);
    }
    return add_exp;
}

AstNodePtr Parser::Exp() {
    // origin: Exp -> AddExp
    auto add_exp = AddExp();
    if (add_exp == nullptr) {
        return nullptr;
    }
    auto exp = std::make_shared<AstNode>(SyEbnfType::Exp, add_exp->line_);
    exp->a_ = add_exp;
    add_exp->parent_ = exp;
    return exp;
}

AstNodePtr Parser::ConstDef() {
    // origin: ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    // Ident
    auto const_def = std::make_shared<AstNode>(SyEbnfType::ConstDef, (*token_iter_)->line_);
    auto ident = Ident();
    if (ident == nullptr) {
        return nullptr;
    }
    // { '[' ConstExp ']' }
    // get the first ConstExp (if there is any)
    AstNodePtr const_exp_start = nullptr;
    if ((*token_iter_)->ast_type_ == SyAstType::LEFT_BRACKET) {
        ++(*token_iter_);
        const_exp_start = ConstExp();
        if (const_exp_start == nullptr) {
            return nullptr;
        }
        // get all ConstExps left, and link them
        auto const_exp_last = const_exp_start;
        while ((*token_iter_)->ast_type_ == SyAstType::RIGHT_BRACKET) {
            ++(*token_iter_);
            auto const_exp = ConstExp();
            const_exp_last->a_ = const_exp;
            const_exp->parent_ = const_exp_last;
            const_exp_last = const_exp;
        }
    }

    // '='
    if ((*token_iter_)->ast_type_ != SyAstType::ASSIGN) {
        return nullptr;
    }
    ++(*token_iter_);

    // ConstInitVal
    // !!!TODO!!!
}

AstNodePtr Parser::ConstDecl() {
    // origin: ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    if ((*token_iter_)->ast_type_ != SyAstType::STM_CONST) {
        return nullptr;
    }
    auto const_decl = std::make_shared<AstNode>(SyEbnfType::ConstDecl, (*token_iter_)->line_);
    ++(*token_iter_);
    auto b_type = BType();
    if (b_type == nullptr) {
        return nullptr;
    }
    auto const_def = ConstDef();
}

AstNodePtr Parser::Decl() {
    // origin: Decl -> ConstDecl | VarDecl
    LexerIterator iter_back = *token_iter_;
    // ConstDecl
    auto const_decl = ConstDecl();
    if (const_decl != nullptr) {
        return const_decl;
    }
    // VarDecl
    *token_iter_ = iter_back;
    auto var_decl = VarDecl();
    if (var_decl != nullptr) {
        return var_decl;
    }
}

AstNodePtr Parser::FuncType() {
    // origin: FuncType -> 'void' | 'int'
    AstNodePtr token = **token_iter_;
    if (token->ast_type_ == SyAstType::TYPE_VOID || token->ast_type_ == SyAstType::TYPE_INT) {
        auto func_type = std::make_shared<AstNode>(SyEbnfType::FuncType, token->line_);
        func_type->a_ = token;
        token->parent_ = func_type;
        ++(*token_iter_);
        return func_type;
    }
    else {
        return nullptr;
    }
}

// !!! this function should be rewritten according to the principle no.6
AstNodePtr Parser::BType() {
    // origin: BType -> 'int'
    if ((*token_iter_)->ast_type_ == SyAstType::TYPE_INT) {
        auto b_type = std::make_shared<AstNode>(SyEbnfType::BType, (*token_iter_)->line_);
        b_type->a_ = **token_iter_;
        (*token_iter_)->parent_ = b_type;
        ++(*token_iter_);
        return b_type;
    }
    else {
        return nullptr;
    }
}

AstNodePtr Parser::FuncFParam() {
    // origin: FuncFParam -> Type Ident
    auto func_f_param = std::make_shared<AstNode>(SyEbnfType::FuncFParam, (*token_iter_)->line_);
    auto b_type = BType();
    // Type
    if (b_type == nullptr) {
        return nullptr;
    }
    // Ident
    auto ident = Ident();
    if (ident == nullptr) {
        return nullptr;
    }

    // link when suceess
    func_f_param->a_ = b_type;
    b_type->parent_ = func_f_param;
    func_f_param->b_ = ident;
    ident->parent_ = func_f_param;
    return func_f_param;
}

AstNodePtr Parser::FuncFParams() {
    // origin: FuncFParams -> '(' FuncFParam {',' FuncFParam} ')'
    // '('
    if ((*token_iter_)->ast_type_ != SyAstType::LEFT_PARENTHESE) {
        return nullptr;
    }
    ++(*token_iter_);
    // FuncFParam
    auto func_f_param_start = FuncFParam();
    auto func_f_param_last = func_f_param_start;
    // {',' FuncFParam}
    while ((*token_iter_)->ast_type_ == SyAstType::COMMA) {
        ++(*token_iter_);
        auto func_f_param = FuncFParam();
        if (func_f_param == nullptr) {
            return nullptr;
        }
        // link when suceess
        func_f_param_last->a_ = func_f_param;
        func_f_param->parent_ = func_f_param_last;
        func_f_param_last = func_f_param;
    }
    // ')'
    if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
        return nullptr;
    }
    return func_f_param_start;
}

AstNodePtr Parser::FuncDef() {
    // origin: FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
    auto func_def = std::make_shared<AstNode>(SyEbnfType::FuncDef, (*token_iter_)->line_);
    // FuncType
    auto func_type = FuncType();
    if (func_type == nullptr) {
        return nullptr;
    }
    // Ident
    auto ident = Ident();
    if (ident == nullptr) {
        return nullptr;
    }
    // '('
    if ((*token_iter_)->ast_type_ != SyAstType::LEFT_PARENTHESE) {
        return nullptr;
    }
    ++(*token_iter_);
    // [FuncFParams]
    LexerIterator token_back = *token_iter_;
    auto func_f_params = FuncFParams();
    if (func_f_params == nullptr) {
        // func_f_params can be nullptr
        // however this means FuncFParams() failed
        // so we need to reset token_iter_ to token_back
        *token_iter_ = token_back;
    }
    if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
        return nullptr;
    }
    ++(*token_iter_);
    auto block = Block();
    if (block == nullptr) {
        return nullptr;
    }

    // if we reach here, it means that we successed
    // so we need to set the parent of all the nodes in func_def
    // and set the children of func_def
    func_def->a_ = func_type;
    func_type->parent_ = func_def;
    func_def->b_ = ident;
    ident->parent_ = func_def;
    func_def->c_ = func_f_params;
    if (func_f_params != nullptr) {
        func_f_params->parent_ = func_def;
    }
    func_def->d_ = block;
    block->parent_ = func_def;
    return func_def;
}

AstNodePtr Parser::CompUnit() {
    // origin: CompUnit -> [ CompUnit ] ( Decl | FuncDef ) 
    // changed: CompUnit -> Decl | FuncDef
    LexerIterator iter_back = *token_iter_;
    auto decl = Decl();
    if (decl != nullptr) {
        return decl;
    }
    *token_iter_ = iter_back;
    auto func_def = FuncDef();
    if (func_def != nullptr) {
        return func_def;
    }
    return nullptr;
}

AstNodePtr Parser::Ident() {
    if ((*token_iter_)->ast_type_ != SyAstType::IDENT) {
        return nullptr;
    }
    ++(*token_iter_);
    return *(*token_iter_);
}

AstNodePtr Parser::parse() {
}
