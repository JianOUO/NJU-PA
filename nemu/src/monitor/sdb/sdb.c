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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
WP *new_wp();
void free_wp(int number);
void wp_display();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  // static：在函数调用间保持值
  static char *line_read = NULL;

  // 释放之前分配的内存(readline会自动分配内存),避免内存泄漏
  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  // 正常输入 - 返回新分配的字符串
  // 空输入（直接按回车） - 返回空字符串
  // EOF（Ctrl+D） - 返回 NULL
  // 错误情况 - 返回 NULL
  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args) {
  int num = 1;
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    cpu_exec(num);
  } else {
    if (sscanf(arg, "%d", &num) != 1) {
      printf("si: sscanf failed\n");
      return 0;
    }
    cpu_exec(num);
  }
  return 0;
}

static int cmd_info(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("info: No SUBCMD\n");
    return 0;
  } else {
    if (*arg == 'r') {
      isa_reg_display();
    } else if (*arg == 'w') {
      wp_display();
    } else {
      printf("info: Unkowned SUBCMD: '%c'\n", *arg);
      return 0;
    }
  }
  return 0;
}

static int cmd_x(char *args) {
  char *arg = strtok(NULL, " ");
  int N;
  paddr_t EXPR;
  if (arg == NULL) {
    printf("x: no N\n");
    return 0;
  } else {
    if (sscanf(arg, "%d", &N) != 1) {
      printf("x: sscanf failed\n");
      return 0;
    }
  }
  arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("x: no EXPR\n");
    return 0;
  } else {
    bool success;
    EXPR = expr(arg, &success);
    printf("arg: %s\n", arg);
    printf("expr: %u\n", EXPR);
    if (!success)  {
      printf("x: Expr computing failed\n");
      return 0;
    }
  }
  uint32_t *byte = (uint32_t *)guest_to_host(EXPR);
  for (int i = 0; i < N; i++) {
    if (in_pmem(EXPR + 4 * i) && in_pmem(EXPR + 4 * i + 3)) {
      printf(FMT_PADDR ": ", EXPR + 4 * i);
      printf("%08x", *(byte + i));
      printf("\n");
    } else {
      if (in_pmem(EXPR + 4 * i)) {
        printf(FMT_PADDR ": ", EXPR + 4 * i);
        printf("%02x", *((uint8_t *)(byte + i)));
      } else break;
      for (int j = 1; j < 3; j++) {
        if (in_pmem(EXPR + 4 * i + j)) printf("%02x", *((uint8_t *)(byte + i) + j));
        else break;
      }
      printf("\n");
      break;
    }
  }
  return 0;
}

static int cmd_p(char *args) {
  if (args == NULL) {
    printf("p: no EXPR\n");
    return 0;
  } else {
    bool success;
    word_t val = expr(args, &success);
    if (success) {
      printf(FMT_WORD_U "\n", val);
      printf(FMT_WORD "\n", val);
    } else {
      printf("p: Expr computing failed\n");
    }
  }
  return 0;
}

static int cmd_w(char *args) {
  if (args == NULL) {
    printf("w: no EXPR\n");
  } else {
    WP *wp = new_wp(args);
    if (wp == NULL) { 
      printf("no free watchpoint or Expr compute failed\n");
      return 0;
    }
    printf("new watchpoint has been created: No.%d Expr \"%s\"\n", wp->NO, wp->expr);
  }
  return 0;
}

static int cmd_d(char *args) {
  char *arg = strtok(NULL, " ");
  if (arg == NULL) {
    printf("d: no watchpoint number\n");
    return 0;
  }
  int number;
  if (sscanf(arg, "%d", &number) != 1) {
    printf("d: sscanf failed\n");
    return 0;
  }
  free_wp(number);
  return 0;
}

static int cmd_t(char *args) {
  FILE* fp = fopen("input", "r");
  if (fp == NULL) {
    printf("Failed to open input file\n");
    return 0;
  }
  
  char line[65596];
  int total_tests = 0;
  int passed_tests = 0;
  int failed_tests = 0;
  
  printf("Running expression tests...\n");
  printf("========================================\n");
  
  while (fgets(line, sizeof(line), fp)) {
    // 跳过空行
    if (strlen(line) <= 1) continue;
    printf("line: %s\n", line);
    // 解析行：result expr
    word_t expected_result;
    char expr_str[65596];
    
    // 使用 sscanf 解析格式：数字 表达式
    // %[^\n]: 读取所有字符直到换行符
    int parsed = sscanf(line, "%u %[^\n]", &expected_result, expr_str);
    printf("expr: %s\n",expr_str);
    if (parsed != 2) {
      printf("Warning: Failed to parse line: %s", line);
      continue;
    }
    
    total_tests++;
    
    // 调用 expr 函数求值
    bool success;
    word_t actual_result = expr(expr_str, &success);
    
    if (!success) {
      printf("FAIL: Expression evaluation failed: %s\n", expr_str);
      failed_tests++;
      continue;
    }
    
    // 比较结果
    if (actual_result == expected_result) {
      passed_tests++;
    } else {
      printf("FAIL: %s = %u (expected %u)\n", expr_str, actual_result, expected_result);
      failed_tests++;
    }
  }
  
  fclose(fp);
  
  // 输出统计信息
  printf("========================================\n");
  printf("Test Results:\n");
  printf("Total:  %d\n", total_tests);
  printf("Passed: %d\n", passed_tests);
  printf("Failed: %d\n", failed_tests);
  
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Steo execution. Usage: si [N]", cmd_si},
  { "info", "Print the program status. Usage: info SUBCMD", cmd_info},
  { "x", "Scan memory. Usage: x N EXPR", cmd_x},
  { "p", "Expression evaluation. Usage: p EXPR", cmd_p},
  { "w", "Set watch point. Usage: w EXPR", cmd_w},
  { "d", "Delete watch point. Usage: d N", cmd_d},
  { "t", "Test expr", cmd_t},
  /* TODO: Add more commands */
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }
/*
清空 SDL 事件队列，丢弃所有待处理的 SDL 事件。
执行流程：
1. rl_gets() 读取用户输入（从终端读取，不是从SDL）
   └─> 用户输入 "si" 命令

2. 清空 SDL 事件队列
   └─> 丢弃在等待输入期间累积的所有 SDL 事件
       └─> 这些事件可能是：
           - 窗口焦点变化
           - 鼠标移动
           - 按键（但这些是给调试器的，不是给模拟程序的）

3. 处理命令
   └─> 执行 "si"（单步执行）
       └─> 执行一条指令
           └─> device_update() 被调用
               └─> 此时 SDL 事件队列是干净的
                   └─> 只处理新的事件
*/
#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
