#include <cstdlib>
#include <cstring>
#include <iostream>

#include "sy.h"
#include "sydebug.h"

#ifndef UNIT_TEST
int main(int argc, char* argv[]) {
	debugInfoInit();
	FileStream* fileStream = new FileStream(argv[1]);
#ifdef LEXER
	// this is the code for lexer
	Lexer* lexer       = new Lexer(fileStream);
	LexerIterator iter = lexer->begin();
	while (iter->getAstType() != SyAstType::EOF_TYPE) {
		printSyToken(*iter);
		++iter;
	}
#endif
#ifdef PARSER
	// this is the code for parser
	Parser parser(fileStream);
	while (1) {
		auto root = parser.parse();
		astWalkThrough(root, 0);
		if (root == nullptr) {
			break;
		}
		std::cout << std::endl;
	}
#endif
#ifdef INTERPRETER
	// this is the code for interpreter
	ParserAPI* parser           = (ParserAPI*)new Parser(fileStream);
	InterpreterAPI* interpreter = new Interpreter(parser);
	syRuntimeInitForAnInterpreter(interpreter);
	interpreter->exec();
#endif

#ifdef COMPILER
	// let's do a sample test for ir gen
	auto fileStreamIR           = new FileStream("test/testcase/test_ir1.sy");
	ParserAPI* parser           = (ParserAPI*)new Parser(fileStreamIR);
	InterpreterAPI* interpreter = new Interpreter(parser);
	// no runtime init here

	codeGenInit();
	interpreter->exec();
#endif

	return 0;
}
#endif
