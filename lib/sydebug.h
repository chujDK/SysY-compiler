#ifndef _SYDEBUG_H_
#define _SYDEBUG_H_
#include "sy.h"

extern const char* SyAstTypeDebugInfo[(int) SyAstType::END_OF_ENUM];
extern const char* SyEbnfTypeDebugInfo[(int) SyEbnfType::END_OF_ENUM];

// this class is mainly for test
class CharStream : public InputStream {
private:
    const char* buf;
    int buf_size;
    int buf_pos;
public:
    // this class is made to read the input "file"
    char getChar(); // get the current char
    char peakChar(); // get the current char
    char peakNextChar(); // get the next char
    void ungetChar(); // unget the current char
    std::string getLine(); // get the current line

    CharStream(const char* buf, int buf_size): buf(buf), buf_size(buf_size), buf_pos(0) {}
    ~CharStream() {}
};

// this class is very plain, just read all the file into a buffer
// then delegate the work to CharStream
class FileStream : public InputStream {
private:
    CharStream* char_stream_;
    char* buf_;
public:
    // this class is made to read the input "file"
    char getChar(); // get the current char
    char peakChar(); // get the current char
    char peakNextChar(); // get the next char
    void ungetChar(); // unget the current char
    std::string getLine(); // get the current line

    FileStream(const char* file_name);
    ~FileStream();
};

void debugInfoInit();
void printSyToken(TokenPtr token);

#endif
