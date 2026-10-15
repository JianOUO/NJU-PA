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
#include "isa-def.h"
#include "../local-include/reg.h"
#include <isa.h>
#include <memory/vaddr.h>
#include <memory/paddr.h>
#include <stdint.h>

typedef uintptr_t PTE;
#define SATP2PPN(satp) ((satp) & 0x003fffff)
#define VPN1(va) ((va) >> 22)
#define VPN0(va) (((va) << 10) >> 22)
#define OFFSET(va) ((va) & 0xfff)
#define PTE_V 0x01
#define PTE2PPN(pte) ((pte) >> 10)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  //return MEM_RET_FAIL;
  //printf("satp: 0x%x, ", csr(SATP));
  paddr_t lv0_pg_pa = SATP2PPN(csr(SATP)) << 12;
  PTE *lv1_pg = (PTE *)guest_to_host(lv0_pg_pa);
  uintptr_t vpn1 = VPN1(vaddr);
  PTE lv1_pte = *(lv1_pg + vpn1);
  assert(lv1_pte & PTE_V);

  paddr_t lv1_pg_pa = PTE2PPN(lv1_pte) << 12;
  PTE *lv0_pg = (PTE *)guest_to_host(lv1_pg_pa);
  uintptr_t vpn0 = VPN0(vaddr);
  PTE lv0_pte = *(lv0_pg + vpn0);
  assert(lv0_pte & PTE_V);

  paddr_t pg_pa = (PTE2PPN(lv0_pte) << 12);
  return pg_pa | MEM_RET_OK;
}

int isa_mmu_check(vaddr_t vaddr, int len, int type) {
  if (vaddr >= 0x80000000) return MMU_DIRECT;
  else return MMU_TRANSLATE;
}