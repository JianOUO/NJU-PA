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
    //Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb, pcb + 1 }, entry, arg);
}

void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]) {
  char *end = (char *)new_page(8) + 8 * PGSIZE;
  int argc = 0, envc = 0;
  assert(argv && envp);
  while (argv[argc] != NULL) argc++;
  while (envp[envc] != NULL) envc++;
  uintptr_t argv_ptr[argc + 1], envp_ptr[envc + 1];
  envp_ptr[envc] = 0;
  for (int i = envc - 1; i >= 0; i--) {
    int len = strlen(envp[i]);
    end -= (len + 1);
    strcpy(end, envp[i]);
    envp_ptr[i] = (uintptr_t)end;
  }
  argv_ptr[argc] = 0; 
  for (int i = argc - 1; i >= 0; i--) {
    int len = strlen(argv[i]);
    end -= (len + 1);
    strcpy(end, argv[i]);
    argv_ptr[i] = (uintptr_t)end;
  }
  uintptr_t *new_end = (uintptr_t *)((uintptr_t)end & ~((uintptr_t)(sizeof(uintptr_t)) - 1));
  for (int i = envc; i >= 0; i--) {
    *(--new_end) = envp_ptr[i];
  }
  for (int i = argc; i >= 0; i--) {
    *(--new_end) = argv_ptr[i];
  }
  *(--new_end) = (uintptr_t)argc;

  uintptr_t entry = uload(pcb, filename);
  // 在riscv-nemu下，as可以传NULL，在native环境下会触发段错误，暂且不管，不在native下运行
  pcb->cp = ucontext(NULL, (Area) { pcb, pcb + 1 }, (void *)entry);
  pcb->cp->GPRx = (uintptr_t)new_end;
}

void init_proc() {
  context_kload(&pcb[0], hello_fun, (void *)0);
  char *argv[] = {"/bin/menu", NULL};
  char *envp[] = {NULL};
  context_uload(&pcb[1], "/bin/menu", argv, envp);
  //context_kload(&pcb[1], hello_fun, (void *)1);

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
