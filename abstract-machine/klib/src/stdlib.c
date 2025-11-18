#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <limits.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
// 随机数生成器的内部状态
static unsigned long int next = 1;

// LCG（线性同余伪随机数生成器）的实现
// 一种经典的伪随机数算法,基于简单的数学公式
int rand(void) {
  // RAND_MAX assumed to be 32767
  /*
  线性同余公式:
  X(n+1) = (a × X(n) + c) mod m
  其中：
  a = 1103515245  (乘数)
  c = 12345       (增量)
  m = 2^32        (模数，unsigned long 隐式)
  */
  next = next * 1103515245 + 12345;
  // next / 65536 - 右移 16 位 
  // 目的：丢弃低 16 位, 因为LCG 的低位随机性较差，高位的随机性更好
  // % 32768 - 限制范围, 因为C 标准要求 RAND_MAX 至少为 32767
  return (unsigned int)(next/65536) % 32768;
}

// 初始化随机数生成器
void srand(unsigned int seed) {
  next = seed;
}

// 如果x == INT_MIN, 会出错
int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

static char *hbrk = NULL;

void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
  /*
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  panic("Not implemented");
#endif
  return NULL;
  */
  if (hbrk == NULL) hbrk = (void *)ROUNDUP(heap.start, 8);
  size = (size_t)ROUNDUP(size, 8);
  char *old = hbrk;
  hbrk += size;
  assert((uintptr_t)hbrk >= (uintptr_t)heap.start && (uintptr_t)hbrk < (uintptr_t)heap.end);
  return old;
}

void free(void *ptr) {
}

#endif
