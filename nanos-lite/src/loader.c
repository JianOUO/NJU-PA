#include <proc.h>
#include <elf.h>

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
#else
# error Unsupported ISA
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
  Elf_Ehdr ehdr;
  Elf_Shdr shdr;
  Elf_Phdr phdr;
  uint32_t e_phnum;

  assert(ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr)) == sizeof(Elf_Ehdr));
  assert(ehdr.e_ident[EI_CLASS] == Elf_Class);
  assert(ehdr.e_ident[EI_MAG0] == ELFMAG0 && ehdr.e_ident[EI_MAG1] == ELFMAG1 && ehdr.e_ident[EI_MAG2] == ELFMAG2 && ehdr.e_ident[EI_MAG3] == ELFMAG3);
  assert(ehdr.e_machine == EXPECT_TYPE);

  assert(ehdr.e_shoff != 0); 
  assert(ramdisk_read(&shdr, ehdr.e_shoff, sizeof(Elf_Shdr)) == sizeof(Elf_Shdr));

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
    assert(ramdisk_read(&phdr, ehdr.e_phoff + i * ehdr.e_phentsize, sizeof(Elf_Phdr)) == sizeof(Elf_Phdr));
    if (phdr.p_type == PT_LOAD) {
      assert(ramdisk_read((void *)phdr.p_vaddr, phdr.p_offset, phdr.p_memsz) == phdr.p_memsz);
      memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
    }
  }

  assert(ehdr.e_entry != 0);
  return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

