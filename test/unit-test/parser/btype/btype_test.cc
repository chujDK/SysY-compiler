#ifdef UNIT_TEST
#include <iostream>

#include "sy.h"
#include "sydebug.h"

class ParserTest : public Parser {
   public:
	bool doTest() {
		while (1) {
			auto root = this->parse();
			astWalkThrough(root, 0);
			if (root == nullptr) {
				break;
			}
			std::cout << std::endl;
		}
	}
	ParserTest(InputStream* input) : Parser(input) {}
};

int main() {
	auto input        = new CharStream("int", 3);
	auto pareser_test = new ParserTest(input);
	pareser_test->doTest();
	return 0;
}
#endif
