#include "pagefault.h"
#include "defs.h"
#include "proc.h"
#include "vm.h"
#include "printk.h"
#include "mm.h"
extern struct task_struct *current;
extern char uapp_start[];
void do_page_fault(struct pt_regs *regs)
{
    unsigned long stval = csr_read(stval);
    unsigned long scause = csr_read(scause);
    for (int i = 0; i < current->vma_cnt; i++)
    {
        struct vm_area_struct *move = (current->vmas + i);
        if (move->vm_start <= stval && move->vm_end > stval)
        {
            // 非法访问
            if (scause == PAGE_INSTRUCTION_FAULT && ((move->vm_flags & EXECUTABLE) == 0))
            {
                printk("invalid mem access for PAGE_INSTRUCTION_FAULT\n");
                return;
            }
            if (scause == PAGE_LOAD_FAULT && ((move->vm_flags & READABLE) == 0))
            {
                printk("invalid mem access for PAGE_LOAD_FAULT\n");
                return;
            }
            if (scause == PAGE_STORE_FAULT && ((move->vm_flags & WRITABLE) == 0))
            {
                printk("invalid mem access for PAGE_INSTRUCTION_FAULT\n");
                return;
            }
            pagetable_t pgtbl = (unsigned long *)(((((unsigned long)(current->pgd)) & 0xfffffffffff) << 12) + PA2VA_OFFSET);
            // 检查是否是匿名页
            if (move->vm_content_offset_in_file == 0 && move->vm_content_size_in_file == 0)
            {
                create_mapping(pgtbl, PGROUNDDOWN(move->vm_start), current->thread_info->user_sp - PA2VA_OFFSET, (unsigned long)(move->vm_end - move->vm_start), move->vm_flags);
                return;
            }
            else
            {
                // 磁盘文件
                unsigned long memPages = (PGROUNDUP(move->vm_end - move->vm_start)) / PGSIZE;
                // 复制代码段
                unsigned long mappedPage = alloc_pages(memPages);
                for (int j = 0; j < PGROUNDUP(move->vm_content_size_in_file) / 8; j++)
                {
                    ((unsigned long *)(mappedPage))[j] = ((unsigned long *)(PGROUNDDOWN(((unsigned long)(uapp_start + move->vm_content_offset_in_file)))))[j];
                }
                // 映射多余内存段
                create_mapping(pgtbl, move->vm_start, (unsigned long)(mappedPage - PA2VA_OFFSET), (memPages)*PGSIZE, move->vm_flags);
                return;
            }
        }
    }
    printk("[page fault] invalid access\n");
    return;
}