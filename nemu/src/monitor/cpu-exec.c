#include "nemu.h"
#include "monitor/monitor.h"

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;

void exec_wrapper(bool);

/* Simulate how the CPU works. 模拟CPU的工作方式：不断执行指令 */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {   // 因此,再次运行程序需要输入q后重新运行NEMU
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  bool print_flag = n < MAX_INSTR_TO_PRINT;     //程序的指令数小于MAX_INSTR_TO_PRIN时才能打印

  for (; n > 0; n --) {
    /* Execute one instruction, including instruction fetch,
     * instruction decode, and the actual execution. */
    exec_wrapper(print_flag);         // 让CPU执行当前 %eip 指向的一条指令，然后更新%eip。 in nemu/src/cpu/exec/exec.c

#ifdef DEBUG
    /* TODO: check watchpoints here. */

#endif

#ifdef HAS_IOE
    extern void device_update();
    device_update();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
  //此处，一个客户程序运行完后，nemu_state 会变为 NEMU_STOP。
}
