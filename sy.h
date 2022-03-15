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

// this class is mainly for test
class CharStream : public InputStream {
private:
    char* buf;
    int buf_size;
    int buf_pos;
public:
    // this class is made to read the input "file"
    char getChar(); // get the current char
    char peakChar(); // get the current char
    char peakNextChar(); // get the next char
    void ungetChar(); // unget the current char
    std::string getLine(); // get the current line

    CharStream(char* buf, int buf_size): buf(buf), buf_size(buf_size), buf_pos(0) {}
    ~CharStream() {}
};

class FileStream : public InputStream {
private:
    FILE* fp;
public:
    // this class is made to read the input "file"
    char getChar(); // get the current char
    char peakChar(); // get the current char
    char peakNextChar(); // get the next char
    void ungetChar(); // unget the current char
    std::string getLine(); // get the current line

    FileStream(const char* file_name): fp(fopen(file_name, "r")) {}
    ~FileStream() {
        fclose(fp);
    }
};

class SyCTX {
private:
public:

};

#endif