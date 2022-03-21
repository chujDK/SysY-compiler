# 语法分析器

### 主要功能
使用递归下降法进行语法分析。具备有限的错误检出和恢复能力

### 构建方法

项目提供了 Makefile。

```
make syparser # 构建 lexer
make test-parser # 使用 lexer 执行所有的 testcases （在 /test/testcase 目录下的 *.sy）
make clean # 删除构建出的文件
```

### 程序亮点

没有什么亮点，就是一个很弱鸡的语法分析器。