# SysY-compiler
### TODO

#### milestone

- [x] lexer
- [x] parser
- [x] interpreter
- [ ] jit compiler

#### refactor

- 当前的文法和后续的语义分析（实现为了解释器）和 ir-gen 都紧耦合了，考虑到对 ast 的访问完全满足“访问者模式”解决的“稳定的数据结构和易变的操作耦合问题”、“需要对一个对象结构中的对象进行很多不同的并且不相关的操作”的问题，应该使用访问者模式重构
### Pre-requirement

clang-12+llvm is needed as the jit compiler will use llvm ir. In ubuntu20.04, just use apt to install them.

```
sudo apt install clang
sudo apt install llvm
```

### Usage

- type `make sylexer` to make the lexer
- type `make syparser` to make the parser
- type `make syinterpreter` to make the interpreter
- type `make test-lexer` to feed all testcases in `/test/testcase/*.sy` to sylexer
- type `make test-parser` to feed all testcases in `/test/testcase/*.sy` to syparser
- type `make clean` to delete all binary
- type `make all` to make both the lexer and parser

### Note

both the output of lexer and parser used the "unix-liked terminal color setting", so it may not output nicely in windows (windows terminal can display the color correctly)