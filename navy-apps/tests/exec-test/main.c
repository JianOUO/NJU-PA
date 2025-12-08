#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
这是一个测试 exec 系统调用的程序，会递归执行自身，每次传递一个递增的数字参数，用于验证 exec 系统调用是否正常工作。
*/

// argc：命令行参数个数（包括程序名）
// argv：命令行参数数组
// argv[0]：程序名(可执行文件路径), argv[1]：第一个参数
int main(int argc, char *argv[]) {
  // argc >= 2：检查是否有命令行参数
  // 如果有参数：n = atoi(argv[1]): 将第一个参数转换为整数
  // 如果没有参数：n = 1
  int n = (argc >= 2 ? atoi(argv[1]) : 1);
  printf("%s: argv[1] = %d\n", argv[0], n);

  char buf[16];
  sprintf(buf, "%d", n + 1);
  // execl：执行程序（列表形式参数）
  // argv[0]：第一个参数，可执行文件路径
  // argv[0]：第二个参数，程序名
  // buf：第三个参数，新的数字（n + 1）
  // NULL：参数列表结束标记
  // 递归执行自身, 成功时不会返回
  execl(argv[0], argv[0], buf, NULL);
  return 0;
}
