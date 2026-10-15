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

#include "common.h"
#include <isa.h>
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len) {
  if (isa_mmu_check(addr, len, MEM_TYPE_IFETCH) == MMU_TRANSLATE) {
    paddr_t pg_pa = isa_mmu_translate(addr, len, MEM_TYPE_IFETCH);
    //printf("pg_pa: 0x%x, ", pg_pa);
    paddr_t pa = pg_pa | (addr & 0xfff);
    word_t data = paddr_read(pa, len);
    //printf("va: 0x%x, len: %d, pa: 0x%x, data: 0x%x\n", addr, len, pa, data);
    return data;
  } else {
    word_t data = paddr_read(addr, len);
    //printf("data: 0x%x\n", data);
    return data;
  }
}

word_t vaddr_read(vaddr_t addr, int len) {
  if (isa_mmu_check(addr, len, MEM_TYPE_READ) == MMU_TRANSLATE) {
    paddr_t pg_pa = isa_mmu_translate(addr, len, MEM_TYPE_READ);
    paddr_t pa = pg_pa | (addr & 0xfff);
    return paddr_read(pa, len);
  } else {
    return paddr_read(addr, len);
  }
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
  if (isa_mmu_check(addr, len, MEM_TYPE_WRITE) == MMU_TRANSLATE) {
    paddr_t pg_pa = isa_mmu_translate(addr, len, MEM_TYPE_WRITE);
    paddr_t pa = pg_pa | (addr & 0xfff);
    paddr_write(pa, len, data);
  } else {
    paddr_write(addr, len, data); 
  }
}
