#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  //panic("Not implemented");
  size_t result = 0;
  while (*(s + result)) result++;
  return result;
}

char *strcpy(char *dst, const char *src) {
  //panic("Not implemented");
  char *d = dst;
  char s;
  do {
    *d++ = s = *src++;
  } while (s);
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  //panic("Not implemented");
  char *d = dst;
  char s;
  while (*d++) {}
  d--;
  do {
    *d++ = s = *src++;
  } while (s);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  //panic("Not implemented");
  unsigned char S1, S2;
  do {
    S1 = *s1++;
    S2 = *s2++;
  } while (S1 == S2 && S1 && S2);
  // 两个 unsigned char 都会先提升为 int, 在 int 类型上进行减法
  return S1 - S2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

#define OPSIZ 4

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
  uintptr_t dstp = (uintptr_t) s;
  if (n >= 8) {
    size_t xlen;
    uint32_t cccc = (unsigned char)c;
    cccc |= cccc << 8;
    cccc |= cccc << 16;
    
    while (dstp % OPSIZ != 0) {
      ((unsigned char *)dstp)[0] = c;
      dstp += 1;
      n -= 1;
    }

    xlen = n / (OPSIZ * 8);
    while (xlen > 0) {
      ((uint32_t *)dstp)[0] = cccc;
      ((uint32_t *)dstp)[1] = cccc;
      ((uint32_t *)dstp)[2] = cccc;
      ((uint32_t *)dstp)[3] = cccc;
      ((uint32_t *)dstp)[4] = cccc;
      ((uint32_t *)dstp)[5] = cccc;
      ((uint32_t *)dstp)[6] = cccc;
      ((uint32_t *)dstp)[7] = cccc;
      dstp += OPSIZ * 8;
      xlen -= 1;
    }
    n %= OPSIZ * 8;

    xlen = n / OPSIZ;
    while (xlen > 0) {
      ((uint32_t *)dstp)[0] = cccc;
      dstp += OPSIZ;
      xlen -= 1;
    }
    n %= OPSIZ;
  }
  while (n > 0) {
    ((unsigned char *)dstp)[0] = c;
    dstp += 1;
    n -= 1;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  //panic("Not implemented");
  uintptr_t dstp = (uintptr_t)dst;
  uintptr_t srcp = (uintptr_t)src;
  if (srcp < dstp && dstp - srcp < n) {
    uint8_t temp[n];
    memcpy(temp, src, n);
    memcpy(dst, temp, n);
  } else {
    memcpy(dst, src, n);
  }
  return dst;
}

#define OP_T_THRES 16

#define BYTE_COPY_FWD(dstp, srcp, nbytes)           \
do {                                                \
  size_t __nbytes = (nbytes);                       \
  while (__nbytes > 0) {                            \
    unsigned char x = ((unsigned char *)srcp)[0]; \
    srcp += 1;                                    \
    __nbytes -= 1;                                  \
    ((unsigned char *)dstp)[0] = x;               \
    dstp += 1;                                    \
  }                                                 \
} while (0)                                       

#define WORD_COPY_FWD(dstp, srcp, nbytes)         \
do {                                              \
  size_t xlen = nbytes / OPSIZ;                \
  while (xlen > 0) {                          \
    uint32_t x = ((uint32_t *)srcp)[0];         \
    srcp += OPSIZ;                              \
    ((uint32_t *)dstp)[0] = x;                  \
    dstp += OPSIZ;                              \
    xlen -= 1;                                  \
  }                                             \
  nbytes %= OPSIZ;                               \
} while (0)                                       

// 只对dstp不对齐进行了处理
void *memcpy(void *out, const void *in, size_t n) {
  //panic("Not implemented");
  /*
  uintptr_t dstp = (uintptr_t)out;
  uintptr_t srcp = (uintptr_t)in;
  if (n >= OP_T_THRES) {
    n -= (-dstp) % OPSIZ;
    BYTE_COPY_FWD(dstp, srcp, (-dstp) % OPSIZ);
    WORD_COPY_FWD(dstp, srcp, n);
  }
  BYTE_COPY_FWD(dstp, srcp, n);
  return out;
  */
  const char *src = in;
  char *dst = out;
  for (size_t i = 0; i < n; i++) {
    *(dst + i) = *(src + i);
  }
  //printf("%s", out);
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  //panic("Not implemented");
  const unsigned char *m1 = s1;
  const unsigned char *m2 = s2;
  unsigned char M1, M2;
  for (size_t i = 0; i < n; i++) {
    M1 = *(m1 + i);
    M2 = *(m2 + i);
    if (M1 == M2) continue;
    // 两个 unsigned char 都会先提升为 int, 在 int 类型上进行减法
    return M1 - M2;
  }
  return 0;
}

#endif
