#include "nemu.h"
#include"device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];      // 模拟内存

// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(va) --/ \--- PTX(va) --/\------ OFF(va) ------/
#define PDX(va)     (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)

// Address in page table or page directory entry 获取高20位
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)

/* Memory accessing interfaces */

// len表示取从低位开始的多少个字节，len一般取1~4。
uint32_t paddr_read(paddr_t addr, int len) {
  // return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  int no = is_mmio(addr);
  if (no == -1)
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  return mmio_read(addr, len, no);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  // memcpy(guest_to_host(addr), &data, len);
  int no = is_mmio(addr);
  if (no == -1)
    memcpy(guest_to_host(addr), &data, len);
  else
    mmio_write(addr, len, data, no);
}

paddr_t page_translate(vaddr_t addr, bool dirty) {
  if(cpu.cr0.paging && cpu.cr0.protect_enable){   
    printf("page translate: vaddr:0x%x\n",addr);
    PDE* pgdir = (PDE*)PTE_ADDR(cpu.cr3.val);   // 页目录表基址
    PDE pde;
    pde.val = paddr_read((paddr_t)(pgdir+PDX(addr)), 4);  //页目录项
    assert(pde.present);
    pde.accessed = true;

    PTE* ptep = (PTE*)PTE_ADDR(pde.val);    // 二级页表基址
    PTE pte;
    pte.val = paddr_read((paddr_t)(ptep+PTX(addr)), 4);   //页表项
    assert(pte.present);
    pte.accessed = true;
    pte.dirty = dirty;    // 设置dirty位

    paddr_t paddr = PTE_ADDR(pte.val) | OFF(addr);
    return paddr;
  }
  return addr;    // (cpu.cr0.val >> 31 != 1)
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  // return paddr_read(addr, len);
  uint32_t res = 0;
  if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1)){    // 跨页，分别读取
    int len1=0x1000-OFF(addr);  
    int len2=len-len1;
    paddr_t paddr1=page_translate(addr,true);
    paddr_t paddr2=page_translate(addr+len1,true);
    
    uint32_t low=paddr_read(paddr1,len1);
    uint32_t high=paddr_read(paddr2,len2);

    res = high<<(8*len1)|low;
  }else{      // 不跨页，直接通过page_translate获取物理地址再读取
    paddr_t paddr = page_translate(addr, false);
    res = paddr_read(paddr, len);
  }
  return res;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  // paddr_write(addr, len, data);
  if (PTE_ADDR(addr) != PTE_ADDR(addr + len - 1)){    // 跨页，先退出
    // assert(0);
    int len1=0x1000-OFF(addr);
    int len2=len-len1;
    paddr_t paddr1=page_translate(addr,false);
    paddr_t paddr2=page_translate(addr+len1,false);

    uint32_t low=data & (~0u >> ((4 - len1) << 3));
    uint32_t high=data>>((4-len2)*8);

    paddr_write(paddr1, len1, low);
    paddr_write(paddr2, len2, high);
  }else{
    paddr_t paddr = page_translate(addr, true);
    paddr_write(paddr, len, data);
  }
}

