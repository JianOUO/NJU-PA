#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static int intlen(int val) {
  int result = 0;

  if (val == INT_MIN) return 11;

  if (val < 0) {
    result++;
    val = -val;
  }

  do {
    result++;
    val /= 10;
  } while (val != 0);

  return result;
}

static int uintlen(unsigned int val, unsigned int base) {
  int result = 0;
  
  do {
    result++;
    val /= base;
  } while (val != 0);

  return result;
}

static int itoa (int val, char *buf) {
  int result = 0;
  int len = 0;
  int tmp = val;
  if (val == INT_MIN) {
    strcpy(buf, "-2147483648");
    return 11;
  }

  if (val < 0) {
    *buf++ = '-';
    result++;
    val = -val;
  }

  do {
    len++;
    tmp /= 10;
  } while (tmp != 0);

  result += len;

  do {
    *(buf + len - 1) = val % 10 + '0';
    val /= 10;
    len--;
  } while (val != 0);

  return result;
}

static int nuitoa (unsigned int val, char *buf, size_t n, unsigned int base) {
  int result = uintlen(val, base);
  int len = result;
  char trans[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  do {
    if (n >= len) {
      *(buf + len - 1) = trans[val % base];
    }
    val /= base;
    len--;
  } while (val != 0);

  return result;
}

static int nitoa (int val, char *buf, size_t n) {
  int result = 0;
  int len = 0;
  int tmp = val;

  if (val == INT_MIN) {
    char int_min[12] = "-2147483648";
    size_t min = n < 11 ? n : 11;
    for (size_t i = 0; i < min; i++) {
      buf[i] = int_min[i];
    }
    return 11;
  }

  if (val < 0) {
    if (n >= 1) {
      *buf++ = '-';
      n--;
    }
    result++;
    val = -val;
  }

  do {
    len++;
    tmp /= 10;
  } while (tmp != 0);

  result += len;

  do {
    if (n >= len) {
      *(buf + len - 1) = val % 10 + '0';
      //n--;
    }
    val /= 10;
    len--;
  } while (val != 0);

  return result;
}

#define OUTSIZE 256

int printf(const char *fmt, ...) {
  //panic("Not implemented");
  char out[OUTSIZE];
  int result = 0;
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(out, OUTSIZE, fmt, ap);
  va_end(ap);
  for (const char *p = out; *p; p++) {
    putch(*p);
    result++;
  }
  return result;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  //panic("Not implemented");
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
        out += itoa(value, out);
        break;
      }
      case 's': {
        char *str = va_arg(ap, char *);
        while ((s = *str++)) *out++ = s;
        break;
      }
      default: panic("Invalid conversion specifiers");
    }
  }
  *out = '\0';
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
  //panic("Not implemented");
  va_list ap;
  va_start(ap, fmt);
  int result = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return result;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  //panic("Not implemented"); 
  int ret = 0;
  int width = 0;
  char f, s, pad = ' ';
  while ((f = *fmt++)) {
    if (f != '%') {
      if (n > 1) {
        *out++ = f;
        n--;  
      }
      ret++;
      continue;
    }

    if ((f = *fmt) == '%') {
      if (n > 1) {
        *out++ = f;
        n--;
      }
      ret++;
      fmt++;
      continue;
    }

    while ((f = *fmt)) {
      if (f == '0') {
        pad = f;
        fmt++;
        continue;
      } else if (f > '0' && f <= '9') {
        width = f - '0';
        fmt++;
        break; 
      }
      break;
    }

    switch ((f = *fmt++)) {
      case 'd': {
        int value = va_arg(ap, int);
        int len = intlen(value);
        int r, off = width - len;
        while (off-- > 0) {
          if (n > 1) {
            *out++ = pad;
            n--;
          }
          ret++;
        }
        pad = ' ';
        width = 0;
        if (n > 1) {
          r = nitoa(value, out, n - 1);
          if (r > n - 1) {
            out += n - 1;
            n = 1;
          } else {
            out += r;
            n -= r;
          }
        } else {
          r = nitoa(value, out, 0);
        }
        ret += r;
        break;
      }
      case 's': {
        char *str = va_arg(ap, char *);
        while ((s = *str++)) {
          if (n > 1) {
            *out++ = s;
            n--;
          }
          ret++;
        }
        break;
      }
      case 'u': {
        unsigned int value = va_arg(ap, unsigned int);
        int len = uintlen(value, 10);
        int r, off = width - len;
        while (off-- > 0) {
          if (n > 1) {
            *out++ = pad;
            n--;
          }
          ret++;
        } 
        pad = ' ';
        width = 0;
        if (n > 1) {
          r = nuitoa(value, out, n - 1, 10);
          if (r > n - 1) {
            out += n - 1;
            n = 1;
          } else {
            out += r;
            n -= r;
          }
        } else {
          r = nuitoa(value, out, 0, 10);
        }
        ret += r;
        break;
      }
      case 'x': {
        unsigned int value = va_arg(ap, unsigned int);
        int len = uintlen(value, 16);
        int r, off = width - len;
        while (off-- > 0) {
          if (n > 1) {
            *out++ = pad;
            n--;
          }
          ret++;
        }
        pad = ' ';
        width = 0;
        if (n > 1) {
          r = nuitoa(value, out, n - 1, 16);
          if (r > n - 1) {
            out += n - 1;
            n = 1;
          } else {
            out += r;
            n -= r;
          }
        } else {
          r = nuitoa(value, out, 0, 16);
        }
        ret += r;
        break;
      }
      default: panic("Invalid conversion specifiers");
    }
  }
  *out = '\0';
  return ret;
}

#endif