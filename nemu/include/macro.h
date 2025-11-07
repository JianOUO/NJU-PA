/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __MACRO_H__
#define __MACRO_H__

#include <string.h>

// macro stringizing
/*
#x 的作用：
将宏参数 x 转换为字符串字面量
在参数两边自动添加双引号 "
str_temp(hello) -> "hello"

为什么需要两层宏？

问题：一层宏的局限性
// 只有一层（错误做法）
#define str(x) #x
#define PLATFORM riscv32
str(PLATFORM)      → "PLATFORM" C 预处理器不会展开 # 操作符后面的宏

解决：两层宏（正确做法)
// 两层宏（正确做法）
#define str_temp(x) #x
#define str(x) str_temp(x)

#define PLATFORM riscv32

str(PLATFORM)
  ↓ 第一层展开宏定义
str_temp(riscv32)
  ↓ 第二层：字符串化
"riscv32"  
*/
#define str_temp(x) #x
#define str(x) str_temp(x)

// strlen() for string constant
// sizeof() 返回字符串常量长度（包括末尾的 '\0'）
#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)

// calculate the length of an array
#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

// macro concatenation
// ## 是预处理器的连接运算符,它将两个令牌 x 和 y 连接成一个新的令牌
// concat_temp("hello", "world") -> "hello""world"
#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define concat3(x, y, z) concat(concat(x, y), z)
#define concat4(x, y, z, w) concat3(concat(x, y), z, w)
#define concat5(x, y, z, v, w) concat4(concat(x, y), z, v, w)

// macro testing
// See https://stackoverflow.com/questions/26099745/test-if-preprocessor-symbol-is-defined-inside-macro
#define CHOOSE2nd(a, b, ...) b
#define MUX_WITH_COMMA(contain_comma, a, b) CHOOSE2nd(contain_comma a, b)
#define MUX_MACRO_PROPERTY(p, macro, a, b) MUX_WITH_COMMA(concat(p, macro), a, b)
// define placeholders for some property
#define __P_DEF_0  X,
#define __P_DEF_1  X,
#define __P_ONE_1  X,
#define __P_ZERO_0 X,
// define some selection functions based on the properties of BOOLEAN macro
#define MUXDEF(macro, X, Y)  MUX_MACRO_PROPERTY(__P_DEF_, macro, X, Y)
#define MUXNDEF(macro, X, Y) MUX_MACRO_PROPERTY(__P_DEF_, macro, Y, X)
#define MUXONE(macro, X, Y)  MUX_MACRO_PROPERTY(__P_ONE_, macro, X, Y)
#define MUXZERO(macro, X, Y) MUX_MACRO_PROPERTY(__P_ZERO_,macro, X, Y)

// test if a boolean macro is defined
#define ISDEF(macro) MUXDEF(macro, 1, 0)
// test if a boolean macro is undefined
#define ISNDEF(macro) MUXNDEF(macro, 1, 0)
// test if a boolean macro is defined to 1
#define ISONE(macro) MUXONE(macro, 1, 0)
// test if a boolean macro is defined to 0
#define ISZERO(macro) MUXZERO(macro, 1, 0)
// test if a macro of ANY type is defined
// NOTE1: it ONLY works inside a function, since it calls `strcmp()`
// NOTE2: macros defined to themselves (#define A A) will get wrong results
// 这里""的作用是如果macro为空，str(macro)展开为空，还剩下""空字符串，不会导致strcmp报错
#define isdef(macro) (strcmp("" #macro, "" str(macro)) != 0)

// simplification for conditional compilation
#define __IGNORE(...)
#define __KEEP(...) __VA_ARGS__
// keep the code if a boolean macro is defined
#define IFDEF(macro, ...) MUXDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)
// keep the code if a boolean macro is undefined
#define IFNDEF(macro, ...) MUXNDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)
// keep the code if a boolean macro is defined to 1
#define IFONE(macro, ...) MUXONE(macro, __KEEP, __IGNORE)(__VA_ARGS__)
// keep the code if a boolean macro is defined to 0
#define IFZERO(macro, ...) MUXZERO(macro, __KEEP, __IGNORE)(__VA_ARGS__)

// functional-programming-like macro (X-macro)
// apply the function `f` to each element in the container `c`
// NOTE1: `c` should be defined as a list like:
//   f(a0) f(a1) f(a2) ...
// NOTE2: each element in the container can be a tuple
#define MAP(c, f) c(f)

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
/*
SEXT(x, len) 展开为：
({
    struct {
        int64_t n : len;  // 定义一个len位的有符号位域
    } __x = { .n = x };   // 将x的低len位赋值给这个位域
    
    (uint64_t)__x.n;      // 读取位域，自动符号扩展到64位
})
int64_t n : len;  表示只使用 len 位来存储有符号整数,超出的位会被截断,读取时会自动进行符号扩展。
__x = { .n = x }; 取 x 的低 len 位存入位域 n, 如果第 len-1 位是1（负数），位域会记住这是个负数。
(uint64_t)__x.n;  如果位域的最高位（第 len-1 位）是 0，高位填充 0；如果位域的最高位（第 len-1 位）是 1，高位填充 1。
                  最后被转化为无符号整数。
*/
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; })

/* 
这两个宏假设 sz 是 2 的幂次方。
sz = 8 (2^3)
二进制: 0000 1000
sz - 1 = 7
二进制: 0000 0111  ← 低位全是 1
~(sz - 1)
二进制: 1111 1000  ← 掩码：低位全是 0
*/
#define ROUNDUP(a, sz)   ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz) ((((uintptr_t)a)) & ~((sz) - 1))
// __attribute__((aligned(N))) 是 GCC 扩展, 告诉编译器将变量对齐到 N 字节边界
#define PG_ALIGN __attribute((aligned(4096)))

#if !defined(likely)
// __builtin_expect 是 GCC 内建函数
// __builtin_expect(表达式, 期望值)，告诉编译器这个表达式的最可能的值，帮助编译器优化分支预测
#define likely(cond)   __builtin_expect(cond, 1)
#define unlikely(cond) __builtin_expect(cond, 0)
#endif

// for AM IOE
/*
1. 语句表达式 ({ ... })
GCC 扩展，允许在表达式中包含多条语句。GCC 扩展，允许在表达式中包含多条语句。
2. 类型拼接 reg##_T
## 是宏连接符，将 reg 参数和 _T 拼接成类型名
3. 调用底层函数
ioe_read(reg, &__io_param) 读取设备寄存器，结果存储在 __io_param 中
最后的 __io_param 作为整个表达式的返回值
*/
#define io_read(reg) \
  ({ reg##_T __io_param; \
    ioe_read(reg, &__io_param); \
    __io_param; })
/*
1. 可变参数 __VA_ARGS__
接受任意数量的参数,用于初始化结构体.
2. 复合字面量 (type) { ... }
C99 特性，创建匿名的复合类型. (reg##_T) { __VA_ARGS__ } 创建并初始化结构体
3. 写入设备
ioe_write(reg, &__io_param) 将数据写入设备寄存器
*/
#define io_write(reg, ...) \
  ({ reg##_T __io_param = (reg##_T) { __VA_ARGS__ }; \
    ioe_write(reg, &__io_param); })

#endif
