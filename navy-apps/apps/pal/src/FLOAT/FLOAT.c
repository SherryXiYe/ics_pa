#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  return ((int64_t)a * (int64_t)b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  // return (a/b)<<16;
  assert(b != 0);
  FLOAT m_a = Fabs(a);
  FLOAT m_b = Fabs(b);
  FLOAT res = m_a / m_b;
  FLOAT remain = m_a % m_b;

  for (int i = 0; i < 16; i++) {
    remain = remain << 1;
    res = res << 1;
    if (remain >= m_b) {
      remain -= m_b;
      res++;
    }
  }
  if (((a ^ b) & 0x80000000) == 0x80000000) {
    res = -res;
  }
  return res;
}

union float_s {
    struct {
      uint32_t m : 23;
      uint32_t e : 8;
      uint32_t s : 1;
    };
    uint32_t value;
};

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   * “a”的位表示形式已在堆栈上。如何在不直接对其执行算术运算的情况下将其检索到另一个变量？
   */

  // assert(0);
  union float_s f;
  f.value = *((uint32_t*)(void*)&a);
  int real_e = f.e - 127;      //127为指数的偏移量！不能忘记减去
  FLOAT res = 0;
  if (real_e == 128)
    assert(0);
  int mov = 23- real_e - 16;
  if (mov >= 0)
    res = (f.m | (1 << 23)) >> mov;
  else
    res = (f.m | (1 << 23)) << (-mov);
  if(f.s!=0){
    res=-res;
  }
  return res;
}

FLOAT Fabs(FLOAT a) {
  if(isNeg(a)){
    return -a;
  }else{
    return a;
  }
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
