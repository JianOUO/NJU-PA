#include "am.h"
#include "klib-macros.h"
#include "memory.h"
#include <proc.h>
#include <elf.h>
#include <fs.h>
#include <stdint.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
# define Elf_Shdr Elf64_Shdr
# define Elf_Class ELFCLASS64
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
# define Elf_Shdr Elf32_Shdr
# define Elf_Class ELFCLASS32
#endif

#if defined(__ISA_AM_NATIVE__)
# define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_RISCV32__)

# define EXPECT_TYPE EM_RISCV
# define VPN1(va) ((va) >> 22)
# define VPN0(va) (((va) << 10) >> 22)
# define PTE2PPN(pte) ((pte) >> 10)
# define PTE_V 0x1

typedef uintptr_t PTE;

void *look_up_pa(AddrSpace *as, uintptr_t va) {
  assert(as != NULL);
  assert(as->ptr != NULL);
  PTE *lv1_pg = as->ptr;
  uintptr_t vpn1 = VPN1(va);
  PTE lv1_pte = *(lv1_pg + vpn1);
  if ((lv1_pte & PTE_V) == 0) return NULL;

  PTE *lv0_pg = (PTE *)(PTE2PPN(lv1_pte) << 12);
  uintptr_t vpn0 = VPN0(va);
  PTE lv0_pte = *(lv0_pg + vpn0);
  if ((lv0_pte & PTE_V) == 0) return NULL;

  return (void *)(PTE2PPN(lv0_pte) << 12);
}

#else
# error Unsupported ISA
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  int fd = fs_open(filename, 0, 0);
  //printf("fd: %d\n", fd);
  Elf_Ehdr ehdr;
  Elf_Shdr shdr;
  Elf_Phdr phdr;
  uint32_t e_phnum;

  //assert(ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr)) == sizeof(Elf_Ehdr));
  assert(fs_read(fd, &ehdr, sizeof(Elf_Ehdr)) == sizeof(Elf_Ehdr));
  assert(ehdr.e_ident[EI_CLASS] == Elf_Class);
  assert(ehdr.e_ident[EI_MAG0] == ELFMAG0 && ehdr.e_ident[EI_MAG1] == ELFMAG1 && ehdr.e_ident[EI_MAG2] == ELFMAG2 && ehdr.e_ident[EI_MAG3] == ELFMAG3);
  assert(ehdr.e_machine == EXPECT_TYPE);

  assert(ehdr.e_shoff != 0); 
  //assert(ramdisk_read(&shdr, ehdr.e_shoff, sizeof(Elf_Shdr)) == sizeof(Elf_Shdr));
  assert(fs_lseek(fd, ehdr.e_shoff, SEEK_SET) == ehdr.e_shoff);
  assert(fs_read(fd, &shdr, sizeof(Elf_Shdr)) == sizeof(Elf_Shdr));

  assert(ehdr.e_phoff != 0);
  assert(ehdr.e_phnum != 0);
  //assert(ehdr.e_phentsize == sizeof(Elf_Phdr));
  if (ehdr.e_phnum == PN_XNUM) {
    assert(shdr.sh_info != 0);
    e_phnum = shdr.sh_info;
  } else {
    e_phnum = ehdr.e_phnum;
  }
  for (uint32_t i = 0; i < e_phnum; i++) {
    //assert(ramdisk_read(&phdr, ehdr.e_phoff + i * ehdr.e_phentsize, sizeof(Elf_Phdr)) == sizeof(Elf_Phdr));
    size_t offset = ehdr.e_phoff + i * ehdr.e_phentsize;
    assert(fs_lseek(fd, offset, SEEK_SET) == offset);
    assert(fs_read(fd, &phdr, sizeof(Elf_Phdr)) == sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      //assert(fs_lseek(fd, phdr.p_offset, SEEK_SET) == phdr.p_offset);
      /*
      assert(fs_read(fd, (void *)phdr.p_vaddr, phdr.p_memsz) == phdr.p_memsz);
      p_filesz：这个段在 ELF 文件里实际有多少字节, p_memsz ：这个段加载到内存后应该占多少字节 
      通常应该从文件里读取 p_filesz 字节，而不是 p_memsz 字节；然后再把 p_memsz - p_filesz 的部分清零。
      因为 fs_read() 最多只能读到当前文件剩余的字节数，如果我请求读phdr.p_memsz，但从 phdr.p_offset 到文件结尾已经没有这么多字节了，
      fs_read() 会把 len 缩短，只返回实际还能读到的长度，最终导致assert失败。
      */
      //assert(fs_read(fd, (void *)phdr.p_vaddr, phdr.p_filesz) == phdr.p_filesz);
      //memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
      uintptr_t start = phdr.p_vaddr;
      uintptr_t end = start + phdr.p_filesz;
      //printf("start: %x, end: %x\n", start, end);
      uintptr_t va_start = ROUNDDOWN(start, PGSIZE);
      uintptr_t va_end = ROUNDUP(start + phdr.p_memsz, PGSIZE); 
      assert(fs_lseek(fd, phdr.p_offset, SEEK_SET) == phdr.p_offset);
      for (uintptr_t va = va_start; va < va_end; va += PGSIZE) {
          //printf("start_0: %x, end_0: %x\n", start, end);
          void *pa = look_up_pa(&(pcb->as), va);
          if (pa == NULL) {
            pa = new_page(1);
            memset(pa, 0, PGSIZE);
            map(&(pcb->as), (void *)va, pa, MMAP_READ | MMAP_WRITE);
          }
          uintptr_t left = va > start ? va : start;
          uintptr_t right = (va + PGSIZE) < end ? (va + PGSIZE) : end;
          //assert(left < right);
          if (left < right) fs_read(fd, (left - va) + pa, right - left);
          //printf("start_1: %x, end_1: %x\n", start, end);
          printf("left: %x, right: %x, start: %x, end: %x, va: %x, va + PGSIZE: %x, pa: %x\n", left, right, start, end, va, va + PGSIZE, (uintptr_t)pa);
      }
    }
  }
  assert(fs_close(fd) == 0);
  assert(ehdr.e_entry != 0);
  printf("entry: 0x%x\n", ehdr.e_entry);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

uintptr_t uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  return entry;
}

