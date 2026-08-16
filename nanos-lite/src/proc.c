#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
uintptr_t uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb, pcb + 1 }, entry, arg);
}

void context_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = uload(pcb, filename);
  pcb->cp = ucontext(NULL, (Area) { pcb, pcb + 1 }, (void *)entry);
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)0);
  //context_kload(&pcb[1], hello_fun, (void *)1);
  context_uload(&pcb[1], "/bin/pal");

  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  // naive_uload(NULL, "/bin/nterm");
}

Context* schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  return current->cp;
}
