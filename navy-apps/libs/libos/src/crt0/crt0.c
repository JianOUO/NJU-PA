#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = (int)(*args);
  printf("argc:%d\n", argc);

  char **argv = (char **)(args + 1);
  char **envp = argv + (argc + 1);
  //char *envp[] = {NULL };
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
