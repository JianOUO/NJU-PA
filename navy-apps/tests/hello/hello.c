#include <unistd.h>
#include <stdio.h>

int main() {
  // 使用 write 系统调用向标准输出写入 "Hello World!\n"
  write(1, "Hello World!\n", 13);
  int i = 2;
  // j可能被外部因素改变,每次读取都要从内存读取，不要使用寄存器缓存
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }
  return 0;
}
