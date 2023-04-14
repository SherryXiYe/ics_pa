#include "cpu/exec.h"
//实现16bit与32bit操作数的切换（指令的0x66前缀）

make_EHelper(real);

make_EHelper(operand_size) {
  decoding.is_operand_size_16 = true;
  exec_real(eip);
  decoding.is_operand_size_16 = false;
}
