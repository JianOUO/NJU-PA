#include <NDL.h>
#include <sdl-timer.h>
#include <assert.h>
#include <stdio.h>

extern uint32_t sdl_init_time;

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  assert(0);
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  assert(0);
  return 1;
}

uint32_t SDL_GetTicks() {
  uint32_t cur = NDL_GetTicks();
  assert(cur >= sdl_init_time);
  return cur - sdl_init_time;
}

void SDL_Delay(uint32_t ms) {
  assert(0);
}
