#include "cpu/exec.h"
//含有IO指令

void diff_test_skip_qemu();
void diff_test_skip_nemu();

make_EHelper(lidt) {
  // TODO();
  // t1=id_dest->val;  //data数组的地址
  // cpu.idtr.limit = vaddr_read(t1, 2);  // 读取data[0]为idtr.limit
  // cpu.idtr.base = vaddr_read(t1 + 2, 4);  // 读取data[1]、data[2]为idtr.base, 32bit
  t1=id_dest->val;//address of data array
  rtl_lm(&t0,&t1,2);//t0 = data[0], the limit length of IDT
  cpu.idtr.limit=t0;

  t1=id_dest->val+2;
  rtl_lm(&t0,&t1,4);//t0 = base address of IDT, 32bit
  cpu.idtr.base=t0;
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  TODO();

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  // TODO();
  id_dest->val = pio_read(id_src->val, id_dest->width);
  operand_write(id_dest, &id_dest->val);
  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  // TODO();
  pio_write(id_dest->val, id_dest->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
