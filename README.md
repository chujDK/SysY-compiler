# SysY-compiler
### TODO
- [x] lexer
- [x] parser
- [x] interpreter
- [ ] jit compiler

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

both the output of lexer and parser used the "unix-liked terminal color setting", so it may not output nicely in windows

### 不足

- 设计之初没有想清楚，其实每种 node 都应该继承一个接口类，然后在 parse 后的操作中直接通过相应的重载方法来生成对应的 ir 等。现在的实现造成了 ir 生成器、解释器和语法紧耦合了。