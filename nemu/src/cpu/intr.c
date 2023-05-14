#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  // TODO();
  rtl_push(&cpu.eflags);    //将eflags、cs、eip入栈
  rtl_push(&cpu.CS);
  rtl_push(&ret_addr);

  //根据x86.h中GateDesc的结构获取跳转地址
  assert(NO*sizeof(GateDesc)<=cpu.idtr.limit);
  vaddr_t gate_addr=cpu.idtr.base+NO*sizeof(GateDesc);
  uint32_t off_15_0=vaddr_read(gate_addr,2);      //低16位offset
  uint32_t off_32_16=vaddr_read(gate_addr+sizeof(GateDesc)-2,2);    //高16位offset
  uint32_t jmp_addr=(off_32_16<<16)|(off_15_0 & 0xffff);
  // Log("jmp_addr: 0x%x",jmp_addr);
  decoding.jmp_eip=jmp_addr;
  decoding.is_jmp=1;
}

void dev_raise_intr() {
}
