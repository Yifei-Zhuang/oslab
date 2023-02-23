// arch/riscv/kernel/proc.c

#include "defs.h"
#include "mm.h"
#include "rand.h"
#include "printk.h"
#include "proc.h"
extern void __dummy();
extern void __switch_to(struct task_struct *prev, struct task_struct *next);

struct task_struct *idle;           // idle process
struct task_struct *current;        // 指向当前运行线程的 `task_struct`
struct task_struct *task[NR_TASKS]; // 线程数组, 所有的线程都保存在此

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

    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`,
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址,  `sp` 设置为 该线程申请的物理页的高地址
    for (int i = 1; i <= NR_TASKS - 1; ++i)
    {
        task[i] = (struct task_struct *)kalloc();
        task[i]->state = TASK_RUNNING;
        task[i]->counter = 0;
        task[i]->priority = rand();
        task[i]->pid = i;
        task[i]->thread.ra = (unsigned long)__dummy;
        task[i]->thread.sp = (unsigned long)task[i] + PGSIZE;
        // printk("SET [PID = %d] COUNTER = %d\n", task[i] -> pid, task[i] -> counter);
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