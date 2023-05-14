#ifndef __COMMON_H__
#define __COMMON_H__

#define DEBUG
#define DIFF_TEST

/* You will define this macro in PA2 */
#define HAS_IOE

#include "debug.h"
#include "macro.h"

#include <stdint.h>
#include <assert.h>
#include <string.h>

typedef uint8_t bool;

//在NEMU中，统一使用 rtlreg_t来定义RTL寄存器。RTL寄存器是RTL指令专门使用的寄存器。NEMU使用RTL(寄存器传输语言)来描述x86指令的行为。
typedef uint32_t rtlreg_t;

typedef uint32_t paddr_t;
typedef uint32_t vaddr_t;

typedef uint16_t ioaddr_t;

#define false 0
#define true 1

#endif
