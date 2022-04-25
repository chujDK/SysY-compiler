# SysY-compiler

Just a toy compiler for me to avoid the regret of never implemented any compiler from scratch in the future.

Currently my intention is to implement a simple jit compiler (of course this interpreter which is actually a semantic analyzer may be deprecated in the future), using llvm to generate machine code. The completely handwritten part is actually just an llvm front end. I plan to write some llvm optimize pass in the future just for fun

### TODO

#### milestone

- [x] lexer
- [x] parser
- [x] interpreter
- [ ] jit compiler
- [ ] opt passes

#### refactor

~~将所有的代码生成变成使用各类重写的 irGen 方法~~ 不知道该怎么设计555
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
