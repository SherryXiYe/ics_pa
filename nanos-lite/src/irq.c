#include "common.h"

extern _RegSet* do_syscall(_RegSet *r);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:
      return do_syscall(r);
    case _EVENT_TRAP:
      printf("EVENT_TRAP: self trap\n");
      return NULL;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);   //_asye_init函数定义在nexus-am/am/arch/x86-nemu/src/asye.c
}
