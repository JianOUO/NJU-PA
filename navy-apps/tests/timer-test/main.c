#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>

// 系统调用接口
#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
  struct timeval start, current;
  long elapsed_us;
  int count = 0;

  // 获取起始时间
  gettimeofday(&start, NULL);
  printf("Timer test started at: %ld seconds, %ld microseconds\n", 
         start.tv_sec, start.tv_usec);

  while (1) {
    // 获取当前时间
    gettimeofday(&current, NULL);
    
    // 计算从起始时间到现在的微秒数
    elapsed_us = (current.tv_sec - start.tv_sec) * 1000000 + 
                  (current.tv_usec - start.tv_usec);
    
    // 每过 0.5 秒（500000 微秒）输出一次
    if (elapsed_us >= count * 500000) {
      count++;
      printf("[%d] Time elapsed: %ld.%06ld seconds\n",
             count,
             elapsed_us / 1000000,
             elapsed_us % 1000000);
    }  
  }

  return 0;
}

