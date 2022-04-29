#include <cstdlib>
#include <cstring>
#include <iostream>

#include "sy.h"
#include "sydebug.h"

int main(int argc, char* argv[]) {
    debugInfoInit();
    FileStream* fileStream = new FileStream(argv[1]);
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
    return 0;
}