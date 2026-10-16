#include <am.h>
#include <nemu.h>
#include <klib.h>
#include <stdint.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);
  printf("kas.ptr: 0x%x\n", (uintptr_t)kas.ptr);
  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  printf("updir: 0x%x\n", (uintptr_t)updir);
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

#define VPN1(va) ((va) >> 22)
#define VPN0(va) (((va) << 10) >> 22)
#define PA2PPN(pa) ((pa) >> 12)
#define PTE2PPN(pte) ((pte) >> 10)

void map(AddrSpace *as, void *va, void *pa, int prot) {
  assert(as->ptr != NULL);
  PTE *lv1_pg = as->ptr;
  uintptr_t va_ = (uintptr_t)va;
  uintptr_t pa_ = (uintptr_t)pa;
  uintptr_t vpn1 = VPN1(va_);
  PTE lv1_pte = *(lv1_pg + vpn1);
  if ((lv1_pte & PTE_V) == 0) {
    uintptr_t new_pg = (uintptr_t)pgalloc_usr(PGSIZE);
    lv1_pte = PA2PPN(new_pg) << 10;
    lv1_pte |= PTE_V;
    *(lv1_pg + vpn1) = lv1_pte;
  }

  PTE *lv0_pg = (PTE *)(PTE2PPN(lv1_pte) << 12);
  uintptr_t vpn0 = VPN0(va_);
  PTE lv0_pte = *(lv0_pg + vpn0);
  assert((lv0_pte & PTE_V) == 0);
  lv0_pte = PA2PPN(pa_) << 10;
  lv0_pte |= PTE_V;
  *(lv0_pg + vpn0) = lv0_pte;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *context = (Context *)kstack.end - 1;
  /*
  初始化上下文
  先全部置零
  epc设置为entry-4，因为后续在__am_asm_trap要+4
  为了保证DiffTest的正确运行 和 mret之后开中断，mstatus设置为0x1880
  */
  memset((void *)context, 0, sizeof(Context));
  context->pdir = as->ptr;
  context->mepc = (uintptr_t)entry;
  context->mstatus = 0x1880; // MPIE = 1u << 7;
  return context;
}
