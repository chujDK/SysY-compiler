#include <cstring>
#include <iostream>
#include <string>
#include "syparse.h"
#include "sydebug.h"

CharStream* charStream;

int main()
{
    std::string prog ("int a, b, c, d, e, f, g;\n\xFF");
    charStream = new CharStream(prog.c_str(), prog.size());
    Parser parser1(charStream);
    auto decl = parser1.parse();
    for (auto i : *(decl->a_->a_->b_)) {
        std::cout << i->a_->literal_ << " ";
    }
    std::cout << std::endl;

    prog = "const int a_c = 1, b_c = 2, c_c = 3, d_c = 4, e_c = 5, f_c = 6, g_c = 7;\n\xFF";
    charStream = new CharStream(prog.c_str(), prog.size());
    Parser parser2(charStream);
    decl = parser2.parse();
    for (auto i : *(decl->a_->a_->b_)) {
        std::cout << i->a_->literal_ << " ";
    }
    std::cout << std::endl;
}
