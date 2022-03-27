# 解释器（语法分析器）

### 主要功能

通过遍历 ast 进行解释执行，同时运行期进行有限的语义分析。

### 构建方法

```
make syinterpreter # 构建解释器
```

### 亮点

一般的解释器都是解释执行字节码，这里偷了懒，直接 ast 上跑了。所以应该会特别慢。

这个解释器有一定的拓展性，如果需要加一些内建函数，可以在参考 `syruntime.cc` 中的代码进行添加。简单的来说，就是提供一个 'jited' 的函数。首先实现函数体，然后通过实现 `execCallBack` 来处理传参。通过 SYFunction 提供的构造函数即可生成一个 'jited' 函数。

```
SYFunction(void* func, bool no_fail, Value (*exec_call_back)(char*, AstNodePtr, InterpreterAPI*))
```