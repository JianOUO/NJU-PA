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

/************************************
* .section .rodata 声明接下来的内容放入 .rodata 段（Read-Only Data，只读数据段）
* .globl bin_start, bin_end 声明 bin_start 和 bin_end 为全局符号,使得这两个标签可以被其他 C/汇编代码引用,这样就能在代码中获取嵌入文件的起始和结束地址
* bin_start: 定义标签 bin_start，标记嵌入文件的起始地址, 其他代码可以通过 extern char bin_start[]; 来引用
* #ifdef BIN_PATH 如果定义了 BIN_PATH 宏
* .incbin BIN_PATH - include binary，将 BIN_PATH 指定的二进制文件内容直接嵌入到这里
* 其中BIN_PATH 在filelist.mk中定义
* #endif - 条件结束
* 定义标签 bin_end，标记嵌入文件的结束地址
* bin_end - bin_start 就是嵌入文件的大小
*************************************/

.section .rodata
.globl bin_start, bin_end
bin_start:
#ifdef BIN_PATH
.incbin BIN_PATH
#endif
bin_end:
