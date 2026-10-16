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

#include "isa-def.h"
#include <isa.h>

#ifdef CONFIG_ETRACE
void etrace(word_t mcause, word_t mstatus, word_t mepc);
#endif

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.csr[MCAUSE] = NO;
  cpu.csr[MEPC] = epc;
  if (cpu.csr[MSTATUS] & MIE) {
    cpu.csr[MSTATUS] |= MPIE;
  } else {
    cpu.csr[MSTATUS] &= (~MPIE);
  }
  cpu.csr[MSTATUS] &= (~MIE);
  #ifdef CONFIG_ETRACE
  etrace(cpu.csr[MCAUSE], cpu.csr[MSTATUS], cpu.csr[MEPC]);
  #endif
  return cpu.csr[MTVEC];
}

word_t isa_query_intr() {
  if (cpu.csr[MSTATUS] & MIE && cpu.INTR) {
    cpu.INTR = false;
    return IRQ_TIMER; 
  }
  return INTR_EMPTY;
}
