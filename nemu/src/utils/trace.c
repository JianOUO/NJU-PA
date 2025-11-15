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
  printf(ANSI_FMT("[MTRACE] Read  " FMT_WORD " FROM " FMT_WORD "\n", ANSI_FG_YELLOW), data, addr);
  return;
}

void mtrace_write(paddr_t addr, int len, word_t data) {
  switch (len) {
    case 1: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_WORD "\n", ANSI_FG_YELLOW), data & 0xFF, addr); break;
    case 2: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_WORD "\n", ANSI_FG_YELLOW), data & 0xFFFF, addr); break;
    case 4: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_WORD "\n", ANSI_FG_YELLOW), data & 0xFFFFFFFF, addr); break;
    IFDEF(CONFIG_ISA64, case 8: printf(ANSI_FMT("[MTRACE] Write " FMT_WORD " TO   " FMT_WORD "\n", ANSI_FG_YELLOW), data & 0xFFFFFFFFFFFFFFFF, addr); break);
    IFDEF(CONFIG_RT_CHECK, default: assert(0));
  }
  return;
}

#endif

#ifdef CONFIG_FTRACE

void init_ftrace(const char *elf) {

}

#endif