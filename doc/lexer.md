# 词法分析器

### 主要功能

实现了对 SysY 语言的词法分析。并且具备错误检出和恢复的能力。

### 构建方法

项目提供了 Makefile。

```
make sylexer # 构建 lexer
make test-lexer # 使用 lexer 执行所有的 testcases （在 /test/testcase 目录下的 *.sy）
make clean # 删除构建出的文件
```

### 程序亮点

没有什么亮点，就是一个很普通的词法分析器。