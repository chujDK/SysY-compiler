#ifndef _SY_H_
#define _SY_H_
#include <string>
#include "syparse.h"

class InputStream {
public:
    // this class is made to read the input "file"
    virtual char getChar() = 0; // get the current char
    virtual char peakChar() = 0; // get the current char
    virtual char peakNextChar() = 0; // get the next char
    virtual void ungetChar() = 0; // unget the current char
    virtual std::string getLine() = 0; // get the current line
};

class SyCTX {
private:
public:

};

#endif
