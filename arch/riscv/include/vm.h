#ifndef __VM_H__
#define __VM_H__
#include "proc.h"
#define VALID 1
#define READABLE (1 << 1)
#define WRITABLE (1 << 2)
#define EXECUTABLE (1 << 3)
#define USER (1 << 4)
#define GLOBAL (1 << 5)
#define ACCESSED (1 << 6)
#define DIRTY (1 << 7)

#define VPN2OFFSET 30
#define VPN1OFFSET 21
#define VPN0OFFSET 12
#define MUSK (0x1ff)

extern char _etext[];
extern char _stext[];

extern char _erodata[];
extern char _srodata[];

extern char _edata[];
extern char _sdata[];

extern char _sbss[];
extern char _ebss[];
void vmMapping(unsigned long vm_start, unsigned long physical_start,
               unsigned long size, unsigned long permission);
void setup_vm(void);
void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa,
                    unsigned long sz, int perm);
void setup_vm_final(void);

void copy(void *src, void *dest);

unsigned long walk(struct task_struct *task, unsigned long vm_address);
#endif