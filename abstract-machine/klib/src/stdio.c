#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  //panic("Not implemented");
  assert(out != NULL);
  assert(fmt != NULL);
  const char *begin = out;
  char f, s;
  while ((f = *fmt++)) {
    if (f != '%') {
      *out++ = f;
      continue;
    }
    switch ((f = *fmt++)) {
      case 'd': {
        int value = va_arg(ap, int);
        assert(itoa(value, out) == 2);
        break;
      }
      case 's': {
        char *str = va_arg(ap, char *);
        assert(str != NULL);
        while ((s = *str++)) *out++ = s;
        break;
      }
      default: panic("Invalid conversion specifiers");
    }
  }
  assert(f == '\0');
  *out = f;
  return out - begin;
}

int sprintf(char *out, const char *fmt, ...) {
  //panic("Not implemented");
  va_list ap;
  va_start(ap, fmt);
  int result = vsprintf(out, fmt, ap);
  va_end(ap);
  return result;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented"); 
}

#endif