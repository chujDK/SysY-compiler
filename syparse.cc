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
    return token->next_token_;
}

TokenPtr Lexer::getPrevToken(TokenPtr token) {
    return token->prev_token_;
}

AstNodePtr Parser::parse() {

}