3-91
# ff0-script
一个运行自制脚本语言的解释器
当前版本：3.91

```
（如果有JavaScript编程经验的程序员可能会更好上手Warfarin语言）

运行方式：

（1.6版本的Warfarin是交互解释器、编译器和虚拟机分开的三个exe，1.7之后正式三合一） 

直接打开Warfarin可启动交互解释器，按Ctrl+Z停止输入并观察程序输出。

也可使用命令行完成操作，格式：
Warfarin <文件名> <参数列表>

参数列表：
-c 是否编译出.ff0文件 
-r 是否运行生成的.ff0文件 
-j 是否反编译成 JavaScript
-p 是否启动伪代码分析器 FF0Parser

例：
Warfarin test.wfr -cr 编译并运行test.wfr文件
Warfarin test.wfr -crj 把test.wfr文件编译并运行，同时反编译成JavaScript
Warfarin test.wfr -crjp 把test.wfr文件编译运行反编译，允许代码中含有伪代码

如果使用交互解释器，将会在当前目录下生成一个a.ff0文件。 

可以使用import来包含代码包，格式如下： 
import 完整文件名 
相当于C++中的 
#include <"完整文件名">

1.变量不需要声明，但需要定义，例如：
    a = 1
    b = a + 1
    c = a + b
任何一个变量在没有初始化时均为undefined类型。
2.Warfarin支持四种类型的数据：
   数字（IEEE浮点数，即C++中的double） a = 42.24
   字符串                              b = "Surtr"
   列表                                   c = {"1", 2, {"another list"}}
   函数                               //见下

    支持位运算（按位与、按位或）
    如果操作数是小数会自动取整再进行运算
    & 按位与 | 按位或
    由于符号用完了，异或采用函数形式xor
    a = 1
    b = 2
    print(a & b)
    print(xor(a, b))

    现在支持16进制整数
    print(0xff0)

    使用readonly函数可以设置常量：
    a = 2
    readonly("a")
    a = 3   会报错
    注意：使用了readonly的列表仍然能够修改列表元素

3.控制流语句，支持if、for、while三种语句。
    if和while语句的用法与C++、JavaScript基本相同。 
  请注意：if 后跟的是elseif ，最后才是else
    while(a[i] < a[j]) {
        i = i + 1
        j = j - 1
    }

    if(a == 1) {
        print("One")
    }
    elseif(a == 2) {
        print("Two")
    }
    else{
        print("Other")
    }

    for语句格式：for(<variable>, <expression>, <expression>, <expression>) <statement>
    for(i, 1, i <= 10, 1) {

    }
    相当于C++中的for(int i = 1; i <= 10; i = i + 1)

4.函数定义与使用
    function <函数名>(可选的参数列表) {
        ...
        return 值 
    }

    如果要检查参数类型，使用：
    function <函数名>(参数名1:类型名, 参数名2:类型名...) {
        ...
        return 值 
    }
    类型名是可选的，如果不限制参数的类型可以直接省去这一部分，比方说：
    function test(a:Number, b:String, c) {

    }
    a必须是数字，b必须是字符串，c不限。 

    你可以直接把函数赋给一个变量：
    another = abs
    another(10)

    等价于abs(10)。

5.列表
    列表允许你使用字符串或数字作为下标：
    a = {}
    a[1] = 123456
    a["test"] = abs
    a["test"](5)

    为了程序更简洁明了，Warfarin允许使用a.test这样的“点语法”进行访问。

6.一些常用函数（可在源代码的init函数中找到1.7版本所有的内置函数）
    print("Hello")  打印（你可以print(a, b, c)）
    exit(1)         强制退出虚拟机
    len({1, 2, 3})  得到列表从0开始的长度（undefined不算在内）
    system("pause") 运行系统命令
    clock()         得到当前时间
    read_number()   从控制台读入一个数
    read_string()   从控制台读入一个字符串（不允许有空格）
    read_str_line() 从控制台读入一行字符串（允许有空格）
    random()        得到一个随机数，范围在[0, 32768]
    warfarin()      得到当前Warfarin虚拟机的版本信息
    alert("ERROR")  弹出一个对话框并返回用户的输入 *在信竞版本不可用
    xor(2, 3)       计算2与3的异或
     readonly("a")   把a变成只读的

7.FAQ
    (I) type Undefined doesnt support operate '+'
    这种错误一般是发生在数组中的。
    a = {}
    a[1] = a[0] + 1
    运行以上程序就会发生这种错误。你必须给所有元素初始化后才能使用它们。

    (II) variable ';' is not declared
    不要在语句后面加分号。Warfarin强制要求不加。 
```

## 伪代码转 Warfarin 语言 FF0Parser 使用方式：

```
使用方式：直接运行程序，输入源代码，使用 Ctrl+Z 停止输入后即可获得输出。
cmd 格式：ff0parser < 输入文件名 > 输出文件名 
1. 运算符格式
常用运算符对照如下：
+ add			<	is bigger than			&&	and
- substract		>	is smaller than			||	or
* multiply		<=	is not-bigger than 		!=	not-equal
/ divide 		>= 	is not-smaller than		==	equal
赋值有两种写法。<- 或 is assigned with 
如 a = b 可写成 a <- b 或 a is assigned with b。
函数调用、下标可按编程语言格式，如 func(1, 2) 和 test[5].propotype。 
2. if-elseif-else 语句
格式如下： 
If <表达式> is true, do:
	<代码>
Done;
Or <表达式> is true, do:
	<代码>
Done; 
(可重复多次 Or)
Otherwise, do:
	<代码> 
Done;
等同于 Warfarin 中的
if(...) {
	...
} 
elseif(...) {
	...
}
else {
	...
}
2. Repeat 语句
格式：
Repeat until <表达式> is false, do:
	<代码>
Done;
等同于
while(...) {
	...
}
3. For 语句
For 语句的支持有限。格式：
For <变量> from <起始值> -> <终值>, do:
	<代码>
Done;
等同于：
for(<变量>, <起始值>, <变量> != <终值>, 1) {
	...
}
4. 函数定义
基本格式：
Function, called <名字>, needs <数量> parameters, [called <参数名1>, [must be <类型名>,] <参数名2>, [must be <类型名>,] ...,] do:
	<代码>
Done;
这里有一个可选的 must be <类型名>，比如
Function called test, needs 1 parameter, called str, must be String, do:
要求参数str必须是字符串类型。
如果不写则可以是任意类型。
注意：
如果参数数量是 0 可以写成 no；
遵循英文复数语法，当数量为 1 时 parameter 不加 s，为 0 或为其他不为 1 的数时要加 s，例：
Function, called Z, needs 0 parameters, do: 
Function, called A, needs 1 parameter, called p1, do:
Function, called B, needs 2 parameters, called p1, p2, do: 
...
5. 其他
一行里面可以写多条语句，但是必须用分号 ; 分隔。
转化成的代码是 Warfarin 格式。如果要转成 JavaScript 可以使用 Warfarin 虚拟机自带的反编译功能反编译成 JavaScript 代码。
All rights are reserved. 
```
