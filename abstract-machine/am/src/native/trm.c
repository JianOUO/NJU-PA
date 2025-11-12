#include <am.h>
#include <stdio.h>
#include <klib-macros.h>

void __am_platform_dummy();
void __am_exit_platform(int code);

void trm_init() {
  __am_platform_dummy();
}

void putch(char ch) {
  putchar(ch);
}

// "40h" 中的 '4' 和 '0' 不是普通字符，而是特殊占位符！它们会被替换成退出码的十六进制数字。
// '4' - 将被替换为退出码的高 4 位（十六进制第二位）,'0' - 将被替换为退出码的低 4 位（十六进制第一位）
// 这样做是因为：不依赖标准库，不需要 printf()、sprintf() 等格式化函数，TRM 是最底层的抽象，应该尽可能简单
void halt(int code) {
  const char *fmt = "Exit code = 40h\n";
  for (const char *p = fmt; *p; p++) {
    char ch = *p;
    if (ch == '0' || ch == '4') {
      ch = "0123456789abcdef"[(code >> (ch - '0')) & 0xf];
    }
    putch(ch);
  }
  __am_exit_platform(code);
  putstr("Should not reach here!\n");
  while (1);
}

Area heap = {};
