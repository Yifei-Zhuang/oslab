#include "../include/syscall.h"
#include "../../../include/printk.h"
#include "../include/mm.h"
#include "../include/defs.h"
#include "../include/vm.h"
#include "../include/proc.h"
extern struct task_struct *current;
int sys_write(unsigned int fd, const char *buf, unsigned long count)
{
    char *buffer = (char *)(alloc_page(PGROUNDUP(count) / PGSIZE));
    for (int i = 0; i < count; i++)
    {
        buffer[i] = buf[i];
    }
    buffer[count] = '\0';
    int result = printk(buffer);
    free_pages((unsigned long)(buffer));
    return result;
}
int sys_getpid()
{
    return current->pid;
}
unsigned long sys_clone(struct pt_regs *regs)
{
    return clone(regs);
}
void syscall(struct pt_regs *regs)
{
    unsigned long sys_code = regs->a7;
    int return_value = -1;
    switch (sys_code)
    {
    case SYS_GETPID:
    {
        return_value = sys_getpid();
        break;
    }
    case SYS_WRITE:
    {
        unsigned long fd = regs->a0;
        unsigned long buf = regs->a1;
        unsigned long count = regs->a2;
        return_value = sys_write(fd, (const char *)buf, count);
        break;
    }
    case SYS_CLONE:
    {
        return_value = sys_clone(regs);
        break;
    }
    }
    // write result to a0
    regs->a0 = return_value;
    regs->sepc += 4;
}