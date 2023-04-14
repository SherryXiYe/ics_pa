#include "cpu/exec.h"
//all-instr文件：所有执行函数的定义（make_Ehelper）
make_EHelper(mov);
make_EHelper(operand_size);
make_EHelper(inv);
make_EHelper(nemu_trap);

make_EHelper(push);
make_EHelper(pop);
make_EHelper(sub);
make_EHelper(call);
make_EHelper(xor);
make_EHelper(ret);