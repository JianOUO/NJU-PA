#include <common.h>

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

/* The kernel is monolithic, therefore we do not need to
 * translate the address `buf' from the user process to
 * a physical one, which is necessary for a microkernel.
 */

/* read `len' bytes starting from `offset' of ramdisk into `buf' */
size_t ramdisk_read(void *buf, size_t offset, size_t len) {
  printf("ramdisk:%p", &ramdisk_start);
  assert(offset + len <= RAMDISK_SIZE);
  memcpy(buf, &ramdisk_start + offset, len);
  //printf("%s", buf);
  return len;
}

/* write `len' bytes starting from `buf' into the `offset' of ramdisk */
size_t ramdisk_write(const void *buf, size_t offset, size_t len) {
  assert(offset + len <= RAMDISK_SIZE);
  memcpy(&ramdisk_start + offset, buf, len);
  return len;
}

void init_ramdisk() {
  Log("ramdisk info: start = %p, end = %p, size = %d bytes",
      &ramdisk_start, &ramdisk_end, RAMDISK_SIZE);
}

size_t get_ramdisk_size() {
  return RAMDISK_SIZE;
}

/*
0x80000000  ┌─────────────────┐
            │   .text (代码)  │
            ├─────────────────┤
            │   .rodata       │
            ├─────────────────┤
            │   .data         │
            │   ├─ ramdisk    │ ← ramdisk_start
            │   │  (29020字节)│
            │   └─            │ ← ramdisk_end
            ├─────────────────┤
            │   .bss          │
            ├─────────────────┤
            │   Stack (32KB)  │
            ├─────────────────┤
            │   Heap Start    │ ← _heap_start (对齐后)
            │   ├─ Free Pages │ ← pf (空闲页从这里开始)
            │   │             │
            │   │  (可分配内存)│
            │   │             │
0x88000000  └─────────────────┘ ← PMEM_END (heap.end)
*/