#ifndef __REG_H__
#define __REG_H__

#include "common.h"

enum { R_EAX, R_ECX, R_EDX, R_EBX, R_ESP, R_EBP, R_ESI, R_EDI };
enum { R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI };
enum { R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH };

/* TODO: Re-organize the `CPU_state' structure to match the register
 * encoding scheme in i386 instruction format. For example, if we
 * access cpu.gpr[3]._16, we will get the `bx' register; if we access
 * cpu.gpr[1]._8[1], we will get the 'ch' register. Hint: Use `union'.
 * For more details about the register encoding scheme, see i386 manual.
 */

typedef struct {
  union
  {
    union {
      uint32_t _32;
      uint16_t _16;
      uint8_t _8[2];
    } gpr[8];

    /* Do NOT change the order of the GPRs' definitions. */

    /* In NEMU, rtlreg_t is exactly uint32_t. This makes RTL instructions
    * in PA2 able to directly access these registers.
    */
    struct {
      rtlreg_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
    };
  };
  vaddr_t eip;

} CPU_state;

extern CPU_state cpu;

static inline int check_reg_index(int index) {
  assert(index >= 0 && index < 8);
  return index;
}

#define reg_l(index) (cpu.gpr[check_reg_index(index)]._32)    // 8个32位寄存器
#define reg_w(index) (cpu.gpr[check_reg_index(index)]._16)    // 8个16位寄存器
#define reg_b(index) (cpu.gpr[check_reg_index(index) & 0x3]._8[index >> 2])
/*
只有AL,DL,CL,BL,AH,DH,CH,BH 8个8位寄存器，应该是：依次的index为0~7，&0x3分别为0，1，2，3，0，1，2，3；index>>2分别为0，0，0，0，1，1，1，1
*/

extern const char* regsl[];   // 8个32位寄存器的名字的数组
extern const char* regsw[];   // 8个16位寄存器的名字的数组
extern const char* regsb[];   // 8个8位寄存器的名字的数组

// index为寄存器编号，width为寄存器的字节数。返回寄存器的名字
static inline const char* reg_name(int index, int width) {
  assert(index >= 0 && index < 8);
  switch (width) {
    case 4: return regsl[index];
    case 1: return regsb[index];
    case 2: return regsw[index];
    default: assert(0);
  }
}

static inline bool get_reg_value(char* reg_name, uint32_t* value){
  if(strcmp(reg_name,"eip")==0){
    *value = cpu.eip;
    return true;
  }
  for(int i=0;i<8;i++){
    if(strcmp(reg_name,regsl[i]==0))
      *value = reg_l(i);
      return true;
  }
  for(int i=0;i<8;i++){
    if(strcmp(reg_name,regsw[i]==0))
      *value = reg_w(i);
      return true;
  }
  for(int i=0;i<8;i++){
    if(strcmp(reg_name,regsb[i]==0))
      *value = reg_b(i);
      return true;
  }
  return false;
}

#endif
