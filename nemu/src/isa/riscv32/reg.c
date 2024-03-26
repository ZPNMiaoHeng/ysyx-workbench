/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
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

// TODO: struct csrs: name，no
const char *csrs[] = {
  "mstatus",      // 0x300
  // "mie",          // 0x304
  "mepc",         // 0x341
  "mcause",        // 0x342
  "mtval",       // 0x343 
  "mip",          // 0x344
  "mtvec"         // 0x305
  // "mscratch"      // 0x310
};

void isa_reg_display() {
  int i, j;
  for(i = 0; i < 4; i++) {
    for(j = 0; j < 8; j++ ) {
      printf("x%-2d(%s):%#-8x\t", 8*i+j, regs[8*i+j], cpu.gpr[8*i+j]);
    }
    printf("\n");
  }
  for(j = 0; j < 6; j++) {
    printf("%s(%d):%#-8x\t", csrs[j], j, cpu.csr[j]);
  }
    printf("\n");
}

bool isa_reg_name(const char *s) {
  int i = 0;
  for(i=0; i<32; i++) {
    if(strcmp(regs[i], s) == 0) {
      return true;
    }
  }
  return false;
}

word_t isa_reg_str2val(const char *s, bool *success) {
  int i = 0;
  for(i=0; i<32; i++) {
    if(strcmp(regs[i], s) == 0) {
      success = (bool *) true;
      return cpu.gpr[i];
    }
  }

  success = (bool *) false;
  Assert(i < 32, "Reg name %s is error,please check it!", s);
  return 0;
}
