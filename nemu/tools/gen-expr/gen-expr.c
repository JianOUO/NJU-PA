/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
// %s是一个格式化占位符，会被实际的数学表达式替换
// %%u中的%%是转义序列，在sprintf()中，%%表示输出一个%字符
// 所以 printf(\"%%u\", result) 实际输出的是 printf("%u", result)
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static int buf_len = 0;

static void gen_num() {
  if (buf_len > 65536 - 12) return;
  uint32_t val = 0;
  if (rand() % 2) {
    buf[buf_len++] = '-';
  }
  val = rand() % 0x100000000;
  while (val != 0) {
    buf[buf_len++] = val % 10 + '0';
    val /= 10;
  }
  buf[buf_len] = '\0';
}

static void gen_rand_op() {
    switch (rand() % 4) {
      case 0: {
        buf[buf_len++] = '+';
        break;
      }
      case 1: {
        buf[buf_len++] = '-';
        break;
      } 
      case 2: {
        buf[buf_len++] = '*';
        break;
      }
      default: {
        buf[buf_len++] = '/';
        break;
      }
    }
    buf[buf_len] = '\0';
}

static void gen_space() {
  int max_num = 5;
  if (buf_len > 65536 - max_num - 1) return;
  if (rand() % 2) {
    int num = rand() % max_num;
    for (int i = 0; i < num; i++) {
      buf[buf_len++] = ' ';
    }
    buf[buf_len] = '\0';
  }
}

static void gen_rand_expr() {
  buf[0] = '\0';
  switch (rand() % 3) {
    case 0: {
      if (buf_len >= 65535) break;
      gen_num(); 
      break;
    }
    case 1: {
      if (buf_len >= 65535) break;
      buf[buf_len++] = '(';
      buf[buf_len] = '\0';
      if (buf_len >= 65535) break;
      gen_space();
      if (buf_len >= 65535) break;
      gen_rand_expr();
      if (buf_len >= 65535) break;
      gen_space();
      if (buf_len >= 65535) break;
      buf[buf_len++] = ')';
      buf[buf_len] = '\0';
      break;
    }
    default: {
      if (buf_len >= 65535) break;
      gen_space();
      if (buf_len >= 65535) break;
      gen_rand_expr(); 
      if (buf_len >= 65535) break;
      gen_space();
      if (buf_len >= 65535) break;
      gen_rand_op();
      if (buf_len >= 65535) break;
      gen_space();
      if (buf_len >= 65535) break;
      gen_rand_expr(); 
      if (buf_len >= 65535) break;
      gen_space();
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  //获取当前时间戳
  int seed = time(0);
  // 设置随机数生成器种子，确保每次运行程序时生成不同的随机数序列
  // 如果不设置种子，rand() 函数会使用默认种子（通常是1），导致：
  // 每次运行程序生成的随机数序列都相同，无法产生真正的随机效果。
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    // 模式 "w"：以写入模式打开，如果文件不存在则创建，如果存在则清空内容
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
    // 执行系统命令编译C代码
    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    // 如果编译失败（ret != 0），跳过当前循环迭代
    if (ret != 0) continue;

    // popen() 启动子进程并建立管道,执行 /tmp/.expr 程序,捕获程序的标准输出
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
