#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include "syparse.h"

static void adjustExpLAst(AstNodePtr node);
static void adjustExpAst(AstNodePtr node);

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
        || c == '}' || c == '\t' || c == '\n';
}

void Lexer::lexError(std::string msg) {
    fprintf(stderr, "\033[1m\033[31mError in lexing\033[0m: line \033[1m%d\033[0m: %s\n", line_, msg.c_str());
    error_occured_ = 1;
}

void Lexer::lexWarning(std::string msg) {
    fprintf(stderr, "\033[1m\033[33mWarning in lexing\033[0m: line \033[1m%d\033[0m: %s\n", line_, msg.c_str());
}

void Parser::parseError(std::string msg, int line) {
    fprintf(stderr, "\033[1m\033[31mError in parser\033[0m: line \033[1m%d\033[0m: %s\n", line, msg.c_str());
    error_occured_ = 1;
}

std::string Lexer::getString() {
    std::string str;
    lexWarning("string is no supported yet, complier will stop after scaning");
    error_occured_ = 1;
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
                ident_jump[i][j] = SyAstType::IDENT;
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
                // as string is not supported, here we continue instead of returning the string token
                continue;
                // return std::make_shared<AstNode>(SyAstType::STRING, line_, std::move(str));
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
// 简单的递归下降法 parser
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
// 2. 有些地方的产生是可选的，所以匹配失败时仍应继续，但是这里很可能会忘记恢复 token 迭代器

// 当前的问题：
// 1. 重复代码略多，之后可以进行重构，合并生成式相同的函数

AstNodePtr Parser::Stmt() {
    // origin: Stmt -> LVal '=' Exp ';' | [Exp] ';' | Block
    //                 | 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
    //                 | 'while' '(' Cond ')' Stmt
    //                 | 'break' ';' | 'continue' ';'
    //                 | 'return' [Exp] ';'
    auto stmt = std::make_shared<AstNode>(SyEbnfType::Stmt, (*token_iter_)->line_);
    AstNodePtr cond = nullptr;
    AstNodePtr stmt_nest = nullptr;
    AstNodePtr stmt_else = nullptr;
    AstNodePtr exp = nullptr;
    LexerIterator iter_back = *token_iter_;
    LexerIterator iter_back_tmp = *token_iter_;
    switch ((*token_iter_)->ast_type_) {
        case SyAstType::STM_IF:
        // 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
            stmt->a_ = *(*token_iter_);
            (*token_iter_)->parent_ = stmt;
            ++(*token_iter_);
            if ((*token_iter_)->ast_type_ != SyAstType::LEFT_PARENTHESE) {
                // here is status like
                // ```
                // if !a) {
                //     ...
                // }
                // ```
                // so we just ignore the missing '(' and report a error
                parseError(std::string("expected \'\033[1m(\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
                --(*token_iter_);
            }
            ++(*token_iter_);
            iter_back_tmp = *token_iter_;
            cond = Cond();
            if (cond == nullptr) {
                // here we want to get a Cond expression, but we get nothing
                // we just jump to the next ')', and report a error
                parseError(std::string("expected a condition expression after \'\033[1m(\033[0m\'"), stmt->line_);
                *token_iter_ = iter_back_tmp;
                while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
                    if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                        // there we giveup
                        return nullptr;
                    }
                    ++(*token_iter_);
                }
                cond = std::make_shared<AstNode>(SyEbnfType::E, 0);
            }
            if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
                // here is status like
                // ```
                // if (!a {
                //     ...
                // }
                // ```
                // so we just ignore the missing ')' and report a error
                parseError(std::string("expected \'\033[1m)\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
                --(*token_iter_);
            }
            ++(*token_iter_);
            stmt_nest = Stmt();
            if (stmt_nest == nullptr) {
                // here is really tricky, it's hard to tell whether the error is
                // so we just return nullptr and maybe giveup this CompUnit
                return nullptr;
            }
            if ((*token_iter_)->ast_type_ == SyAstType::STM_ELSE) {
                stmt_else = Stmt();
                if (stmt_else == nullptr) {
                    // here is really tricky, it's hard to tell whether the error is
                    // so we just return nullptr and maybe giveup this CompUnit
                    return nullptr;
                }
            }
            stmt->b_ = cond;
            cond->parent_ = stmt;
            stmt->c_ = stmt_nest;
            stmt_nest->parent_ = stmt;
            if (stmt_else != nullptr) {
                stmt->d_ = stmt_else;
                stmt_else->parent_ = stmt;
            }
            return stmt;
        case SyAstType::STM_WHILE:
        // 'while' '(' Cond ')' Stmt
        // error handling is almost same as 'if'
            stmt->a_ = *(*token_iter_);
            (*token_iter_)->parent_ = stmt;
            ++(*token_iter_);
            if ((*token_iter_)->ast_type_ != SyAstType::LEFT_PARENTHESE) {
                parseError(std::string("expected \'\033[1m(\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
                --(*token_iter_);
            }
            ++(*token_iter_);
            cond = Cond();
            if (cond == nullptr) {
                parseError(std::string("expected a condition expression after \'\033[1m(\033[0m\'"), stmt->line_);
                *token_iter_ = iter_back_tmp;
                while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
                    if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                        // there we giveup
                        return nullptr;
                    }
                    ++(*token_iter_);
                }
                cond = std::make_shared<AstNode>(SyEbnfType::E, 0);
            }
            if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
                parseError(std::string("expected \'\033[1m)\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
                --(*token_iter_);
            }
            ++(*token_iter_);
            stmt_nest = Stmt();
            if (stmt_nest == nullptr) {
                // here is really tricky, it's hard to tell whether the error is
                // so we just return nullptr and maybe giveup this CompUnit
                return nullptr;
            }
            stmt->b_ = cond;
            cond->parent_ = stmt;
            stmt->c_ = stmt_nest;
            stmt_nest->parent_ = stmt;
            return stmt;
        case SyAstType::STM_BREAK:
        // 'break' ';'
            stmt->a_ = *(*token_iter_);
            (*token_iter_)->parent_ = stmt;
            ++(*token_iter_);
            if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
                // ignore the missing ';' and report a error
                parseError(std::string("expected \'\033[1m;\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
                 --(*token_iter_);
            }
            ++(*token_iter_);
            return stmt;
        case SyAstType::STM_CONTINUE:
        // 'continue' ';'
            stmt->a_ = *(*token_iter_);
            (*token_iter_)->parent_ = stmt;
            ++(*token_iter_);
            if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
                // ignore the missing ';' and report a error
                parseError(std::string("expected \'\033[1m;\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
                 --(*token_iter_);
            }
            ++(*token_iter_);
            return stmt;
        case SyAstType::STM_RETURN:
        // 'return' [Exp] ';'
            stmt->a_ = *(*token_iter_);
            (*token_iter_)->parent_ = stmt;
            ++(*token_iter_);
            iter_back = *token_iter_;
            exp = Exp();
            if (exp == nullptr) {
                // that's ok at least in the parsing phase
                // typing phase will check if it is valid
                // just reset the iterator
                *token_iter_ = iter_back;
            }
            else {
                // link
                stmt->b_ = exp;
                exp->parent_ = stmt;
            }
            if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
                // ignore the missing ';' and report a error
                parseError(std::string("expected \'\033[1m;\033[0m\' before \'\033[1m")
                 + (*token_iter_)->literal_ + std::string("\033[0m\'"), (*token_iter_)->line_);
                 --(*token_iter_);
            }
            ++(*token_iter_);
            return stmt;
        default:
            break;
    }
    // now we should try Stmt -> LVal '=' Exp ';' | [Exp] ';' | Block
    // try LVal '=' Exp ';'
    iter_back = *token_iter_;
    auto l_val = LVal();
    if (l_val != nullptr) {
        if ((*token_iter_)->ast_type_ == SyAstType::ASSIGN) {
            ++(*token_iter_);
            exp = Exp();
            if (exp == nullptr) {
                return nullptr;
            }
            if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
                // TODO error handle
                return nullptr;
            }
            ++(*token_iter_);
            stmt->a_ = l_val;
            l_val->parent_ = stmt;
            stmt->b_ = exp;
            exp->parent_ = stmt;
            return stmt;
        }
    }
    // then try [Exp] ';'
    // rewrite [Exp] ';' => Exp ';' | ';'
    // first we try ';'
    *token_iter_ = iter_back;
    if ((*token_iter_)->ast_type_ == SyAstType::SEMICOLON) {
        ++(*token_iter_);
        // for simplicity, we just return a null stmt
        // no fancy optimization here
        return stmt;
    }
    // then try Exp ';'
    *token_iter_ = iter_back;
    exp = Exp();
    if (exp != nullptr) {
        if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
            // ignore the missing ';' and report a error
            parseError(std::string("expected \'\033[1m;\033[0m\' before \'\033[1m")
             + (*token_iter_)->literal_ + std::string("\033[0m\'"), stmt->line_);
             --(*token_iter_);
        }
        ++(*token_iter_);
        stmt->a_ = exp;
        exp->parent_ = stmt;
        return stmt;
    }
    // try Block
    *token_iter_ = iter_back;
    auto block = Block();
    if (block == nullptr) {
        // here we giveup
        return nullptr;
    }
    stmt->a_ = block;
    block->parent_ = stmt;
    return stmt;
}

AstNodePtr Parser::VarDef() {
    // origin: VarDef -> Ident { '[' ConstExp ']' } | Ident { '[' ConstExp ']' } '=' InitVal 
    auto ident = Ident();
    if (ident == nullptr) {
        return nullptr;
    }
    AstNodePtr const_exp_start = nullptr;
    AstNodePtr const_exp_last = nullptr;
    LexerIterator iter_back = *token_iter_;
    while ((*token_iter_)->ast_type_ == SyAstType::LEFT_BRACKET) {
        ++(*token_iter_);
        iter_back = *token_iter_;
        auto const_exp = ConstExp();
        if (const_exp == nullptr) {
            // here we want to get a const_exp, but we get nothing
            // we just jump to the next ']', and report a error
            parseError(std::string("expected a const expression after \'\033[1m[\033[0m\'"), ident->line_);
            *token_iter_ = iter_back;
            while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACKET) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            const_exp = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
        if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACKET) {
            // just ignore the missing ']' and report a error
            parseError(std::string("expected \'\033[1m]\033[0m\' before \'\033[1m")
             + (*token_iter_)->literal_ + std::string("\033[0m\'"), ident->line_);
             --(*token_iter_);
        }
        ++(*token_iter_);
        if (const_exp_start == nullptr) {
            const_exp_start = const_exp;
            const_exp_last = const_exp;
        }
        else {
            const_exp_last->d_ = const_exp;
            const_exp->parent_ = const_exp_last;
            const_exp_last = const_exp;
        }
    }
    AstNodePtr init_val = nullptr;
    // '=' InitVal
    if ((*token_iter_)->ast_type_ == SyAstType::ASSIGN) {
        // it's a var def with init val
        ++(*token_iter_);
        init_val = InitVal();
        if (init_val == nullptr) {
            // VarDef only end with ';' or ','
            // so here we just find the nearest ';' or ','
            // and report an error
            parseError(std::string("expected a init value after \'\033[1m=\033[0m\'"), ident->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON
             && (*token_iter_)->ast_type_ != SyAstType::COMMA) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            init_val = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
    }
    auto var_def = std::make_shared<AstNode>(SyEbnfType::VarDef, ident->line_);
    var_def->a_ = ident;
    ident->parent_ = var_def;
    if (const_exp_start != nullptr) {
        var_def->b_ = const_exp_start;
        const_exp_start->parent_ = var_def;
    }
    if (init_val != nullptr) {
        var_def->c_ = init_val;
        init_val->parent_ = var_def;
    }
    return var_def;
}

AstNodePtr Parser::VarDecl() {
    // origin: VarDecl -> BType VarDef { ',' VarDef } ';'
    auto b_type = BType();
    if (b_type == nullptr) {
        return nullptr;
    }
    auto var_def_start = VarDef();
    auto var_def_last = var_def_start;
    if (var_def_start == nullptr) {
        return nullptr;
    }
    while ((*token_iter_)->ast_type_ == SyAstType::COMMA) {
        ++(*token_iter_);
        LexerIterator iter_back = *token_iter_;
        auto var_def = VarDef();
        if (var_def == nullptr) {
            // here we just find the nearest ';' or ','
            // and report an error
            *token_iter_ = iter_back;
            parseError(std::string("expected a init value after \'\033[1m,\033[0m\'"), (*token_iter_)->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON
             && (*token_iter_)->ast_type_ != SyAstType::COMMA) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            var_def = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
        var_def_last->d_ = var_def;
        var_def->parent_ = var_def_last;
        var_def_last = var_def;
    }
    if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
        // here we can't ignore the missing ';'
        // example: int main()
        // so we giveup
        // as a result, error like `int a, b, c` won't be recoverable
        return nullptr;
    }
    ++(*token_iter_);
    auto var_decl = std::make_shared<AstNode>(SyEbnfType::VarDecl, b_type->line_);
    var_decl->a_ = b_type;
    b_type->parent_ = var_decl;
    var_decl->b_ = var_def_start;
    var_def_start->parent_ = var_decl;
    return var_decl;
}

AstNodePtr Parser::InitVal() {
    // origin: InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
    auto init_val = std::make_shared<AstNode>(SyEbnfType::InitVal, (*token_iter_)->line_);
    // first we try to match '{'
    if ((*token_iter_)->ast_type_ != SyAstType::LEFT_BRACE) {
        // then we try to match Exp
        auto exp = Exp();
        if (exp == nullptr) {
            return nullptr;
        }
        init_val->a_ = exp;
        exp->parent_ = init_val;
        return init_val;
    }
    // '{' is matched
    ++(*token_iter_);
    LexerIterator iter_back = *token_iter_;
    // init_val_can be nested, so this init_val list is just a list, a_ point
    // to the real init_val (list)
    AstNodePtr init_val_nest = InitVal();
    AstNodePtr init_val_start(nullptr);
    AstNodePtr init_val_last(nullptr);
    if (init_val_nest != nullptr) {
        init_val_start = std::make_shared<AstNode>(SyEbnfType::InitVal, init_val_nest->line_);
        init_val_start->a_ = init_val_nest;
        init_val_nest->parent_ = init_val_start;
        init_val_last = init_val_start;
    }
    if (init_val_start == nullptr) {
        // it ok to return nullptr if the next token is '}'
        // it means in src, it just uses the "{}" to init
        // so we still need to find the '}'
        // first reset the token_iter_
        *token_iter_ = iter_back;
        if((*token_iter_)->ast_type_ == SyAstType::RIGHT_BRACE) {
            ++(*token_iter_);
            return init_val;
        }
        // if we reach here, it means in {...}, ... is invaild
        // we don't care if ... is like .., ...
        // just find the nearest '}' and report an error
        parseError(std::string("expected a init value after \'\033[1m{\033[0m\'"), (*token_iter_)->line_);
        while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
            if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                // there we giveup
                return nullptr;
            }
            ++(*token_iter_);
        } 
        ++(*token_iter_);
        return init_val;
    }
    else {
        while ((*token_iter_)->ast_type_ == SyAstType::COMMA)
        {
            ++(*token_iter_);
            iter_back = *token_iter_;
            auto init_val_nest = InitVal();
            AstNodePtr init_val_next(nullptr);
            if (init_val_nest != nullptr) {
                init_val_next = std::make_shared<AstNode>(SyEbnfType::InitVal, init_val_nest->line_);
                init_val_next->a_ = init_val_nest;
                init_val_nest->parent_ = init_val_next;
            }
            if (init_val_next == nullptr) {
                // here we just find the nearest '}' or ',' and report an error
                *token_iter_ = iter_back;
                parseError(std::string("expected a init value after \'\033[1m,\033[0m\'"), (*token_iter_)->line_);
                while ((*token_iter_)->ast_type_ != SyAstType::COMMA &&
                       (*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
                    if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                        // there we giveup
                        return nullptr;
                    }
                    ++(*token_iter_);
                }
                init_val_next = std::make_shared<AstNode>(SyEbnfType::E, 0);
            }
            // link
            init_val_last->d_ = init_val_next;
            init_val_next->parent_ = init_val_last;
            init_val_last = init_val_next;
        }
        if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
            // here we just ignore the missing '}' and report an error
            parseError(std::string("expected a \'\033[1m}\033[0m\' in initial list"), (*token_iter_)->line_);
            --(*token_iter_);
        }
        ++(*token_iter_);
        if (init_val_start != nullptr) {
            init_val->a_ = init_val_start;
            init_val_start->parent_ = init_val; 
        }
        return init_val;
    }
}

AstNodePtr Parser::Block() {
    // origin: Block -> '{' { BlockItem } '}'
    if ((*token_iter_)->ast_type_ != SyAstType::LEFT_BRACE) {
        return nullptr;
    }
    auto block = std::make_shared<AstNode>(SyEbnfType::Block, (*token_iter_)->line_);
    ++(*token_iter_);
    if ((*token_iter_)->ast_type_ == SyAstType::RIGHT_BRACE) {
        // this is a empty block
        // just return
        return block;
    }
    AstNodePtr block_item_start = nullptr;
    AstNodePtr block_item_last = nullptr;
    while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
        // if we reach here, then there must have a block item ahead
        LexerIterator iter_back = *token_iter_;
        auto block_item = BlockItem();
        if (block_item == nullptr) {
            // here we couldn't get any BlockItem, 
            // so we can just try to find the nearest '}'
            // and report an error
            *token_iter_ = iter_back;
            parseError(std::string("expected a block item after \'\033[1m{\033[0m\'"), (*token_iter_)->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            break;
        }
        if (block_item_start == nullptr) {
            block_item_start = block_item;
            block_item_last = block_item;
        }
        else {
            // link
            block_item_last->d_ = block_item;
            block_item->parent_ = block_item_last;
            block_item_last = block_item;
        }
    }
    ++(*token_iter_);
    if (block_item_start != nullptr) {
        block->a_ = block_item_start;
        block_item_start->parent_ = block;
    }
    return block;
}

AstNodePtr Parser::BlockItem() {
    // origin: BlockItem -> Decl | Stmt
    LexerIterator iter_back = *token_iter_;
    auto block_item = std::make_shared<AstNode>(SyEbnfType::BlockItem, (*token_iter_)->line_);
    // try Decl
    auto decl = Decl();
    if (decl != nullptr) {
        block_item->a_ = decl;
        decl->parent_ = block_item;
        return block_item;
    }
    // try Stmt
    *token_iter_ = iter_back;
    auto stmt = Stmt();
    if (stmt == nullptr) {
        // here we couldn't get any Stmt or Decl,
        // we giveup
        return nullptr;
    }
    block_item->a_ = stmt;
    stmt->parent_ = block_item;
    return block_item;
}

AstNodePtr Parser::Cond() {
    // origin: Cond -> LOrExp
    auto l_or_exp = LOrExp();
    if (l_or_exp == nullptr) {
        // giveup
        return nullptr;
    }
    auto cond = std::make_shared<AstNode>(SyEbnfType::Cond, l_or_exp->line_);
    cond->a_ = l_or_exp;
    l_or_exp->parent_ = cond;
    return  cond;
}

AstNodePtr Parser::Number() {
    // origin: Number -> IntConst
    if ((*token_iter_)->ast_type_ != SyAstType::INT_IMM) {
        // giveup
        return nullptr;
    }
    auto number = std::make_shared<AstNode>(SyEbnfType::Number, (*token_iter_)->line_);
    number->a_ = *(*token_iter_);
    (*token_iter_)->parent_ = number;
    ++(*token_iter_);
    return number;
}

// todo: parser for MulExp, AddExp, RelExp, EqExp, LAndExp, LOrExp maybe can be
// rewrited to a single template function
// this two helper can adjust MulExp, AddExp, RelExp, EqExp, LAndExp, LOrExp 
// they are all left recursive in the same form
// this comment inside uses AddExp as example
static void adjustExpLAst(AstNodePtr node) {
    // before addjust, node is already an AddExp
    // but node->b_ is nullptr
    // after addjust, node->b_ is an '+' or '-',
    // and node->c_ is an AddExp, and node->c_->b_ is nullptr
    if (node->c_->ebnf_type_ == SyEbnfType::E) {
        // replace node to node->a_, then node turns to a MulExp
        SyEbnfType ebnf_type;
        // in the switch, node is an AddExp (for example)
        // this switch get the MulExp (for example)
        switch (node->ebnf_type_) {
            case SyEbnfType::MulExp:
                ebnf_type = SyEbnfType::UnaryExp;
                break;
            case SyEbnfType::AddExp:
                ebnf_type = SyEbnfType::MulExp;
                break;
            case SyEbnfType::RelExp:
                ebnf_type = SyEbnfType::AddExp;
                break;
            case SyEbnfType::EqExp:
                ebnf_type = SyEbnfType::RelExp;
                break;
            case SyEbnfType::LAndExp:
                ebnf_type = SyEbnfType::EqExp;
                break;
            case SyEbnfType::LOrExp:
                ebnf_type = SyEbnfType::LAndExp;
                break;
            default:
                // won't reach here
                // trigger a bug
                // just for debugging
                assert(1!=1);
                break;
        }
        node->parent_.lock()->c_ = node->a_;
        node->parent_.lock()->c_->ebnf_type_ = ebnf_type;
        return;
    }
    else {
        auto add_exp_l = node->c_;
        node->b_ = add_exp_l->a_;
        add_exp_l->a_ = add_exp_l->b_;
        add_exp_l->b_ = nullptr;
        add_exp_l->ebnf_type_ = node->ebnf_type_;
        adjustExpLAst(add_exp_l);
    }
}

// take AddExp ast as an example
// origin: AddExp -> MulExp | AddExp ('+' | '−') MulExp 
// ast:        AddExp
//            /  |  \
//       AddExp '+' MulExp
//      /  |  \
// AddExp '+' MulExp

// rewrite: AddExp -> MulExp AddExpL
//          AddExpL -> ('+' | '−') MulExp AddExpL | e
// ast:       AddExp type: AddExp
//           /      \
//        MulExp  AddExpL type: END_OF_ENUM
//               /   |   \
//              '+' MulExp AddExpL type: END_OF_ENUM
//                        /   |   \
//                       '+' MulExp AddExpL type: E 

// adjust:
// ast:      AddExp
//          /  |  \
//     MulExp '+' AddExp(L) type: AddExp
//               /   |   \
//           MulExp '+' MulExp type: END_OF_ENUM => MulExp
//
// after adjust, all AddExpL is a AddExp
// @input: root of the ast
static void adjustExpAst(AstNodePtr node) {
    // before addjust, node->b_ is an AddExpL
    // after addjust, node->b_ is an '+' or '-', 
    // and node->c_ is an AddExp
    if (node->b_ != nullptr) {
        auto add_exp_l = node->b_;
        node->c_ = add_exp_l;
        node->b_ = add_exp_l->a_;
        add_exp_l->a_ = add_exp_l->b_;
        add_exp_l->b_ = nullptr;
        add_exp_l->ebnf_type_ = node->ebnf_type_;
        adjustExpLAst(add_exp_l);
    }
}

AstNodePtr Parser::RelExpL() {
    LexerIterator iter_back = *token_iter_;
    // RelExpL -> ('<' | '>' | '<=' | '>=') AddExp RelExpL | e
    auto rel_exp_l = std::make_shared<AstNode>(SyEbnfType::END_OF_ENUM, (*token_iter_)->line_);
    if ((*token_iter_)->ast_type_ != SyAstType::LNE &&
        (*token_iter_)->ast_type_ != SyAstType::GNE &&
        (*token_iter_)->ast_type_ != SyAstType::LE &&
        (*token_iter_)->ast_type_ != SyAstType::GE) {
        // no need to reset the token_iter_ 
        // this is not a failure
        // just return e
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    rel_exp_l->a_ = *(*token_iter_); // link the '<' or '>' or '<=' or '>='
    (*token_iter_)->parent_ = rel_exp_l;
    ++(*token_iter_);
    auto add_exp = AddExp();
    if (add_exp == nullptr) {
        // this is not a failure
        // reset the token_iter_
        *token_iter_ = iter_back;
        // unlink the '<' or '>' or '<=' or '>='
        rel_exp_l->a_ = nullptr;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    rel_exp_l->b_ = add_exp;
    add_exp->parent_ = rel_exp_l;
    rel_exp_l->c_ = RelExpL();
    rel_exp_l->c_->parent_ = rel_exp_l;
    return rel_exp_l;
}

AstNodePtr Parser::RelExp() {
    // origin: RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
    // rewrite: RelExp -> AddExp RelExpL
    //          RelExpL -> ('<' | '>' | '<=' | '>=') AddExp RelExpL | e
    auto rel_exp = std::make_shared<AstNode>(SyEbnfType::RelExp, (*token_iter_)->line_);
    auto add_exp = AddExp();
    if (add_exp == nullptr) {
        return nullptr;
    }
    // need not to check the return value of RelExpL
    auto rel_exp_l = RelExpL();
    rel_exp->a_ = add_exp;
    add_exp->parent_ = rel_exp;
    if (rel_exp_l->ebnf_type_ != SyEbnfType::E) {
        rel_exp->b_ = rel_exp_l;
        rel_exp_l->parent_ = rel_exp;
        adjustExpAst(rel_exp);
    }
    return rel_exp;
}

AstNodePtr Parser::EqExpL() {
    LexerIterator iter_back = *token_iter_;
    // EqExpL -> ('==' | '!=') RelExp EqExpL | e
    auto eq_exp_l = std::make_shared<AstNode>(SyEbnfType::END_OF_ENUM, (*token_iter_)->line_);
    if ((*token_iter_)->ast_type_ != SyAstType::EQ &&
        (*token_iter_)->ast_type_ != SyAstType::NEQ) {
        // no need to reset the token_iter_ 
        // this is not a failure
        // just return e
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    eq_exp_l->a_ = *(*token_iter_); // link the '==' or '!='
    (*token_iter_)->parent_ = eq_exp_l;
    ++(*token_iter_);
    auto rel_exp = RelExp();
    if (rel_exp == nullptr) {
        // this is not a failure
        // reset the token_iter_
        *token_iter_ = iter_back;
        // unlink the '==' or '!='
        eq_exp_l->a_ = nullptr;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    eq_exp_l->b_ = rel_exp;
    rel_exp->parent_ = eq_exp_l;
    eq_exp_l->c_ = EqExpL();
    eq_exp_l->c_->parent_ = eq_exp_l;
    return eq_exp_l;
}

AstNodePtr Parser::EqExp() {
    // origin: EqExp -> RelExp | EqExp ('==' | '!=') RelExp
    // rewrite: EqExp -> RelExp EqExpL
    //          EqExpL -> ('==' | '!=') RelExp EqExpL | e
    auto eq_exp = std::make_shared<AstNode>(SyEbnfType::EqExp, (*token_iter_)->line_);
    auto rel_exp = RelExp();
    if (rel_exp == nullptr) {
        return nullptr;
    }
    // need not to check the return value of EqExpL
    auto eq_exp_l = EqExpL();
    eq_exp->a_ = rel_exp;
    rel_exp->parent_ = eq_exp;
    if (eq_exp_l->ebnf_type_ != SyEbnfType::E) {
        eq_exp->b_ = eq_exp_l;
        eq_exp_l->parent_ = eq_exp;
        adjustExpAst(eq_exp);
    }
    return eq_exp;
}

// this parse won't return nullptr (it success all the time)
AstNodePtr Parser::LAndExpL() {
    LexerIterator iter_back = *token_iter_;
    // LAndExpL -> '&&' EqExp LAndExpL | e
    auto l_and_exp_l = std::make_shared<AstNode>(SyEbnfType::END_OF_ENUM, (*token_iter_)->line_);
    if ((*token_iter_)->ast_type_ != SyAstType::LOGIC_AND) {
        // no need to reset the token_iter_ 
        // this is not a failure
        // just return e
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    l_and_exp_l->a_ = *(*token_iter_); // link the '&&'
    (*token_iter_)->parent_ = l_and_exp_l;
    ++(*token_iter_);
    auto eq_exp = EqExp();
    if (eq_exp == nullptr) {
        // this is not a failure
        // reset the token_iter_
        *token_iter_ = iter_back;
        // unlink the '&&'
        l_and_exp_l->a_ = nullptr;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    l_and_exp_l->b_ = eq_exp;
    eq_exp->parent_ = l_and_exp_l;
    l_and_exp_l->c_ = LAndExpL();
    l_and_exp_l->c_->parent_ = l_and_exp_l;
    return l_and_exp_l;
}

AstNodePtr Parser::LAndExp() {
    // origin: LAndExp -> EqExp | LAndExp '&&' EqExp
    // rewrite: LAndExp -> EqExp LAndExpL
    //          LAndExpL -> '&&' EqExp LAndExpL | e
    auto l_and_exp = std::make_shared<AstNode>(SyEbnfType::LAndExp, (*token_iter_)->line_);
    auto eq_exp = EqExp();
    if (eq_exp == nullptr) {
        return nullptr;
    }
    // need not to check the return value of LAndExpL
    auto l_and_exp_l = LAndExpL();
    l_and_exp->a_ = eq_exp;
    eq_exp->parent_ = l_and_exp;
    if (l_and_exp_l->ebnf_type_ != SyEbnfType::E) {
        l_and_exp->b_ = l_and_exp_l;
        l_and_exp_l->parent_ = l_and_exp;
        adjustExpAst(l_and_exp);
    }
    return l_and_exp;
}

// this parse won't return nullptr (it success all the time)
AstNodePtr Parser::LOrExpL() {
    LexerIterator iter_back = *token_iter_;
    // LOrExpL -> '||' LAndExp LOrExpL | e
    auto l_or_exp_l = std::make_shared<AstNode>(SyEbnfType::END_OF_ENUM, (*token_iter_)->line_);
    if ((*token_iter_)->ast_type_ != SyAstType::LOGIC_OR) {
        // no need to reset the token_iter_ 
        // this is not a failure
        // just return e
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    l_or_exp_l->a_ = *(*token_iter_); // link the '||'
    (*token_iter_)->parent_ = l_or_exp_l;
    ++(*token_iter_);
    auto l_and_exp = LAndExp();
    if (l_and_exp == nullptr) {
        // this is not a failure
        // just return e
        *token_iter_ = iter_back;
        // unlink the '||'
        l_or_exp_l->a_ = nullptr;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    l_or_exp_l->b_ = l_and_exp;
    l_and_exp->parent_ = l_or_exp_l;
    l_or_exp_l->c_ = LOrExpL();
    l_or_exp_l->c_->parent_ = l_or_exp_l;
    return l_or_exp_l;
}

AstNodePtr Parser::LOrExp() {
    // origin: LOrExp -> LAndExp | LOrExp '||' LAndExp
    // rewrite: LOrExp -> LAndExp LOrExpL
    //          LOrExpL -> '||' LAndExp LOrExpL | e
    auto l_or_exp = std::make_shared<AstNode>(SyEbnfType::LOrExp, (*token_iter_)->line_);
    auto l_and_exp = LAndExp();
    if (l_and_exp == nullptr) {
        return nullptr;
    }
    // need not to check the return value of LOrExpL
    auto l_or_exp_l = LOrExpL();
    l_or_exp->a_ = l_and_exp;
    l_and_exp->parent_ = l_or_exp;
    if (l_or_exp_l->ebnf_type_ != SyEbnfType::E) {
        l_or_exp->b_ = l_or_exp_l;
        l_or_exp_l->parent_ = l_or_exp;
        adjustExpAst(l_or_exp);
    }
    return l_or_exp;
}

AstNodePtr Parser::LVal() {
    // origin: LVal -> Ident {'[' Exp ']'} 
    auto ident = Ident();
    if (ident == nullptr) {
        return nullptr;
    }
    // construct the exp list
    AstNodePtr exp_start = nullptr;
    AstNodePtr exp_last = nullptr;
    while ((*token_iter_)->ast_type_ == SyAstType::LEFT_BRACKET) {
        ++(*token_iter_);
        auto exp = Exp();
        if (exp == nullptr) {
            // this should be an error
            return nullptr;
        }
        if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACKET) {
            // this should be an error
            // break the link list
            exp_start->a_ = nullptr;
            return nullptr;
        }
        ++(*token_iter_);
        // link the list
        if (exp_start == nullptr) {
            exp_start = exp;
            exp_last = exp;
        }
        else {
            exp_last->d_ = exp;
            exp->parent_ = exp_last;
            exp_last = exp;
        }
    }
    auto l_val = std::make_shared<AstNode>(SyEbnfType::LVal, ident->line_);
    l_val->a_ = ident;
    ident->parent_ = l_val;
    if (exp_start != nullptr) {
        l_val->b_ = exp_start;
        exp_start->parent_ = l_val;
    }
    return l_val;
}

AstNodePtr Parser::ConstInitVal() {
    // ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
    auto const_init_val = std::make_shared<AstNode>(SyEbnfType::ConstInitVal, (*token_iter_)->line_);
    if ((*token_iter_)->ast_type_ != SyAstType::LEFT_BRACE) {
        // ConstInitVal -> ConstExp
        auto const_exp = ConstExp();
        if (const_exp == nullptr) {
            return nullptr;
        }
        const_init_val->a_ = const_exp;
        const_exp->parent_ = const_init_val;
        return const_init_val;
    }
    else {
        // ConstInitVal -> '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
        ++(*token_iter_);
        // build up the list
        LexerIterator iter_back = *token_iter_;
        auto const_init_val_start = ConstInitVal();
        auto const_init_val_last = const_init_val_start;
        if (const_init_val_start == nullptr) {
            // it ok to return nullptr if the next token is '}'
            // it means in src, it just uses the "{}" to init
            // so we still need to find the '}'
            // first reset the token_iter_
            *token_iter_ = iter_back;
            if((*token_iter_)->ast_type_ == SyAstType::RIGHT_BRACE) {
                ++(*token_iter_);
                return const_init_val;
            }
            // if we reach here, it means in {...}, ... is invaild
            // we don't care if ... is like .., ...
            // just find the nearest '}' and report an error
            parseError(std::string("expected a const init value after \'\033[1m{\033[0m\'"), (*token_iter_)->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            } 
            ++(*token_iter_);
            return const_init_val;
        }
        else {
            while ((*token_iter_)->ast_type_ == SyAstType::COMMA) {
                ++(*token_iter_);
                iter_back = *token_iter_;
                auto const_init_val_next = ConstInitVal();
                if (const_init_val_next == nullptr) {
                    // here we just find the nearest '}' or ',' and report an error
                    *token_iter_ = iter_back;
                    parseError(std::string("expected a const init value after \'\033[1m,\033[0m\'"), (*token_iter_)->line_);
                    while ((*token_iter_)->ast_type_ != SyAstType::COMMA &&
                           (*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
                        if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                            // there we giveup
                            return nullptr;
                        }
                        ++(*token_iter_);
                    }
                    const_init_val_next = std::make_shared<AstNode>(SyEbnfType::E, 0);
                }
                // link
                const_init_val_last->d_ = const_init_val_next;
                const_init_val_next->parent_ = const_init_val_last;
                const_init_val_last = const_init_val_next;
            }
        }
        // '}'
        if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACE) {
            // here we just ignore the missing '}' and report an error
            parseError(std::string("expected a \'\033[1m}\033[0m\' in initial list"), (*token_iter_)->line_);
            --(*token_iter_);
        }
        ++(*token_iter_);
        if (const_init_val_start != nullptr) {
            const_init_val->a_ = const_init_val_start;
            const_init_val_start->parent_ = const_init_val;
        }
        return const_init_val;
    }
}

AstNodePtr Parser::FuncRParams() {
    // FuncRParams -> Exp { ',' Exp }
    auto func_r_params = std::make_shared<AstNode>(SyEbnfType::FuncRParams, (*token_iter_)->line_);
    auto exp_start = Exp();
    auto exp_last = exp_start;
    if (exp_start == nullptr) {
        return nullptr;
    }
    while ((*token_iter_)->ast_type_ == SyAstType::COMMA) {
        ++(*token_iter_);
        LexerIterator iter_back = *token_iter_;
        auto exp = Exp();
        if (exp == nullptr) {
            // the caller should be confident the next factor is FuncRParams
            // so we just skip the exp and report a error
            parseError(std::string("expected a function parameter after \'\033[1m,\033[0m\'"), (*token_iter_)->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::COMMA) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            exp = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
        exp_last->d_ = exp;
        exp->parent_ = exp_last;
        exp_last = exp;
    }
    func_r_params->a_ = exp_start;
    exp_start->parent_ = func_r_params;
    return func_r_params;
}

AstNodePtr Parser::PrimaryExp() {
    // PrimaryExp -> '(' Exp ')' | LVal | Number
    auto primary_exp = std::make_shared<AstNode>(SyEbnfType::PrimaryExp, (*token_iter_)->line_);
    LexerIterator iter_back = *token_iter_;
    if ((*token_iter_)->ast_type_ != SyAstType::LEFT_PARENTHESE) {
        // try PrimaryExp -> LVal
        // there we don't need to use the backup iter
        auto l_val = LVal();
        if (l_val == nullptr) {
            // try Number
            *token_iter_ = iter_back;
            auto number = Number();
            if (number == nullptr) {
                // this time really failed
                return nullptr;
            }
            primary_exp->a_ = number;
            number->parent_ = primary_exp;
            return primary_exp;
        }
        primary_exp->a_ = l_val;
        l_val->parent_ = primary_exp;
        return primary_exp;
    }
    else {
        // PrimaryExp -> '(' Exp ')'
        ++(*token_iter_);
        auto exp = Exp();
        if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
            // this time really failed
            return nullptr;
        }
        ++(*token_iter_);
        primary_exp->a_ = exp;
        exp->parent_ = primary_exp;
        return exp;
    }
}

AstNodePtr Parser::UnaryExp() {
    // UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
    // first, try UnaryOp UnaryExp
    // second, try Ident '(' [FuncRParams] ')' here we can't handle the error :(
    // third, try PrimaryExp
    auto unary_exp = std::make_shared<AstNode>(SyEbnfType::UnaryExp, (*token_iter_)->line_);
    LexerIterator iter_back = *token_iter_;

    // try UnaryOp UnaryExp
    auto unary_op = UnaryOp();
    if (unary_op != nullptr) {
        auto unary_exp_new = UnaryExp();
        if (unary_exp_new != nullptr) {
            unary_exp->a_ = unary_op;
            unary_op->parent_ = unary_exp;
            unary_exp->b_ = unary_exp_new;
            unary_exp_new->parent_ = unary_exp;
            return unary_exp;
        }
    }
    // try Ident '(' [FuncRParams] ')'
    *token_iter_ = iter_back;
    auto ident = Ident();
    if (ident != nullptr) {
        if ((*token_iter_)->ast_type_ == SyAstType::LEFT_PARENTHESE) {
            ++(*token_iter_);
            LexerIterator iter_back_for_func_r_params = *token_iter_;
            auto func_r_params = FuncRParams();
            if (func_r_params == nullptr) {
                // it's ok the have null func_r_params
                // just reset the token_iter
                *token_iter_ = iter_back_for_func_r_params;
            }
            if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE) {
                // this time really failed
                // the error can't be handled
                // because a missing ')' can be hard to find
                // like this: foo (;
                // if we try to match the ';' then '+' '-' .. all should be handled
                // so we giveup
                return nullptr;
            }
            ++(*token_iter_);
            unary_exp->a_ = ident;
            ident->parent_ = unary_exp;
            if (func_r_params != nullptr) {
                unary_exp->b_ = func_r_params;
                func_r_params->parent_ = unary_exp;
            }
            return unary_exp;
        }
    }
    // try PrimaryExp
    *token_iter_ = iter_back;
    auto primary_exp = PrimaryExp();
    if (primary_exp == nullptr) {
        return nullptr;
    }
    unary_exp->a_ = primary_exp;
    primary_exp->parent_ = unary_exp;
    return unary_exp;
}

AstNodePtr Parser::UnaryOp() {
    // UnaryOp -> '+' | '-' | '!'
    auto token = *(*token_iter_);
    if (token->ast_type_ != SyAstType::ALU_ADD &&
        token->ast_type_ != SyAstType::ALU_SUB &&
        token->ast_type_ != SyAstType::LOGIC_NOT) {
        return nullptr;
    }
    auto unary_op = std::make_shared<AstNode>(SyEbnfType::UnaryOp, (*token_iter_)->line_);
    unary_op->a_ = token;
    token->parent_ = unary_op;
    ++(*token_iter_);
    return unary_op;
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
    (*token_iter_)->parent_ = mul_exp_l;
    ++(*token_iter_);
    auto unary_exp = UnaryExp();
    if (unary_exp == nullptr) {
        // this is no a failure
        // just return e
        *token_iter_ = iter_back;
        // unlink the ('*' | '/' | '%')
        mul_exp_l->a_ = nullptr;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    mul_exp_l->b_ = unary_exp;
    unary_exp->parent_ = mul_exp_l;
    mul_exp_l->c_ = MulExpL();
    mul_exp_l->c_->parent_ = mul_exp_l;
    return mul_exp_l;
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
        adjustExpAst(mul_exp);
    }
    return mul_exp;
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
    (*token_iter_)->parent_ = add_exp_l;
    ++(*token_iter_);
    auto mul_exp = MulExp();
    if (mul_exp == nullptr) {
        // this is not an failure
        // just return e
        *token_iter_ = iter_back;
        return std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    add_exp_l->b_ = mul_exp;
    mul_exp->parent_ = add_exp_l;
    add_exp_l->c_ = AddExpL();
    add_exp_l->c_->parent_ = add_exp_l;
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
        adjustExpAst(add_exp);
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

AstNodePtr Parser::ConstExp() {
    // origin: ConstExp -> AddExp
    auto add_exp = AddExp();
    if (add_exp == nullptr) {
        return nullptr;
    }
    auto const_exp = std::make_shared<AstNode>(SyEbnfType::ConstExp, add_exp->line_);
    const_exp->a_ = add_exp;
    add_exp->parent_ = const_exp;
    return const_exp;
}

AstNodePtr Parser::ConstDef() {
    // origin: ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
    // Ident
    auto const_def = std::make_shared<AstNode>(SyEbnfType::ConstDef, (*token_iter_)->line_);
    auto ident = Ident();
    if (ident == nullptr) {
        return nullptr;
    }
    // following code is same as the VarDef() until the '=' part
    AstNodePtr const_exp_start = nullptr;
    AstNodePtr const_exp_last = nullptr;
    LexerIterator iter_back = *token_iter_;
    while ((*token_iter_)->ast_type_ == SyAstType::LEFT_BRACKET) {
        ++(*token_iter_);
        iter_back = *token_iter_;
        auto const_exp = ConstExp();
        if (const_exp == nullptr) {
            // here we want to get a const_exp, but we get nothing
            // we just jump to the next ']', and report a error
            parseError(std::string("expected a const expression after \'\033[1m[\033[0m\'"), ident->line_);
            *token_iter_ = iter_back;
            while ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACKET) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            const_exp = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
        if ((*token_iter_)->ast_type_ != SyAstType::RIGHT_BRACKET) {
            // just ignore the missing ']' and report a error
            parseError(std::string("expected \'\033[1m]\033[0m\' before \'\033[1m")
             + (*token_iter_)->literal_ + std::string("\033[0m\'"), ident->line_);
             --(*token_iter_);
        }
        ++(*token_iter_);
        if (const_exp_start == nullptr) {
            const_exp_start = const_exp;
            const_exp_last = const_exp;
        }
        else {
            const_exp_last->d_ = const_exp;
            const_exp->parent_ = const_exp_last;
            const_exp_last = const_exp;
        }
    }

    // '='
    if ((*token_iter_)->ast_type_ != SyAstType::ASSIGN) {
        // define const whitout init value
        parseError(std::string("conflicting type qualifiers for \'\033[1m" + ident->literal_ + "\033[0m\'"), (*token_iter_)->line_);
        const_def->a_ = ident;
        ident->parent_ = const_def;
        auto const_init_val = std::make_shared<AstNode>(SyEbnfType::E, 0);
        const_def->c_ = const_init_val;
        const_init_val->parent_ = const_def;
        return const_def;
    }
    ++(*token_iter_);

    // ConstInitVal
    auto const_init_val = ConstInitVal();
    if (const_init_val == nullptr) {
        // ConstDef only end with ';' or ','
        // so here we just find the nearest ';' or ','
        // and report an error
        parseError(std::string("expected a const init value after \'\033[1m=\033[0m\'"), ident->line_);
        while ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON
            && (*token_iter_)->ast_type_ != SyAstType::COMMA) {
            if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                // there we giveup
                return nullptr;
            }
            ++(*token_iter_);
        }
        const_init_val = std::make_shared<AstNode>(SyEbnfType::E, 0);
    }
    const_def->a_ = ident;
    ident->parent_ = const_def;
    if (const_exp_start != nullptr) {
        const_def->b_ = const_exp_start;
        const_exp_start->parent_ = const_def;
    }
    const_def->c_ = const_init_val;
    const_init_val->parent_ = const_def;
    return const_def;
}

AstNodePtr Parser::ConstDecl() {
    // origin: ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
    if ((*token_iter_)->ast_type_ != SyAstType::STM_CONST) {
        return nullptr;
    }
    ++(*token_iter_);
    auto b_type = BType();
    if (b_type == nullptr) {
        return nullptr;
    }
    auto const_def_start = ConstDef();
    auto const_def_last = const_def_start;
    if (const_def_start == nullptr) {
        return nullptr;
    }
    // the following code used the VarDecl()'s code
    // just changed the type and add the const_def's parser
    while ((*token_iter_)->ast_type_ == SyAstType::COMMA) {
        ++(*token_iter_);
        LexerIterator iter_back = *token_iter_;
        auto var_def = ConstDef();
        if (var_def == nullptr) {
            // here we just find the nearest ';' or ','
            // and report an error
            *token_iter_ = iter_back;
            parseError(std::string("expected a init value after \'\033[1m,\033[0m\'"), (*token_iter_)->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON
             && (*token_iter_)->ast_type_ != SyAstType::COMMA) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // there we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            var_def = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
        const_def_last->d_ = var_def;
        var_def->parent_ = const_def_last;
        const_def_last = var_def;
    }
    if ((*token_iter_)->ast_type_ != SyAstType::SEMICOLON) {
        // here we ignore the missing ';' and report an error
        parseError(std::string("expected \'\033[1m;\033[0m\' before \'\033[1m")
            + (*token_iter_)->literal_ + std::string("\033[0m\'"), (*token_iter_)->line_);
        --(*token_iter_);
    }
    ++(*token_iter_);
    auto var_decl = std::make_shared<AstNode>(SyEbnfType::ConstDecl, b_type->line_);
    var_decl->a_ = b_type;
    b_type->parent_ = var_decl;
    var_decl->b_ = const_def_start;
    const_def_start->parent_ = var_decl;
    return var_decl;
}

AstNodePtr Parser::Decl() {
    // origin: Decl -> ConstDecl | VarDecl
    LexerIterator iter_back = *token_iter_;
    auto decl = std::make_shared<AstNode>(SyEbnfType::Decl, (*token_iter_)->line_);
    // ConstDecl
    auto const_decl = ConstDecl();
    if (const_decl != nullptr) {
        decl->a_ = const_decl;
        const_decl->parent_ = decl;
        return decl;
    }
    // VarDecl
    *token_iter_ = iter_back;
    auto var_decl = VarDecl();
    if (var_decl != nullptr) {
        decl->a_ = var_decl;
        var_decl->parent_ = decl;
        return decl;
    }
    return nullptr;
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

AstNodePtr Parser::BType() {
    // origin: BType -> 'int'
    if ((*token_iter_)->ast_type_ != SyAstType::TYPE_INT) {
        return nullptr;
    }
    auto b_type = std::make_shared<AstNode>(SyEbnfType::BType, (*token_iter_)->line_);
    b_type->a_ = **token_iter_;
    (*token_iter_)->parent_ = b_type;
    ++(*token_iter_);
    return b_type;
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
    // origin: FuncFParams -> FuncFParam {',' FuncFParam}
    // FuncFParam
    auto func_f_param_start = FuncFParam();
    auto func_f_param_last = func_f_param_start;
    // {',' FuncFParam}
    while ((*token_iter_)->ast_type_ == SyAstType::COMMA) {
        ++(*token_iter_);
        LexerIterator iter_back = *token_iter_;
        auto func_f_param = FuncFParam();
        if (func_f_param == nullptr) {
            // there, we find the nearest ',' or ')' or '{', report an error and continue
            // as the calling of this function should be really confident
            *token_iter_ = iter_back;
            parseError(std::string("expected a function parameter after \'\033[1m(\033[0m\'"), (*token_iter_)->line_);
            while ((*token_iter_)->ast_type_ != SyAstType::COMMA && 
            (*token_iter_)->ast_type_ != SyAstType::RIGHT_PARENTHESE && 
            (*token_iter_)->ast_type_ != SyAstType::LEFT_BRACE) {
                if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
                    // we giveup
                    return nullptr;
                }
                ++(*token_iter_);
            }
            func_f_param = std::make_shared<AstNode>(SyEbnfType::E, 0);
        }
        // link when suceess
        func_f_param_last->d_ = func_f_param;
        func_f_param->parent_ = func_f_param_last;
        func_f_param_last = func_f_param;
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
        // here we ignore the missing ')'
        // and report an error
        parseError(std::string("expected \'\033[1m)\033[0m\' before \'\033[1m")
            + (*token_iter_)->literal_ + std::string("\033[0m\'"), (*token_iter_)->line_);
        --(*token_iter_);
    }
    ++(*token_iter_);
    auto block = Block();
    if (block == nullptr) {
        // we giveup
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
    auto comp_unit = std::make_shared<AstNode>(SyEbnfType::CompUnit, (*token_iter_)->line_);
    auto decl = Decl();
    if (decl != nullptr) {
        comp_unit->a_ = decl;
        decl->parent_ = comp_unit;
        return comp_unit;
    }
    *token_iter_ = iter_back;
    auto func_def = FuncDef();
    if (func_def != nullptr) {
        comp_unit->a_ = func_def;
        func_def->parent_ = comp_unit;
        return comp_unit;
    }
    parseError("\033[1m\033[35mFATAL ERROR, STOP NOW\033[0m", (*token_iter_)->line_);
    return nullptr;
}

AstNodePtr Parser::Ident() {
    if ((*token_iter_)->ast_type_ != SyAstType::IDENT) {
        return nullptr;
    }
    auto ident = *(*token_iter_);
    ++(*token_iter_);
    return ident;
}

AstNodePtr Parser::parse() {
    if ((*token_iter_)->ast_type_ == SyAstType::EOF_TYPE) {
        end_parse_ = 1;
        return nullptr;
    }
    auto node = CompUnit();
    if (node == nullptr) {
        parseError("\033[1m\033[35mFATAL ERROR, STOP NOW\033[0m", (*token_iter_)->line_);
        exit(-1);
    }
    return node;
}
