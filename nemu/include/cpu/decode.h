#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include "common.h"

#include "rtl.h"

enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

#define OP_STR_SIZE 40

typedef struct {
  uint32_t type;
  int width;
  union {
    uint32_t reg;
    rtlreg_t addr;        // NEMU中，id_src,id_src2和id_dest中的访存地址addr和操作数内容val是RTL寄存器
    uint32_t imm;
    int32_t simm;
  };
  rtlreg_t val;
  char str[OP_STR_SIZE];
} Operand;

//记录一些全局译码信息供后续使用，包括操作数的类型、宽度、值等信息。
typedef struct {
  uint32_t opcode;    //当前执行指令的操作码
  vaddr_t seq_eip;    // sequential eip  顺序执行时（不跳转），%eip应该指向的地址
  bool is_operand_size_16;
  uint8_t ext_opcode;
  bool is_jmp;                //当前指令是不是跳转指令
  vaddr_t jmp_eip;            //跳转指令的目标地址（跳转后%eip应该指向的地址）
  Operand src, dest, src2;    //代表两个源操作数和一个目的操作数
#ifdef DEBUG
  char assembly[80];
  char asm_buf[128];
  char *p;
#endif
} DecodeInfo;

typedef union {
  struct {
    uint8_t R_M		:3;
    uint8_t reg		:3;
    uint8_t mod		:2;
  };
  struct {
    uint8_t dont_care	:3;
    uint8_t opcode		:3;
  };
  uint8_t val;
} ModR_M;

typedef union {
  struct {
    uint8_t base	:3;
    uint8_t index	:3;
    uint8_t ss		:2;
  };
  uint8_t val;
} SIB;

void load_addr(vaddr_t *, ModR_M *, Operand *);
void read_ModR_M(vaddr_t *, Operand *, bool, Operand *, bool);

void operand_write(Operand *, rtlreg_t *);

/* shared by all helper functions */
extern DecodeInfo decoding;

// 三个宏id_src、id_src2、id_dest用于方便地访问decoding结构的两个源操作数和一个目的操作数
#define id_src (&decoding.src)      
#define id_src2 (&decoding.src2)
#define id_dest (&decoding.dest)

//宏make_DopHelper(name)定义一个名字是decode_'name'的译码函数。这些译码函数会进一步分解成各种不同操作数的译码的组合，以实现操作数译码的解耦
#define make_DHelper(name) void concat(decode_, name) (vaddr_t *eip)
typedef void (*DHelper) (vaddr_t *);

make_DHelper(I2E);
make_DHelper(I2a);
make_DHelper(I2r);        
make_DHelper(SI2E);
make_DHelper(SI_E2G);
make_DHelper(I_E2G);
make_DHelper(I_G2E);
make_DHelper(I);
make_DHelper(r);
make_DHelper(E);
make_DHelper(gp7_E);
make_DHelper(test_I);
make_DHelper(SI);
make_DHelper(G2E);
make_DHelper(E2G);

make_DHelper(mov_I2r);
make_DHelper(mov_I2E);
make_DHelper(mov_G2E);
make_DHelper(mov_E2G);
make_DHelper(lea_M2G);

make_DHelper(gp2_1_E);
make_DHelper(gp2_cl2E);
make_DHelper(gp2_Ib2E);

make_DHelper(O2a);
make_DHelper(a2O);

make_DHelper(J);

make_DHelper(push_SI);

make_DHelper(in_I2a);
make_DHelper(in_dx2a);
make_DHelper(out_a2I);
make_DHelper(out_a2dx);

make_DHelper(lidt_a);

#endif
