// arch/riscv/kernel/vm.c
#include "../include/defs.h"
#include "string.h"
#include "mm.h"
#include "vm.h"
#include "printk.h"
/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void setup_vm(void)
{
    /*
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    */
    // 低地址映射
    unsigned long index = (PHY_START >> VPN2OFFSET) & MUSK;
    early_pgtbl[index] = ((PHY_START >> VPN2OFFSET) & 0x3ffffff) << 28;
    early_pgtbl[index] |= (VALID | READABLE | WRITABLE | EXECUTABLE);

    // 高地址映射
    index = ((PHY_START + PA2VA_OFFSET) >> VPN2OFFSET) & MUSK;
    early_pgtbl[index] = ((PHY_START >> VPN2OFFSET) & 0x3ffffff) << 28;
    early_pgtbl[index] |= (VALID | READABLE | WRITABLE | EXECUTABLE);
}

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

/* 创建多级页表映射关系 */
void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm)
{
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小, represents bits of current slice
    perm 为映射的读写权限

    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    // 存储已经完成映射的页面的总大小
    int mapped = 0;
    while (mapped < sz)
    {
        unsigned long *pglevel2 = pgtbl;
        unsigned long vpn2 = (va >> VPN2OFFSET) & MUSK;
        // 如果本级页表不存在，那么分配一个新的物理页
        if ((pglevel2[vpn2] & VALID) == 0)
        {
            unsigned long pglevel1 = (unsigned long)(kalloc());
            pglevel2[vpn2] = ((((pglevel1 - PA2VA_OFFSET) >> 12) << 10) | VALID);
        }
        unsigned long *pglevel1 = (unsigned long *)(((pglevel2[vpn2] >> 10) << 12) + PA2VA_OFFSET);
        unsigned long vpn1 = (va >> VPN1OFFSET) & MUSK;
        // 如果本级页表不存在，那么分配一个新的物理页
        if ((pglevel1[vpn1] & VALID) == 0)
        {
            unsigned long pglevel0 = (unsigned long)(kalloc());
            pglevel1[vpn1] = ((((pglevel0 - PA2VA_OFFSET) >> 12) << 10) | VALID);
        }
        unsigned long *pglevel0 = (unsigned long *)(((pglevel1[vpn1] >> 10) << 12) + PA2VA_OFFSET);
        unsigned long vpn0 = (va >> VPN0OFFSET) & MUSK;
        pglevel0[vpn0] = ((pa >> 12) << 10) | perm;
        mapped += PGSIZE;
        va += PGSIZE;
        pa += PGSIZE;
    }
}

void setup_vm_final(void)
{
    memset(swapper_pg_dir, 0x0, PGSIZE);

    // No OpenSBI mapping required

    unsigned long pm_pointer = PHY_START + OPENSBI_SIZE;
    unsigned long vm_pointer = VM_START + OPENSBI_SIZE;
    // mapping kernel text X|-|R|V
    create_mapping(swapper_pg_dir, vm_pointer, pm_pointer, _srodata - _stext, EXECUTABLE | READABLE | VALID);
    pm_pointer += _srodata - _stext;
    vm_pointer += _srodata - _stext;
    // mapping kernel rodata -|-|R|V
    create_mapping(swapper_pg_dir, vm_pointer, pm_pointer, _sdata - _srodata, READABLE | VALID);
    pm_pointer += _sdata - _srodata;
    vm_pointer += _sdata - _srodata;
    // mapping other memory -|W|R|V
    create_mapping(swapper_pg_dir, vm_pointer, pm_pointer, 0x80000000 - (_sdata - _stext), WRITABLE | READABLE | VALID);

    // pm_pointer += 0x80000000 - (_sdata - _stext);
    unsigned long phypg = (unsigned long)swapper_pg_dir - PA2VA_OFFSET;
    // set satp with swapper_pg_dir
    __asm__ volatile(
        "add t1, zero, zero\n"
        "ori t1, t1, 1\n"
        "slli t1, t1, 63\n"
        "mv t0, %[phypg]\n"
        "srli t0, t0, 12\n"
        "add t0, t0, t1\n"
        "csrw satp, t0\n"
        :
        : [phypg] "r"(phypg)
        : "memory");
    // YOUR CODE HERE
    printk("setup_vm_final finish\n");
    // flush TLB
    __asm__ volatile("sfence.vma zero, zero");
    return;
}
// copy one page from src to dest
void copy(void *src, void *dest)
{
    unsigned long *beh = (unsigned long *)(dest);
    unsigned long *pre = (unsigned long *)(src);
    for (int i = 0; i < PGSIZE / 8; i++)
    {
        ((unsigned long *)(beh))[i] = ((unsigned long *)(pre))[i];
    }
}