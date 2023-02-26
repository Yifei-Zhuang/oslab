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
extern void __ret_from_fork(struct pt_regs *regs);
extern char uapp_start[];
extern char uapp_end[];
extern unsigned long swapper_pg_dir[];

struct task_struct *idle;     // idle process
struct task_struct *current;  // 指向当前运行线程的 `task_struct`
struct task_struct *task[16]; // 线程数组, 所有的线程都保存在此
unsigned long task_count = 1;
static void load_program(struct task_struct *task)
{
    Elf64_Ehdr *ehdr = (Elf64_Ehdr *)uapp_start;

    unsigned long phdr_start = (unsigned long)ehdr + ehdr->e_phoff;
    int phdr_cnt = ehdr->e_phnum;

    Elf64_Phdr *phdr;
    for (int i = 0; i < phdr_cnt; i++)
    {
        phdr = (Elf64_Phdr *)(phdr_start + sizeof(Elf64_Phdr) * i);
        if (phdr->p_type == PT_LOAD)
        {
            // do mapping
            // code...
            unsigned long va = phdr->p_vaddr;
            do_mmap(task, PGROUNDDOWN(va), phdr->p_memsz, (phdr->p_flags << 1) | USER | VALID, phdr->p_offset, phdr->p_filesz);
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

    struct task_struct *launched = task[1] = (struct task_struct *)kalloc();
    launched->state = TASK_RUNNING;
    launched->counter = 0;
    launched->priority = rand();
    launched->pid = 1;
    launched->thread.ra = (unsigned long)__dummy;
    launched->thread.sp = (unsigned long)launched + PGSIZE;
    launched->thread_info = (struct thread_info *)(alloc_page());
    launched->thread_info->kernel_sp = launched->thread.sp;
    launched->thread_info->user_sp = (alloc_page());
    // 用户页表初始化
    pagetable_t pgtbl = (unsigned long *)(alloc_page());
    copy(swapper_pg_dir, pgtbl);
    do_mmap(launched, USER_END - PGSIZE, PGSIZE, READABLE | WRITABLE | USER | VALID, 0, 0);
    // set sstatus
    unsigned long sstatus = csr_read(sstatus);
    // set spp bit to 0
    sstatus &= (~(1 << 8));
    // set SPIE bit to 1
    sstatus |= (1 << 5);
    // set SUM bit to 1
    sstatus |= (1 << 18);
    launched->thread.sstatus = sstatus;
    // user mode stack
    launched->thread.sscratch = USER_END;

    // printk("SET [PID = %d] COUNTER = %d\n", launched -> pid, launched -> counter);
    unsigned long satp = csr_read(satp);
    launched->pgd = (pagetable_t)(((satp >> 44) << 44) | (((unsigned long)(pgtbl)-PA2VA_OFFSET) >> 12));
    load_program(launched);
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

void do_mmap(struct task_struct *current_task, unsigned long addr, unsigned long length, unsigned long flags, unsigned long vm_content_offset_in_file, unsigned long vm_content_size_in_file)
{
    struct vm_area_struct *newvma = &(current_task->vmas[current_task->vma_cnt]);
    newvma->vm_start = addr;
    newvma->vm_end = addr + length;
    newvma->vm_flags = flags;
    newvma->vm_content_offset_in_file = vm_content_offset_in_file;
    newvma->vm_content_size_in_file = vm_content_size_in_file;
    // 检查是否在第一个前面
    current_task->vma_cnt++;
}

struct vm_area_struct *find_vma(struct task_struct *task, unsigned long addr)
{
    int vma_count = task->vma_cnt;
    for (int i = 0; i < vma_count; i++)
    {
        struct vm_area_struct *move = &(task->vmas[i]);
        if (addr >= move->vm_start && addr < move->vm_end)
        {
            return move;
        }
    }
    return NULL;
}

#ifdef SJF
void schedule(void)
{
    /* YOUR CODE HERE */
    // printk("time chedule!\n");
    int flag = 0;
    int t = 0, mint = 1000000007;
    // printk("schedule:: begin for %d, %d\n", 1, NR_TASKS - 1);
    for (int i = 0; i <= task_count; ++i)
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
        for (int i = 1; i <= task_count; i++)
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
    for (int i = 0; i <= task_count; ++i)
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
        for (int i = 1; i <= task_count; i++)
        {
            task[i]->counter = rand();
            printk("SET [PID = %d PRIORITY = %d COUNTER = %d]\n", task[i]->pid, task[i]->priority, task[i]->counter);
        }
        schedule();
    }
}
#endif
unsigned long do_fork(struct pt_regs *regs)
{
    /*
        1. 参考 task_init 创建一个新的 task, 将的 parent task 的整个页复制到新创建的task_struct 页上(这一步复制了哪些东西?）。将 thread.ra 设置为__ret_from_fork, 并正确设置 thread.sp(仔细想想，这个应该设置成什么值?可以根据 child task 的返回路径来倒推)

        2. 利用参数 regs 来计算出 child task 的对应的 pt_regs 的地址，并将其中的 a0, sp, sepc 设置成正确的值(为什么还要设置 sp?)

        3. 为 child task 申请 user stack, 并将 parent task 的 user stack数据复制到其中。

        3.1. 同时将子 task 的 user stack 的地址保存在 thread_info->user_sp 中，如果你已经去掉了 thread_info，那么无需执行这一步

        4. 为 child task 分配一个根页表，并仿照 setup_vm_final 来创建内核空间的映射

        5. 根据 parent task 的页表和 vma 来分配并拷贝 child task 在用户态会用到的内存

        6. 返回子 task 的 pid
       */
    task_count++;
    int i = task_count;
    struct task_struct *child = task[i] = (struct task_struct *)kalloc();
    // 拷贝父进程整个page
    for (int i = 0; i < PGSIZE / 8; i++)
    {
        ((unsigned long *)child)[i] = ((unsigned long *)current)[i];
    }
    child->counter = 0;
    child->priority = rand();
    child->pid = i;
    child->thread.ra = (unsigned long)(__ret_from_fork);
    child->thread.sp = (unsigned long)child + (unsigned long)(((unsigned long)(regs)-PGROUNDDOWN((unsigned long)(current))));
    child->thread_info = (struct thread_info *)(alloc_page());
    child->thread_info->kernel_sp = child->thread.sp;
    // 拷贝父进程栈
    child->thread_info->user_sp = (alloc_page());
    unsigned long parent_user_stack = (unsigned long)(csr_read(sscratch));
    for (int j = 0; j < PGSIZE / 8; j++)
    {
        ((unsigned long *)(child->thread_info->user_sp))[j] = ((unsigned long *)(PGROUNDDOWN(parent_user_stack)))[j];
    }

    // 用户页表初始化
    pagetable_t pgtbl = (unsigned long *)(alloc_page());
    copy(swapper_pg_dir, pgtbl);
    // 拷贝父进程的mmap
    child->vma_cnt = current->vma_cnt;
    for (int j = 0; j < current->vma_cnt; j++)
    {
        child->vmas[j] = current->vmas[j];
    }
    // 如果有mmap已经映射，那么进行深拷贝
    for (int i = 0; i < current->vma_cnt; i++)
    {
        unsigned long pte = walk(current, current->vmas[i].vm_start);
        if (pte & VALID)
        {
            // 已经mapping，进行拷贝
            unsigned long *child_pages = (unsigned long *)(alloc_pages(PGROUNDUP((unsigned long)(current->vmas[i].vm_end) - (unsigned long)(current->vmas[i].vm_start)) / PGSIZE));
            for (int j = 0; j < (PGROUNDUP((unsigned long)(current->vmas[i].vm_end) - (unsigned long)(current->vmas[i].vm_start)) / 8); j++)
            {
                child_pages[j] = ((unsigned long *)((unsigned long)current->vmas[i].vm_start))[j];
            }
            // 执行mapping
            create_mapping(pgtbl, current->vmas[i].vm_start, (unsigned long)child_pages - PA2VA_OFFSET, PGROUNDUP((unsigned long)(current->vmas[i].vm_end) - (unsigned long)(current->vmas[i].vm_start)), current->vmas[i].vm_flags);
        }
    }
    // 拷贝父进程frame
    struct pt_regs *frame = (struct pt_regs *)((unsigned long)child + (unsigned long)(regs) - (unsigned long)(current));
    frame->a0 = 0;
    frame->sp = csr_read(sscratch);
    frame->sepc += 4;

    unsigned long sstatus = csr_read(sstatus);
    child->thread.sstatus = sstatus;

    child->thread.sscratch = child->thread_info->kernel_sp;

    unsigned long satp = csr_read(satp);
    child->pgd = (pagetable_t)(((satp >> 44) << 44) | (((unsigned long)(pgtbl)-PA2VA_OFFSET) >> 12));
    return i;
}

unsigned long clone(struct pt_regs *regs)
{
    return do_fork(regs);
}
