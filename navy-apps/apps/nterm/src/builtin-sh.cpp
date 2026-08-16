#include <nterm.h>
#include <stdarg.h>
#include <unistd.h>
#include <SDL.h>
#include <string.h>
#include <stdlib.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() {
  sh_printf("sh> ");
}

#define MAXARGS 10
static void sh_handle_cmd(const char *cmd) {
  assert((uintptr_t)cmd != 64);
  int len = strlen(cmd);
  char buf[len + 1];
  strcpy(buf, cmd);
  char *command = strtok(buf, " \t\r\n");
  char *argv[MAXARGS] = {command};
  char *envp[] = {NULL};
  printf("command:%s\n", argv[0]);
  int argc = 1;
  while (argc < MAXARGS - 1 && (argv[argc] = strtok(NULL, " \t\r\n")) != NULL) {
    printf("args[%d]:%s\n", argc, argv[argc]);
    argc++;
  }
  argv[argc] = NULL;
  execvpe(command, argv, envp);
}

void builtin_sh_run() {
  sh_banner();
  sh_prompt();

  assert(setenv("PATH", "/bin", 0) != -1);
  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}
