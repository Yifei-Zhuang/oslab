#include "clock.h"
#include "printk.h"
#include "proc.h"
#include "syscall.h"
#include "pagefault.h"
void trap_handler(unsigned long scause, unsigned long sepc, struct pt_regs *regs)
{
    if ((scause >> 63) & 1)
    {
        // interrupt
        if ((scause & 5) == 5)
        {
            clock_set_next_event();
            do_timer();
        }
    }
    else
    {
        // exception
        if (scause == 8)
        {
            // ecall from u_mode
            syscall(regs);
        }
        else if (scause == PAGE_INSTRUCTION_FAULT || scause == PAGE_LOAD_FAULT || scause == PAGE_STORE_FAULT)
        {
            do_page_fault(regs);
        }
        else
        {
            printk("[S] Unhandled trap, ");
            printk("scause: %lx, ", scause);
            printk("sepc: %lx\n", regs->sepc);
            while (1)
                ;
        }
    }
}
