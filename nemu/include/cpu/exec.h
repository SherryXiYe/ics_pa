#ifndef __CPU_EXEC_H__
#define __CPU_EXEC_H__

#include "nemu.h"

//宏make_EHelpr(name)定义一个名字是exec_'name'的执行函数（定义一个执行阶段相关的helper函数），均有一个参数eip（指针类型，传入参数时记得传地址）
#define make_EHelper(name) void concat(exec_, name) (vaddr_t *eip)
typedef void (*EHelper) (vaddr_t *);

#include "cpu/decode.h"

//取指。取指针eip对应指令的从低位开始的len个字节，并更改（*eip）令eip指向len个字节之后
static inline uint32_t instr_fetch(vaddr_t *eip, int len) {
  uint32_t instr = vaddr_read(*eip, len);
#ifdef DEBUG
  uint8_t *p_instr = (void *)&instr;
  int i;
  for (i = 0; i < len; i ++) {
    decoding.p += sprintf(decoding.p, "%02x ", p_instr[i]);
  }
#endif
  (*eip) += len;      
  return instr;
}

void rtl_setcc(rtlreg_t*, uint8_t);

static inline const char* get_cc_name(int subcode) {
  static const char *cc_name[] = {
    "o", "no", "b", "nb",
    "e", "ne", "be", "nbe",
    "s", "ns", "p", "np",
    "l", "nl", "le", "nle"
  };
  return cc_name[subcode];
}

#ifdef DEBUG
#define print_asm(...) Assert(snprintf(decoding.assembly, 80, __VA_ARGS__) < 80, "buffer overflow!")
#else
#define print_asm(...)
#endif

#define suffix_char(width) ((width) == 4 ? 'l' : ((width) == 1 ? 'b' : ((width) == 2 ? 'w' : '?')))

#define print_asm_template1(instr) \
  print_asm(str(instr) "%c %s", suffix_char(id_dest->width), id_dest->str)

#define print_asm_template2(instr) \
  print_asm(str(instr) "%c %s,%s", suffix_char(id_dest->width), id_src->str, id_dest->str)

#define print_asm_template3(instr) \
  print_asm(str(instr) "%c %s,%s,%s", suffix_char(id_dest->width), id_src->str, id_src2->str, id_dest->str)

#endif
