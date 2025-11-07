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
#include <capstone/capstone.h>
#include <common.h>

static size_t (*cs_disasm_dl)(csh handle, const uint8_t *code,
    size_t code_size, uint64_t address, size_t count, cs_insn **insn);
static void (*cs_free_dl)(cs_insn *insn, size_t count);

static csh handle;

void init_disasm() {
  // 运行时加载 Capstone 共享库
  void *dl_handle;
  // RTLD_LAZY：延迟绑定，函数第一次被调用时才解析
  dl_handle = dlopen("tools/capstone/repo/libcapstone.so.5", RTLD_LAZY);
  assert(dl_handle);

  // 获取 cs_open 函数指针
  // cs_open 函数的作用：初始化 Capstone 反汇编器，指定架构和模式
  cs_err (*cs_open_dl)(cs_arch arch, cs_mode mode, csh *handle) = NULL;
  // dlsym(dl_handle, "cs_open")：从库中查找名为 "cs_open" 的函数
  cs_open_dl = dlsym(dl_handle, "cs_open");
  assert(cs_open_dl);
  // cs_disasm_dl：反汇编机器码
  cs_disasm_dl = dlsym(dl_handle, "cs_disasm");
  assert(cs_disasm_dl);

  // cs_free_dl：释放反汇编结果占用的内存
  cs_free_dl = dlsym(dl_handle, "cs_free");
  assert(cs_free_dl);

  cs_arch arch = MUXDEF(CONFIG_ISA_x86,      CS_ARCH_X86,
                   MUXDEF(CONFIG_ISA_mips32, CS_ARCH_MIPS,
                   MUXDEF(CONFIG_ISA_riscv,  CS_ARCH_RISCV,
                   MUXDEF(CONFIG_ISA_loongarch32r,  CS_ARCH_LOONGARCH, -1))));
  cs_mode mode = MUXDEF(CONFIG_ISA_x86,      CS_MODE_32,
                   MUXDEF(CONFIG_ISA_mips32, CS_MODE_MIPS32,
                   MUXDEF(CONFIG_ISA_riscv,  MUXDEF(CONFIG_ISA64, CS_MODE_RISCV64, CS_MODE_RISCV32) | CS_MODE_RISCVC, // | CS_MODE_RISCVC：按位或，表示同时支持压缩指令集（RVC）
                   MUXDEF(CONFIG_ISA_loongarch32r,  CS_MODE_LOONGARCH32, -1))));
	int ret = cs_open_dl(arch, mode, &handle);
  assert(ret == CS_ERR_OK);

#ifdef CONFIG_ISA_x86
  cs_err (*cs_option_dl)(csh handle, cs_opt_type type, size_t value) = NULL;
  cs_option_dl = dlsym(dl_handle, "cs_option");
  assert(cs_option_dl);

  // 设置反汇编器使用 AT&T 语法
  ret = cs_option_dl(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
  assert(ret == CS_ERR_OK);
#endif
}

/*
str: 输出缓冲区，用于存储反汇编结果
size: 缓冲区大小
pc: 指令的地址（程序计数器）
code: 指向机器码的字节数组
nbyte: 机器码的字节数
*/
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
/*
Capstone 定义（简化版）
typedef struct cs_insn {
    unsigned int id;        // 指令 ID
    uint64_t address;       // 指令地址
    uint16_t size;          // 指令大小（字节）
    uint8_t bytes[16];      // 机器码字节
    char mnemonic[32];      // 助记符（如 "add"）
    char op_str[160];       // 操作数字符串（如 "a0, a1, a2"）
    ... 其他字段
} cs_insn;
*/
	cs_insn *insn;
  // 反汇编机器码，将结果存储在 insn 中，返回值：成功反汇编的指令数量
	size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
  // 确保恰好反汇编出 1 条指令
  assert(count == 1);
  // 将指令助记符写入缓冲区
  int ret = snprintf(str, size, "%s", insn->mnemonic);
  // 如果操作数字符串不为空，则将操作数字符串写入缓冲区
  if (insn->op_str[0] != '\0') {
    // str + ret：从助记符结尾开始写入
    // size - ret：剩余缓冲区大小
    // "\t%s"：制表符 + 操作数字符串
    snprintf(str + ret, size - ret, "\t%s", insn->op_str);
  }
  // Capstone 在 cs_disasm_dl 中动态分配了 insn 结构体，需要手动释放
  cs_free_dl(insn, count);
}
