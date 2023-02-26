#ifndef __PAGE_FAULT_H__
#define __PAGE_FAULT_H__
#include "syscall.h"
#define PAGE_INSTRUCTION_FAULT 12
#define PAGE_LOAD_FAULT 13
#define PAGE_STORE_FAULT 15
void do_page_fault(struct pt_regs *regs);
#endif