#include <stdio.h>
#include <NDL.h>
#include <stdint.h>

int main() {
  // 初始化 NDL（这会记录起始时间）
  NDL_Init(0);
  
  uint32_t start_ms, current_ms;
  uint32_t elapsed_ms;
  int count = 0;

  // 获取起始时间（NDL_Init 后立即获取）
  start_ms = NDL_GetTicks();
  printf("Timer test started at: %u milliseconds\n", start_ms);

  while (1) {
    // 获取当前时间（毫秒）
    current_ms = NDL_GetTicks();
    
    // 计算从起始时间到现在的毫秒数
    elapsed_ms = current_ms - start_ms;
    
    // 每过 0.5 秒（500 毫秒）输出一次
    if (elapsed_ms >= count * 500) {
      count++;
      printf("[%d] Time elapsed: %u.%03u seconds\n",
             count,
             elapsed_ms / 1000,
             elapsed_ms % 1000);
    }
    
  }

  NDL_Quit();
  return 0;
}

