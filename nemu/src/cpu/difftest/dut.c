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

#include <dlfcn.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <utils.h>
#include <difftest-def.h>

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

#ifdef CONFIG_DIFFTEST

// 由于实现差异，ref和dut的相同指令可能生成不同的状态, 为了避免状态不一致产生的报错，
// 将is_skip_ref为true，表示让ref跳过执行这些指令，直接将dut的状态同步到ref
static bool is_skip_ref = false;
// 由于QEMU指令打包的特性，QEMU可能一次执行多条指令
// 这时我们跳过检查，直到NEMU的PC追上QEMU的PC，再继续检查
// skip_dut_nr_inst记录需要跳过检查的指令数
static int skip_dut_nr_inst = 0;

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
  is_skip_ref = true;
  // If such an instruction is one of the instruction packing in QEMU
  // (see below), we end the process of catching up with QEMU's pc to
  // keep the consistent behavior in our best.
  // Note that this is still not perfect: if the packed instructions
  // already write some memory, and the incoming instruction in NEMU
  // will load that memory, we will encounter false negative. But such
  // situation is infrequent.

  // 如果正在追赶QEMU,现在遇到需要跳过的指令(如ebreak),就不需要再追赶了,后续直接同步状态
  skip_dut_nr_inst = 0;
}

// this is used to deal with instruction packing in QEMU.
// Sometimes letting QEMU step once will execute multiple instructions.
// We should skip checking until NEMU's pc catches up with QEMU's pc.
// The semantic is
//   Let REF run `nr_ref` instructions first.
//   We expect that DUT will catch up with REF within `nr_dut` instructions.
void difftest_skip_dut(int nr_ref, int nr_dut) {
  skip_dut_nr_inst += nr_dut;

  while (nr_ref -- > 0) {
    ref_difftest_exec(1);
  }
}

void init_difftest(char *ref_so_file, long img_size, int port) {
  assert(ref_so_file != NULL);

  void *handle;
  // 动态加载参考实现的共享库文件
  // RTLD_LAZY：延迟绑定模式，符号在第一次使用时才解析
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  //使用 dlsym 从动态库中查找并绑定函数
  ref_difftest_memcpy = dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  void (*ref_difftest_init)(int) = dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);

  // 初始化参考实现
  ref_difftest_init(port);
  // 同步初始内存：将客户机程序镜像复制到参考实现的内存
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  // 同步初始寄存器状态：将 NEMU 的寄存器状态复制到参考实现
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}

static void checkregs(CPU_state *ref, vaddr_t pc) {
  // 对比寄存器状态是isa相关的，具体实现在/src/isa/$(GUEST_ISA)/difftest/dut.c中
  // 如果寄存器状态不一致，终止NEMU，并显示寄存器状态
  if (!isa_difftest_checkregs(ref, pc)) {
    nemu_state.state = NEMU_ABORT;
    nemu_state.halt_pc = pc;
    isa_reg_display();
  }
}

// 每执行一条指令，在cpu-exec.c:trace_and_difftest函数中调用一次difftest_step
// 其中pc指向当前指令,npc指向下一条指令
void difftest_step(vaddr_t pc, vaddr_t npc) {
  CPU_state ref_r;
  // 需要跳过检查的指令数大于0，dut正在追赶ref的PC
  if (skip_dut_nr_inst > 0) {
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT); // 复制ref的寄存器状态
    if (ref_r.pc == npc) { // 追赶成功，skip_dut_nr_inst清零,对比寄存器状态
      skip_dut_nr_inst = 0;
      checkregs(&ref_r, npc);
      return;
    }
    skip_dut_nr_inst --; // 还未追上，跳过检查的指令数减1
    if (skip_dut_nr_inst == 0) // 追赶失败，报错并终止
      panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
    return;
  }

  if (is_skip_ref) { // 跳过检查，直接复制dut的寄存器状态到ref
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    is_skip_ref = false;
    return;
  }
  // ref执行一条指令
  ref_difftest_exec(1);
  // 复制ref的寄存器状态到dut
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
  // 对比寄存器状态
  checkregs(&ref_r, pc);
}
#else
void init_difftest(char *ref_so_file, long img_size, int port) { }
#endif
