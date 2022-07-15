# TinyJSON

此项目参考 [json-tutorial 项目][1] 使用C++实现简单的JSON解析器

- [ ] 完成初始C语言风格版本
- [ ] 现代化C++改进项目

运行环境：
cmake 工具
gtest 框架

```
mkdir build
cd build
cmake ..
make
./test
```


JSON（JavaScript Object Notation）是一个用于数据交换的文本格式，现时的标准为[ECMA-404][2]。

JSON包含6总基本数据类型
- boolean
- null
- number
- array
- object
- string 

那么每种类型开头都是有规律
```
n ➔ null
t ➔ true
f ➔ false
" ➔ string
0-9/- ➔ number
[ ➔ array
{ ➔ object
```

数字解析过程

![number][3]

UTF-8编码解析

数组解析
```
array = %x5B ws [ value *( ws %x2C ws value ) ] ws %x5D
例如：
[[1,2,3], "abe", true, object]
```

中间值的类型是JSON基本数据类型，不接受末尾的额外`","`，如 `[1,2,]`

这里使用数组存储，达到使用下标访问数据时间复杂度O(1)的目的。

解析过程：

数组分配一个指针，指向一个指针数组，空间不够时relloc开辟新空间，然后复制数据到新的空间。

[1]: https://github.com/miloyip/json-tutorial
[2]: http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf
[3]: ./resource/picture/JSON%20%E6%A0%87%E5%87%86%20ECMA-404%20number.png
