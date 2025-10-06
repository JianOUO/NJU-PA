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
static char buf1[65536] = {};
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
static int buf1_len = 0;

static void gen_num() {
  int negative = rand() % 2;
  uint32_t val = 0;
  if (negative) {
    if (buf_len > 65536 - 22) return; // (unsigned) + 负号 + 10位数字 + 终结符
    strcpy(buf + buf_len, "(unsigned)");
    buf_len += 10;
    buf[buf_len++] = '-';
    buf1[buf1_len++] = '-' ;
    val = rand() % 0x80000001;
  } else {
    if (buf_len > 65536 - 12) return; // 10位数字 + U + 终结符
    val = rand() % 0x100000000;
  }
  int decimal = rand() % 2;
  if (decimal) {
    buf_len += snprintf(buf + buf_len, 11, "%u", val);
    buf1_len += snprintf(buf1 + buf1_len, 11, "%u", val);
  } else {
    buf_len += snprintf(buf + buf_len, 11, "0x%08x", val);
    buf1_len += snprintf(buf1 + buf1_len, 11, "0x%08x", val);
  }
  if (!negative) buf[buf_len++] = 'U';
  buf[buf_len] = '\0';
  buf1[buf1_len] = '\0';
}

static void gen_rand_op() {
    if (buf_len > 65536 - 3) return;
    switch (rand() % 8) {
      case 0: {
        buf[buf_len++] = '+';
        buf1[buf1_len++] = '+';
        break;
      }
      case 1: {
        buf[buf_len++] = '-';
        buf1[buf1_len++] = '-';
        break;
      } 
      case 2: {
        buf[buf_len++] = '*';
        buf1[buf1_len++] = '*';
        break;
      }
      case 3: {
        buf[buf_len++] = '/';
        buf1[buf1_len++] = '/';
        break;
      }
      case 4: {
        strcpy(buf + buf_len, "==");
        strcpy(buf1 + buf1_len, "==");
        buf_len += 2;
        buf1_len += 2;
        break;
      } 
      case 5: {
        strcpy(buf + buf_len, "!=");
        strcpy(buf1 + buf1_len, "!=");
        buf_len += 2;
        buf1_len += 2;
        break;
      }
      case 6: {
        strcpy(buf + buf_len, "<=");
        strcpy(buf1 + buf1_len, "<=");
        buf_len += 2;
        buf1_len += 2;
        break;
      }
      default: {
        strcpy(buf + buf_len, "&&");
        strcpy(buf1 + buf1_len, "&&");
        buf_len += 2;
        buf1_len += 2;
        break;
      }
    }
    buf[buf_len] = '\0';
    buf1[buf1_len] = '\0';
}

static void gen_space() {
  if (buf_len > 65536 - 2) return;
  if (rand() % 2) {
    buf[buf_len++] = ' ';
    buf1[buf1_len++] = ' ';
    buf[buf_len] = '\0';
    buf1[buf1_len] = '\0';
  }
}

static void gen_rand_expr() {
  switch (rand() % 3) {
    case 0: {
      if (buf_len >= 65535) break;
      gen_num(); 
      break;
    }
    case 1: {
      if (buf_len >= 65535) break;
      buf[buf_len++] = '(';
      buf1[buf1_len++] = '(';
      buf[buf_len] = '\0';
      buf1[buf1_len] = '\0';
      if (buf_len >= 65535) break;
      gen_rand_expr();
      if (buf_len >= 65535) break;
      buf[buf_len++] = ')';
      buf1[buf1_len++] = ')';
      buf[buf_len] = '\0';
      buf1[buf1_len] = '\0';
      break;
    }
    default: {
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
    buf_len = 0;
    buf1_len = 0;
    gen_rand_expr();
    //printf("expr_len: %lu\n", strlen(buf));
    //printf("expr: %s\n", buf);
    sprintf(code_buf, code_format, buf);
    //printf("code_buf: %s\n", code_buf);
    // 模式 "w"：以写入模式打开，如果文件不存在则创建，如果存在则清空内容
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);
    // 执行系统命令编译C代码
    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr");
    // 如果编译失败（ret != 0），跳过当前循环迭代
    if (ret != 0) continue;

    // popen() 启动子进程并建立管道,执行 /tmp/.expr 程序,捕获程序的标准输出
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf1);
  }
  return 0;
}
