#include "sydebug.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

const char* SyAstTypeDebugInfo[(int)SyAstType::END_OF_ENUM];
const char* SyEbnfTypeDebugInfo[(int)SyEbnfType::END_OF_ENUM];

FileStream::FileStream(const char* file_name) {
	std::ifstream in(file_name);
	std::ostringstream tmp;
	tmp << in.rdbuf();
	buf_         = strdup(tmp.str().c_str());
	char_stream_ = new CharStream(buf_, strlen(buf_));
}

char FileStream::getChar() { return char_stream_->getChar(); }

char FileStream::peakChar() { return char_stream_->peakChar(); }

char FileStream::peakNextChar() { return char_stream_->peakNextChar(); }

void FileStream::ungetChar() { return char_stream_->ungetChar(); }

std::string FileStream::getLine() { return char_stream_->getLine(); }

FileStream::~FileStream() {
	free(buf_);
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

static void SyAstTypeDebugInfoInit() {
	SyAstTypeDebugInfo[(int)SyAstType::LEFT_PARENTHESE]  = "LEFT_PARENTHESE";
	SyAstTypeDebugInfo[(int)SyAstType::RIGHT_PARENTHESE] = "RIGHT_PARENTHESE";
	SyAstTypeDebugInfo[(int)SyAstType::LEFT_BRACKET]     = "LEFT_BRACKET";
	SyAstTypeDebugInfo[(int)SyAstType::RIGHT_BRACKET]    = "RIGHT_BRACKET";
	SyAstTypeDebugInfo[(int)SyAstType::LEFT_BRACE]       = "LEFT_BRACE";
	SyAstTypeDebugInfo[(int)SyAstType::RIGHT_BRACE]      = "RIGHT_BRACE";
	SyAstTypeDebugInfo[(int)SyAstType::ALU_ADD]          = "ALU_ADD";
	SyAstTypeDebugInfo[(int)SyAstType::ALU_SUB]          = "ALU_SUB";
	SyAstTypeDebugInfo[(int)SyAstType::ALU_DIV]          = "ALU_DIV";
	SyAstTypeDebugInfo[(int)SyAstType::ALU_MUL]          = "ALU_MUL";
	SyAstTypeDebugInfo[(int)SyAstType::ALU_MOD]          = "ALU_MOD";
	SyAstTypeDebugInfo[(int)SyAstType::ASSIGN]           = "ASSIGN";
	SyAstTypeDebugInfo[(int)SyAstType::COMMA]            = "COMMA";
	SyAstTypeDebugInfo[(int)SyAstType::DOT]              = "DOT";
	SyAstTypeDebugInfo[(int)SyAstType::EQ]               = "EQ";
	SyAstTypeDebugInfo[(int)SyAstType::NEQ]              = "NEQ";
	SyAstTypeDebugInfo[(int)SyAstType::LNE]              = "LNE";
	SyAstTypeDebugInfo[(int)SyAstType::LE]               = "LE";
	SyAstTypeDebugInfo[(int)SyAstType::GNE]              = "GNE";
	SyAstTypeDebugInfo[(int)SyAstType::GE]               = "GE";
	SyAstTypeDebugInfo[(int)SyAstType::LOGIC_NOT]        = "LOGIC_NOT";
	SyAstTypeDebugInfo[(int)SyAstType::LOGIC_AND]        = "LOGIC_AND";
	SyAstTypeDebugInfo[(int)SyAstType::LOGIC_OR]         = "LOGIC_OR";
	SyAstTypeDebugInfo[(int)SyAstType::SEMICOLON]        = "SEMICOLON";
	SyAstTypeDebugInfo[(int)SyAstType::QUOTE]            = "QUOTE";
	SyAstTypeDebugInfo[(int)SyAstType::TYPE_INT]         = "TYPE_INT";
	SyAstTypeDebugInfo[(int)SyAstType::TYPE_VOID]        = "TYPE_VOID";
	SyAstTypeDebugInfo[(int)SyAstType::STM_IF]           = "STM_IF";
	SyAstTypeDebugInfo[(int)SyAstType::STM_ELSE]         = "STM_ELSE";
	SyAstTypeDebugInfo[(int)SyAstType::STM_WHILE]        = "STM_WHILE";
	SyAstTypeDebugInfo[(int)SyAstType::STM_BREAK]        = "STM_BREAK";
	SyAstTypeDebugInfo[(int)SyAstType::STM_CONTINUE]     = "STM_CONTINUE";
	SyAstTypeDebugInfo[(int)SyAstType::STM_CONST]        = "STM_CONST";
	SyAstTypeDebugInfo[(int)SyAstType::STM_RETURN]       = "STM_RETURN";
	SyAstTypeDebugInfo[(int)SyAstType::INT_IMM]          = "INT_IMM";
	SyAstTypeDebugInfo[(int)SyAstType::STRING]           = "STRING";
	SyAstTypeDebugInfo[(int)SyAstType::IDENT]            = "IDENT";
	SyAstTypeDebugInfo[(int)SyAstType::EOF_TYPE]         = "EOF_TYPE";
}

static void SyEbnfTypeDebugInfoInit() {
	SyEbnfTypeDebugInfo[(int)SyEbnfType::CompUnit]     = "CompUnit";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::Decl]         = "Decl";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::ConstDecl]    = "ConstDecl";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::BType]        = "BType";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::ConstDef]     = "ConstDef";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::ConstInitVal] = "ConstInitVal";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::VarDecl]      = "VarDecl";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::VarDef]       = "VarDef";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::InitVal]      = "InitVal";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::FuncDef]      = "FuncDef";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::FuncType]     = "FuncType";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::FuncFParams]  = "FuncFParams";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::FuncFParam]   = "FuncFParam";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::Block]        = "Block";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::BlockItem]    = "BlockItem";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::Stmt]         = "Stmt";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::Exp]          = "Exp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::Cond]         = "Cond";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::LVal]         = "LVal";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::PrimaryExp]   = "PrimaryExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::Number]       = "Number";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::UnaryExp]     = "UnaryExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::UnaryOp]      = "UnaryOp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::FuncRParams]  = "FuncRParams";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::MulExp]       = "MulExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::AddExp]       = "AddExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::RelExp]       = "RelExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::EqExp]        = "EqExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::LAndExp]      = "LAndExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::LOrExp]       = "LOrExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::ConstExp]     = "ConstExp";
	SyEbnfTypeDebugInfo[(int)SyEbnfType::E]            = "E";
}

void debugInfoInit() {
	SyAstTypeDebugInfoInit();
	SyEbnfTypeDebugInfoInit();
}

void printSyToken(TokenPtr token) {
	std::cout << "\033[1m\033[34m"
	          << SyAstTypeDebugInfo[(int)token->getAstType()] << "\033[0m"
	          << " \""
	             "\033[32m"
	          << token->getLiteral() << "\033[0m"
	          << "\"" << std::endl;
}

void astWalkThrough(AstNodePtr node, int level) {
	if (node == nullptr) {
		return;
	}
	for (int i = 0; i < level; i++) {
		std::cout << ". ";
	}
	if (node->getLiteral().empty()) {
		if (node->getEbnfType() != SyEbnfType::END_OF_ENUM) {
			std::cout << "\033[1m\033[33m"
			          << SyEbnfTypeDebugInfo[(int)node->getEbnfType()]
			          << "\033[0m";
		} else {
			std::cout << "\033[1m\033[33m"
			          << SyAstTypeDebugInfo[(int)node->getAstType()]
			          << "\033[0m";
		}
	} else {
		if (node->getEbnfType() != SyEbnfType::END_OF_ENUM) {
			std::cout << "\033[1m\033[34m"
			          << SyEbnfTypeDebugInfo[(int)node->getEbnfType()]
			          << "\033[0m \"\033[32m" << node->getLiteral()
			          << "\033[0m\"";
		} else {
			std::cout << "\033[1m\033[34m"
			          << SyAstTypeDebugInfo[(int)node->getAstType()]
			          << "\033[0m \"\033[32m" << node->getLiteral()
			          << "\033[0m\"";
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
