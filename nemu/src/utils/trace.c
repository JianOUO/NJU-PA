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