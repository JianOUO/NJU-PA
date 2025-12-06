#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  assert(fp);
  assert(fseek(fp, 0, SEEK_END) != -1);
  long size = ftell(fp);
  assert(size != -1);
  char *buf = (char *)malloc(size);
  assert(buf);
  assert(fseek(fp, 0, SEEK_SET) != -1);
  int ret = fread(buf, size, 1, fp);
  assert(ret == 1);
  SDL_Surface *s = STBIMG_LoadFromMemory(buf, size);
  assert(s);
  free(buf);
  assert(fclose(fp) == 0);
  return s;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
