#include <common.h>

#ifdef CONFIG_ITRACE

#define IRINGBUFLEN 32

typedef struct IRingBuf {
  char buf[128];
} IRingBuf;

static IRingBuf iringbuf[IRINGBUFLEN];
static int top = 0;
static int count = 0;

void iringbuf_push(const char *log_buf) {
  Assert(log_buf != NULL, "%s\n", "iringbuf_push: log_buf is NULL");
  strcpy(iringbuf[top].buf, log_buf);
  top = (top + 1) % IRINGBUFLEN;
  if (count < IRINGBUFLEN) count++;
}

void iringbuf_print() {
  if (count == IRINGBUFLEN) {
    for (int i = 0; i < IRINGBUFLEN; i++) {
      printf(ANSI_FMT("[ITRACE] %s\n", ANSI_FG_BLACK), iringbuf[(top + i) % IRINGBUFLEN].buf);
    }
  } else {
    for (int i = 0; i < top; i++) {
      printf(ANSI_FMT("[ITRACE] %s\n", ANSI_FG_BLACK), iringbuf[(i) % IRINGBUFLEN].buf);
    }
  }
}

#endif

#ifdef CONFIG_MTRACE
void mtrace_read(paddr_t addr, word_t data) {
  printf(ANSI_FMT("[MTRACE] Read  " FMT_WORD " FROM " FMT_PADDR "\n", ANSI_FG_YELLOW), data, addr);
  return;
}

void mtrace_write(paddr_t addr, int len, word_t data) {
  switch (len) {
    case 1: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_PADDR "\n", ANSI_FG_YELLOW), data & 0xFF, addr); break;
    case 2: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_PADDR "\n", ANSI_FG_YELLOW), data & 0xFFFF, addr); break;
    case 4: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_PADDR "\n", ANSI_FG_YELLOW), data & 0xFFFFFFFF, addr); break;
    IFDEF(CONFIG_ISA64, case 8: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_PADDR "\n", ANSI_FG_YELLOW), data & 0xFFFFFFFFFFFFFFFF, addr); break);
    IFDEF(CONFIG_RT_CHECK, default: assert(0));
  }
  return;
}

#endif

#ifdef CONFIG_FTRACE

#include <fcntl.h>
#include <unistd.h>
#include <elf.h>

static int fd = 0;
static Elf32_Off  symtab_off;
static Elf32_Off  strtab_off;
static uint32_t   symtab_entsize;
static uint32_t   symtab_entcnt;
static Elf32_Sym  symtab_ent;
static uint32_t   space = 0;

void init_ftrace(const char *elf) {
  unsigned char e_ident[EI_NIDENT];
  Elf32_Ehdr ehdr;
  Elf32_Shdr shdr;
  uint32_t shnum;
  uint32_t shstrndx;
  if (elf != NULL) {
    fd = open(elf, O_RDONLY);
    Assert(fd != -1, "elf open failed");
    Log("Get ELF file %s", elf);
  } else {
    Log("ELF file is NULL"); 
    return;
  }
  Assert(read(fd, e_ident, EI_NIDENT) == EI_NIDENT, "e_ident read failed");
  Assert(e_ident[EI_CLASS] == ELFCLASS32, "EI_CLASS is NOT ELFCLASS32");
  Assert(e_ident[EI_MAG0] == ELFMAG0 && e_ident[EI_MAG1] == ELFMAG1 && e_ident[EI_MAG2] == ELFMAG2 && e_ident[EI_MAG3] == ELFMAG3,
  "Invalid ELF magic number");
  // 一开始忘记将文件指针重置到文件开头
  Assert(lseek(fd, 0, SEEK_SET) != -1, "to file start lseek failed");
  Assert(read(fd, &ehdr, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr), "ehdr read failed");
  Assert(ehdr.e_machine == EM_RISCV, "The elf architecture is NOT riscv");
  Assert(ehdr.e_shoff != 0, "this elf file has no section table");
  Assert(lseek(fd, ehdr.e_shoff, SEEK_SET) != -1, "e_shoff lseek failed");
  Assert(read(fd, &shdr, sizeof(Elf32_Shdr)) == sizeof(Elf32_Shdr), "shdr entry 0 read failed");
  Assert(ehdr.e_shnum != 0 || shdr.sh_size != 0, "section header table's entry num is zero");
  if (ehdr.e_shnum == 0 && shdr.sh_size != 0) shnum = shdr.sh_size;
  else shnum = ehdr.e_shnum;
  Assert(ehdr.e_shstrndx != SHN_UNDEF, "the file has no section name string table");
  if (ehdr.e_shstrndx == SHN_XINDEX) shstrndx = shdr.sh_link;
  else shstrndx = ehdr.e_shstrndx;
  for (uint32_t i = 1; i < shnum; i++) {
    Assert(read(fd, &shdr, sizeof(Elf32_Shdr)) == sizeof(Elf32_Shdr), "shdr entry %" PRIu32 " read failed", i);
    if (shdr.sh_type == SHT_SYMTAB) {
      symtab_off = shdr.sh_offset;
      Assert(shdr.sh_entsize != 0, "symtab entsize == 0");
      symtab_entsize = shdr.sh_entsize;
      symtab_entcnt = shdr.sh_size / shdr.sh_entsize;
    } else if (shdr.sh_type == SHT_STRTAB && i != shstrndx) {
      strtab_off = shdr.sh_offset;
    }
  }
  return;
}

void ftrace(vaddr_t pc, vaddr_t addr, bool call) {
  Assert(lseek(fd, symtab_off, SEEK_SET) != -1, "symtab offset lseek failed");
  for (uint32_t i = 0; i < symtab_entcnt; i++) { 
    Assert(read(fd, &symtab_ent, sizeof(Elf32_Sym)) == sizeof(Elf32_Sym), "shdr entry %" PRIu32 "read failed", i);
    if (ELF32_ST_TYPE(symtab_ent.st_info) == STT_FUNC && addr >= symtab_ent.st_value && addr < symtab_ent.st_value + symtab_ent.st_size) {
      if (call) {
        char c;
        printf(ANSI_FG_CYAN "[FTRACE] " FMT_WORD ": ", pc);
        for (uint32_t j = 0; j < space; j++) putchar(' ');
        printf("call [");
        Assert(lseek(fd, strtab_off + symtab_ent.st_name, SEEK_SET) != -1, "function name offset lseek failed");
        Assert(read(fd, &c, sizeof(char)) == sizeof(char), "function name read failed");
        while (c != '\0') {
          putchar(c);
          Assert(read(fd, &c, sizeof(char)) == sizeof(char), "function name read failed");
        }
        printf("@" FMT_WORD "]\n" ANSI_NONE, symtab_ent.st_value);
        space += 2;
        return;
      } else {
        space -= 2;
        char c;
        printf(ANSI_FG_CYAN "[FTRACE] " FMT_WORD ": ", pc);
        for (uint32_t j = 0; j < space; j++) putchar(' ');
        printf("ret  [");
        Assert(lseek(fd, strtab_off + symtab_ent.st_name, SEEK_SET) != -1, "function name offset lseek failed");
        Assert(read(fd, &c, sizeof(char)) == sizeof(char), "function name read failed");
        while (c != '\0') {
          putchar(c);
          Assert(read(fd, &c, sizeof(char)) == sizeof(char), "function name read failed");
        }
        printf("]\n" ANSI_NONE);
        return;
      }
    }
  }
}

#endif

#ifdef CONFIG_DTRACE

#include <device/map.h>

void dtrace_read(paddr_t addr, int len, IOMap *map, word_t data) {
  printf(ANSI_FMT("[DTRACE] Read  " FMT_WORD " FROM %s(" FMT_PADDR ")", ANSI_FG_MAGENTA), data, map->name, addr);
  printf("\n");
}

void dtrace_write(paddr_t addr, int len, IOMap *map, word_t data) {
  printf(ANSI_FMT("[DTRACE] Write " FMT_WORD " TO   %s(" FMT_PADDR ")", ANSI_FG_MAGENTA), data, map->name, addr);
  printf("\n");
}

#endif