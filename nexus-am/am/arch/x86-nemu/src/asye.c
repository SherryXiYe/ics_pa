#include <am.h>
#include <x86.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;

void vecsys();
void vecnull();
void vecselftrap();     //内核自陷

// 根据_RegSet结构中的异常请求号irq设置事件ev.event（把异常封装为事件）
_RegSet* irq_handle(_RegSet *tf) {
  _RegSet *next = tf;
  if (H) {
    _Event ev;
    switch (tf->irq) {
      case 0x80: ev.event = _EVENT_SYSCALL; break;
      case 0x81: ev.event = _EVENT_TRAP; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = H(ev, tf);
    if (next == NULL) {
      next = tf;
    }
  }

  return next;
}

static GateDesc idt[NR_IRQ];

void _asye_init(_RegSet*(*h)(_Event, _RegSet*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vecselftrap, DPL_USER);
  
  set_idt(idt, sizeof(idt));    //在IDTR中设置idt的首地址和长度

  // register event handler
  H = h;
}

_RegSet *_make(_Area stack, void *entry, void *arg) {
  return NULL;
}

void _trap() {
  asm volatile("int $0x81");
}

int _istatus(int enable) {
  return 0;
}
