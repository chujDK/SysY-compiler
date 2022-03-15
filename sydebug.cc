#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include "sydebug.h"

const char* SyAstTypeDebugInfo[(int) SyAstType::END_OF_ENUM];

FileStream::FileStream(const char* file_name)
{
    std::ifstream in(file_name);
    std::ostringstream tmp;
    tmp << in.rdbuf();
    buf_ = strdup(tmp.str().c_str());
    char_stream_ = new CharStream(buf_, strlen(buf_));
}

char FileStream::getChar()
{
    return char_stream_->getChar();
}

char FileStream::peakChar()
{
    return char_stream_->peakChar();
}

char FileStream::peakNextChar()
{
    return char_stream_->peakNextChar();
}

void FileStream::ungetChar()
{
    return char_stream_->ungetChar();
}

std::string FileStream::getLine()
{
    return char_stream_->getLine();
}

FileStream::~FileStream()
{
    delete[] buf_;
    delete char_stream_;
}

char CharStream::getChar() {
    if (buf_pos >= buf_size) {
        return EOF;
    }
    return buf[buf_pos++];
}

char CharStream::peakChar() {
    if (buf_pos >= buf_size) {
        return EOF;
    }
    return buf[buf_pos];
}

char CharStream::peakNextChar() {
    if (buf_pos + 1 >= buf_size) {
        return EOF;
    }
    return buf[buf_pos + 1];
}

void CharStream::ungetChar() {
    if (buf_pos > 0) {
        --buf_pos;
    }
}

std::string CharStream::getLine() {
    std::string line;
    while (true) {
        char c = getChar();
        if (c == '\n') {
            break;
        }
        line += c;
    }
    return line;
}

void SyAstTypeDebugInfoInit() {
    SyAstTypeDebugInfo[(int) SyAstType::LEFT_PARENTHESE] = "LEFT_PARENTHESE";
    SyAstTypeDebugInfo[(int) SyAstType::RIGHT_PARENTHESE] = "RIGHT_PARENTHESE";
    SyAstTypeDebugInfo[(int) SyAstType::LEFT_BRACKET] = "LEFT_BRACKET";
    SyAstTypeDebugInfo[(int) SyAstType::RIGHT_BRACKET] = "RIGHT_BRACKET";
    SyAstTypeDebugInfo[(int) SyAstType::LEFT_BRACE] = "LEFT_BRACE";
    SyAstTypeDebugInfo[(int) SyAstType::RIGHT_BRACE] = "RIGHT_BRACE";
    SyAstTypeDebugInfo[(int) SyAstType::ALU_ADD] = "ALU_ADD";
    SyAstTypeDebugInfo[(int) SyAstType::ALU_SUB] = "ALU_SUB";
    SyAstTypeDebugInfo[(int) SyAstType::ALU_DIV] = "ALU_DIV";
    SyAstTypeDebugInfo[(int) SyAstType::ALU_MUL] = "ALU_MUL";
    SyAstTypeDebugInfo[(int) SyAstType::ALU_MOD] = "ALU_MOD";
    SyAstTypeDebugInfo[(int) SyAstType::ASSIGN] = "ASSIGN";
    SyAstTypeDebugInfo[(int) SyAstType::COMMA] = "COMMA";
    SyAstTypeDebugInfo[(int) SyAstType::DOT] = "DOT";
    SyAstTypeDebugInfo[(int) SyAstType::EQ] = "EQ";
    SyAstTypeDebugInfo[(int) SyAstType::NEQ] = "NEQ";
    SyAstTypeDebugInfo[(int) SyAstType::LNE] = "LNE";
    SyAstTypeDebugInfo[(int) SyAstType::LE] = "LE";
    SyAstTypeDebugInfo[(int) SyAstType::GNE] = "GNE";
    SyAstTypeDebugInfo[(int) SyAstType::GE] = "GE";
    SyAstTypeDebugInfo[(int) SyAstType::LOGIC_NOT] = "LOGIC_NOT";
    SyAstTypeDebugInfo[(int) SyAstType::LOGIC_AND] = "LOGIC_AND";
    SyAstTypeDebugInfo[(int) SyAstType::LOGIC_OR] = "LOGIC_OR";
    SyAstTypeDebugInfo[(int) SyAstType::SEMICOLON] = "SEMICOLON";
    SyAstTypeDebugInfo[(int) SyAstType::QUOTE] = "QUOTE";
    SyAstTypeDebugInfo[(int) SyAstType::TYPE_INT] = "TYPE_INT";
    SyAstTypeDebugInfo[(int) SyAstType::TYPE_VOID] = "TYPE_VOID";
    SyAstTypeDebugInfo[(int) SyAstType::STM_IF] = "STM_IF";
    SyAstTypeDebugInfo[(int) SyAstType::STM_ELSE] = "STM_ELSE";
    SyAstTypeDebugInfo[(int) SyAstType::STM_WHILE] = "STM_WHILE";
    SyAstTypeDebugInfo[(int) SyAstType::STM_BREAK] = "STM_BREAK";
    SyAstTypeDebugInfo[(int) SyAstType::STM_CONTINUE] = "STM_CONTINUE";
    SyAstTypeDebugInfo[(int) SyAstType::STM_RETURN] = "STM_RETURN";
    SyAstTypeDebugInfo[(int) SyAstType::INT_IMM] = "INT_IMM";
    SyAstTypeDebugInfo[(int) SyAstType::STRING] = "STRING";
    SyAstTypeDebugInfo[(int) SyAstType::IDENT] = "IDENT";
    SyAstTypeDebugInfo[(int) SyAstType::EOF_TYPE] = "EOF_TYPE";
}

void debugInfoInit() {
    SyAstTypeDebugInfoInit();
}