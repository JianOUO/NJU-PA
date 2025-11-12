#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif

/*
推荐实现顺序：
实现 vsnprintf() - 核心格式化引擎
基于它实现 snprintf() - 包装可变参数
基于 vsnprintf 实现 vsprintf() - 不限长度
基于 snprintf 实现 sprintf() - 不限长度
基于 vsnprintf 实现 printf() - 输出到标准输出
*/