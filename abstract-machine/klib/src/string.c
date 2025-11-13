#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  panic("Not implemented");
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

void *memset(void *s, int c, size_t n) {
  //panic("Not implemented");
  unsigned char *m = s;
  for (size_t i = 0; i < n; i++) *(m + i) = c;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
  /*
  const unsigned char *in1 = in;
  unsigned char *out1 = out;
  for (size_t i = 0; i < n; i++)
    *(out1 + i) = *(in1 + i);
  return out1;
  */
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
