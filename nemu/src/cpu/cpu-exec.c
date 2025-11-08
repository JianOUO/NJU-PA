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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
// 全局变量，记录执行的总指令数
uint64_t g_nr_guest_inst = 0;
// 指令执行的总时间，用主机时间，即电脑实际花费的时间计算
/*
为什么只统计主机时间？
NEMU 是指令级模拟器，没有时钟周期模拟：
├─ 不模拟 CPU 的时钟频率
├─ 不模拟指令的执行周期数
├─ 只关心指令的功能正确性
└─ 不关心时序
所以：
├─ 没有客机的"虚拟时间"
├─ 只有主机的真实时间
└─ 用于评估模拟器性能

为什么用微秒？
单步执行：可能只需要几十微秒
短程序：可能几百微秒
长程序：可能几秒到几分钟
用微秒：
精度够高（可以测量单步）
范围够大（uint64_t 可表示很大的值）
整数运算（避免浮点误差）
*/
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

void device_update();
void wp_scan();

// 在每条指令执行完毕后被调用
static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  // IRTACE_COND在nemu/Makefile通过-D编译选项定义
  // 将指令的反汇编结果(存储在_this->logbuf中)写入日志文件
  if (ITRACE_COND) { log_write("%s\n", _this->logbuf); }
#endif
  // g_print_step在cpu_exec函数中设置,表示执行的指令数是否小于MAX_INST_TO_PRINT
  // 如果执行指令数 < MAX_INST_TO_PRINT，将指令输出到屏幕，主要用于单步调试
  // 当连续执行很多指令，不输出到屏幕，只写日志文件
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  // 与参考实现对比状态,如果不一致，报错并终止
  // _this->pc指向当前指令，dnpc指向下一条动态指令
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
  // 监视点扫描
  IFDEF(CONFIG_WATCHPOINT, wp_scan());
}

// 让CPU执行当前PC指向的一条指令, 然后更新PC
static void exec_once(Decode *s, vaddr_t pc) {
  // s->pc在后续执行与PC相关的指令时表示当前的指令地址
  s->pc = pc;
  // s->snpc在后续阶段用于取指令，并且会被更新指向静态下一条指令
  s->snpc = pc;
  isa_exec_once(s);
  // 指令执行完毕，s->dnpc和cpu.pc指向动态下一条指令
  cpu.pc = s->dnpc;
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  //  向 logbuf 缓冲区写入 PC 地址
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  // ilen为当前指令的长度
  int ilen = s->snpc - s->pc;
  int i;
  // inst为当前指令的机器码
  uint8_t *inst = (uint8_t *)&s->isa.inst;
#ifdef CONFIG_ISA_x86
  for (i = 0; i < ilen; i ++) {
#else
  // 小端序，需要从高地址往低地址打印，方便阅读
  for (i = ilen - 1; i >= 0; i --) {
#endif
    // 打印机器码，每个字节占2个字符，不足2位前面补0
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  // 计算需要填充的空格数，使汇编指令对齐
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  // 填充空格
  memset(p, ' ', space_len);
  p += space_len;
  // 写入汇编指令
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst, ilen);
  /*
  示例：
  0x80000000: 97 02 00 00  auipc	t0, 0
  0x80000004: 23 88 02 00  sb	zero, 16(t0)
  0x80000008: 03 c5 02 01  lbu	a0, 16(t0)
  0x8000000c: 73 00 10 00  ebreak
  */
#endif
}

// 模拟了CPU的工作方式: 不断执行指令
static void execute(uint64_t n) {
  Decode s;
  for (;n > 0; n --) {
    exec_once(&s, cpu.pc);
    // 执行总指令数加1
    g_nr_guest_inst ++;
    // 可选指令追踪，差分验证，监视点扫描
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    // 更新模拟设备的状态
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic() {
  /*
  Native	在 Linux 上运行	 支持 setlocale 和 %'
  AM	    裸机环境	       没有 C 标准库完整支持
  */
  // 如果不是 AM（Abstract Machine）目标平台，设置数字本地化，使千位分隔符生效
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
  // 如果不是 AM 目标平台，使用千位分隔符
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  // 显示指令执行所花费的主机时间
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  // 显示总指令数
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  // 如果主机时间大于0，计算模拟频率
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT: case NEMU_QUIT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }
  // get_time()返回从NEMU启动到现在的微秒数
  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  // 累计总执行时间
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;
    /*
    if (state == NEMU_ABORT)
      输出："ABORT" (红色)
    else if (state == NEMU_END)
      if (halt_ret == 0)
        输出："HIT GOOD TRAP" (绿色)
      else
        输出："HIT BAD TRAP" (红色)
    */
    case NEMU_END: case NEMU_ABORT:
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :
            ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}
