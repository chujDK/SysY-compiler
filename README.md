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

使用访问者模式重构代码生成部分。同时抛弃解释器，实现一个独立的语法分析器。

### Pre-requirement

clang-12+llvm is needed as the jit compiler will use llvm ir. In ubuntu20.04, just use apt to install them.

```
sudo apt install clang
sudo apt install llvm
```

### Usage

this project use CMake for building

use following command to build

```
mkdir build && cd ./build
cmake .. -DLLVM_DIR=$(llvm-config --cmakedir) -DCMAKE_CXX_COMPILER="clang++"
make all -j$(nproc) # this will make all the tests
```

### Note

both the output of lexer and parser used the "unix-liked terminal color setting", so it may not output nicely in windows (windows terminal can display the color correctly)
