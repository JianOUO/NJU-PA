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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const char *csrs[] = {
  "mstatus", "mtvec", "mepc", "mcause"
};

void isa_reg_display() {
  for (int i = 0; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    printf("%-3s = " FMT_WORD " " 
    MUXDEF(CONFIG_ISA64, "%20" PRId64, "%10" PRId32) " "
    MUXDEF(CONFIG_ISA64, "%20" PRIu64, "%10" PRIu32) "\n", 
    reg_name(i), gpr(i), gpr(i), gpr(i));
  }
  
    printf("%-7s = " FMT_WORD " " 
    MUXDEF(CONFIG_ISA64, "%20" PRId64, "%10" PRId32) " "
    MUXDEF(CONFIG_ISA64, "%20" PRIu64, "%10" PRIu32) "\n", 
    csr_name(MSTATUS), csr(MSTATUS), csr(MSTATUS), csr(MSTATUS));

  for (int i = 0; i < CSR_LEN; i++) {
    printf("%-7s = " FMT_WORD " " 
    MUXDEF(CONFIG_ISA64, "%20" PRId64, "%10" PRId32) " "
    MUXDEF(CONFIG_ISA64, "%20" PRIu64, "%10" PRIu32) "\n", 
    csr_name(i), csr(i), csr(i), csr(i));
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  // $pc
  if (strcmp(s, "$pc") == 0) {
    *success = true;
    return cpu.pc;
  }
  // $0 or $zero
  if (strcmp(s, "$0") == 0 || strcmp(s, "$zero") == 0) {
    *success = true;
    return gpr(0);
  }
  for (int i = 1; i < MUXDEF(CONFIG_RVE, 16, 32); i++) {
    if (strcmp(s + 1, regs[i]) == 0) {
      *success = true;
      return gpr(i);
    }
  }
  *success = false;
  return 0;
}
