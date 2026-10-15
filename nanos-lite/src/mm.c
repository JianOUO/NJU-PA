#include "am.h"
#include "klib-macros.h"
#include "proc.h"
#include <memory.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  uintptr_t old = (uintptr_t)pf;
  pf = (void *)(old + nr_page * PGSIZE);
  return (void *)old;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE == 0);
  void *addr = new_page((size_t)n / PGSIZE);
  memset(addr, 0, (size_t)n);
  return addr;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
// 处理 brk() 系统调用，用于调整程序堆的边界
int mm_brk(uintptr_t brk) {
  uintptr_t brk_roundup = ROUNDUP(brk, PGSIZE);
  while (current->max_brk < brk_roundup) {
    void *pa = new_page(1);
    memset(pa, 0, PGSIZE);
    map(&(current->as), (void *)current->max_brk, pa, MMAP_READ | MMAP_WRITE);
    current->max_brk += PGSIZE;
  }
  return 0;
}

void init_mm() {
  // pf：静态变量，指向空闲物理页的起始地址
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
