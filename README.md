# SysY-compiler
### TODO
- [x] lexer
- [x] parser
- [ ] interpreter
- [ ] jit compiler

### Usage

- type `make sylexer` to make the lexer
- type `make syparser` to make the parser
- type `make test-lexer` to feed all testcases in `/test/testcase/*.sy` to sylexer
- type `make test-parser` to feed all testcases in `/test/testcase/*.sy` to syparser
- type `make clean` to delete all binary
- type `make all` to make both the lexer and parser

### Note

both the output of lexer and parser used the "unix-liked terminal color setting", so it may not output nicely in windows