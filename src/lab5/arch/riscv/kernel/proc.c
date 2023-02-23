// arch/riscv/kernel/proc.c

#include "defs.h"
#include "mm.h"
#include "rand.h"
#include "printk.h"
#include "proc.h"
#include "vm.h"
#include "elf.h"
#include "string.h"
extern void __dummy();
extern void __switch_to(struct task_struct *prev, struct task_struct *next);
extern void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa, unsigned long sz, int perm);
extern char uapp_start[];
extern char uapp_end[];
extern unsigned long swapper_pg_dir[];

struct task_struct *idle;           // idle process
struct task_struct *current;        // 指向当前运行线程的 `task_struct`
struct task_struct *task[NR_TASKS]; // 线程数组, 所有的线程都保存在此
static void load_program(struct task_struct *task)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)uapp_start;

    unsigned long phdr_start = (unsigned long)ehdr + ehdr->e_phoff;
    int phdr_cnt = ehdr->e_phnum;
    pagetable_t pgtbl = (unsigned long *)(((((unsigned long)(task->pgd)) & 0xfffffffffff) << 12) + PA2VA_OFFSET);
    Elf64_Phdr *phdr;
    int load_phdr_cnt = 0;
    for (int i = 0; i < phdr_cnt; i++)
    {
        phdr = (Elf64_Phdr *)(phdr_start + sizeof(Elf64_Phdr) * i);
        if (phdr->p_type == PT_LOAD)
        {
            // do mapping
            // code...
            unsigned long va = phdr->p_vaddr;
            // unsigned long filePages = PGROUNDUP(phdr->p_filesz) / PGSIZE;
            unsigned long memPages = (PGROUNDUP(phdr->p_memsz)) / PGSIZE;
            // 复制代码段
            unsigned long mappedPage = alloc_pages(memPages);
            for (int i = 0; i < PGROUNDUP(phdr->p_filesz) / 8; i++)
            {
                ((unsigned long *)(mappedPage))[i] = ((unsigned long *)(PGROUNDDOWN(((unsigned long)uapp_start + phdr->p_offset))))[i];
            }
            // 映射多余内存段
            create_mapping(pgtbl, va, (unsigned long)(mappedPage)-PA2VA_OFFSET, memPages * PGSIZE, (phdr->p_flags << 1) | USER | VALID);
            load_phdr_cnt++;
        }
    }

    task->thread.sepc = ehdr->e_entry;
}
void task_init()
{
    // printk("Begin!\n");
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    idle = (struct task_struct *)kalloc();
    // printk("Allocate success!\n");

    // 2. 设置 state 为 TASK_RUNNING;
    idle->state = TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    idle->counter = 0;
    idle->priority = 0;
    // 4. 设置 idle 的 pid 为 0
    idle->pid = 0;
    // 5. 将 current 和 task[0] 指向 idle
    current = idle;
    task[0] = idle;
    /* YOUR CODE HERE */

    for (int i = 1; i <= NR_TASKS - 1; ++i)
    {
        task[i] = (struct task_struct *)kalloc();
        task[i]->state = TASK_RUNNING;
        task[i]->counter = 0;
        task[i]->priority = rand();
        task[i]->pid = i;
        task[i]->thread.ra = (unsigned long)__dummy;
        task[i]->thread.sp = (unsigned long)task[i] + PGSIZE;
        task[i]->thread_info = (struct thread_info *)(alloc_page());
        task[i]->thread_info->kernel_sp = task[i]->thread.sp;
        task[i]->thread_info->user_sp = (alloc_page());
        // 用户页表初始化
        pagetable_t pgtbl = (unsigned long *)(alloc_page());
        copy(swapper_pg_dir, pgtbl);
        create_mapping(pgtbl, USER_END - PGSIZE, (unsigned long)(task[i]->thread_info->user_sp - PA2VA_OFFSET), PGSIZE, READABLE | WRITABLE | USER | VALID);
        // set sstatus
        unsigned long sstatus = csr_read(sstatus);
        // set spp bit to 0
        sstatus &= (~(1 << 8));
        // set SPIE bit to 1
        sstatus |= (1 << 5);
        // set SUM bit to 1
        sstatus |= (1 << 18);
        task[i]->thread.sstatus = sstatus;
        // user mode stack
        task[i]->thread.sscratch = USER_END;

        // printk("SET [PID = %d] COUNTER = %d\n", task[i] -> pid, task[i] -> counter);
        unsigned long satp = csr_read(satp);
        task[i]->pgd = (pagetable_t)(((satp >> 44) << 44) | (((unsigned long)(pgtbl)-PA2VA_OFFSET) >> 12));
        load_program(task[i]);
    }
    /* YOUR CODE HERE */
    printk("...proc_init done!\n");
}

void dummy()
{
    unsigned long MOD = 1000000007;
    unsigned long auto_inc_local_var = 0;
    int last_counter;
    last_counter = -1;
    // printk("dummy::\n");
    while (1)
    {
        if (last_counter == -1 || current->counter != last_counter)
        {
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            printk("[PID = %d] is running. thread space begin at 0x%lx\n", current->pid, (unsigned long)(current->thread.sp));
        }
    }
}

void switch_to(struct task_struct *next)
{
    // printk("Begin::switch_to\n");
    if (next == current)
        return;
    else
    {
        printk("switch to [PID = %d COUNTER = %d]\n", next->pid, next->counter);

        struct task_struct *pre = current;
        current = next;
        __switch_to(pre, next);
    }
}

void do_timer(void)
{
    // 1. 如果当前线程是 idle 线程 直接进行调度
    // 2. 如果当前线程不是 idle 对当前线程的运行剩余时间减1 若剩余时间仍然大于0 则直接返回 否则进行调度
    if (current == idle)
    {
        printk("idle process is running!\n\n");
        schedule();
    }
    else if (--(current->counter) == 0)
    {
        schedule();
    }
}
#ifdef SJF
void schedule(void)
{
    /* YOUR CODE HERE */
    // printk("time chedule!\n");
    int flag = 0;
    int t = 0, mint = 1000000007;
    // printk("schedule:: begin for %d, %d\n", 1, NR_TASKS - 1);
    for (int i = 0; i <= NR_TASKS - 1; ++i)
    {
        // printk("schedule:: i = %d, state = %d, counter = %d\n", i, task[i] -> state, task[i] -> counter);
        if ((task[i]->state == TASK_RUNNING) && (task[i]->counter != 0))
        {
            // printk("schedule:: i = %d, ok!\n", i);
            flag = 1;
            if (task[i]->counter < mint)
            {
                t = i;
                mint = task[i]->counter;
            }
        }
    }
    // printk("schedule:: end for\n");

    if (flag)
    {
        // printk("switch to [PID = %d COUNTER = %d]\n", task[t]->pid, task[t]->counter);
        switch_to(task[t]);
    }
    else
    {
        for (int i = 1; i < NR_TASKS; i++)
        {
            task[i]->counter = rand();
            printk("SET [PID = %d COUNTER = %d]\n", task[i]->pid, task[i]->counter);
        }
        schedule();
    }
}
#endif

#ifdef PRIORITY
void schedule(void)
{
    /* YOUR CODE HERE */
    // printk("priority schedule!\n");
    int flag = 0;
    int t = 0, maxp = 0;
    // printk("schedule:: begin for %d, %d\n", 1, NR_TASKS - 1);
    for (int i = 0; i <= NR_TASKS - 1; ++i)
    {
        // printk("schedule:: i = %d, state = %d, counter = %d\n", i, task[i] -> state, task[i] -> counter);
        if ((task[i]->state == TASK_RUNNING) && (task[i]->counter != 0))
        {
            // printk("schedule:: i = %d, ok!\n", i);
            flag = 1;
            if (task[i]->priority >= maxp)
            {
                t = i;
                maxp = task[i]->priority;
            }
        }
    }
    // printk("schedule:: end for\n");

    if (flag)
    {
        printk("switch to [PID = %d PRIORITY = %d COUNTER = %d]\n", task[t]->pid, task[t]->priority, task[t]->counter);
        switch_to(task[t]);
    }
    else
    {
        for (int i = 1; i < NR_TASKS; i++)
        {
            task[i]->counter = rand();
            printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
        }
        schedule();
    }
}
#endif