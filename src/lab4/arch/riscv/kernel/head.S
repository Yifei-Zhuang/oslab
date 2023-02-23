#include "defs.h"
.extern start_kernel
.extern task_init
.extern mm_init
.extern setup_vm
.extern setup_vm_final
.section .text.init
.globl _start
_start:
    la sp,boot_stack_top
    // 等值映射和高地址映射
    call setup_vm
    // 设置satp，开启虚拟内存
    call relocate
    // 初始化 kernel 内存
    call mm_init
    // 对kernel内存进行rv39三级映射
    call setup_vm_final
    // 开启中断处理
    la t0, _traps
    csrw stvec,t0

# enable time interrupt
# set sie[STIE] = 1
    li t0,0x20
    csrw sie,t0

# set mtimecmp register
# load mtime value
    rdtime a0
    li t1, 10000000
    add a0,t1,a0
    call sbi_set_timer
# enable time interrupt
# set sstatus[SIE] = 1
    csrr t0,sstatus
    ori t0,t0,0x2
    csrw sstatus,t0
# call start_kernel
    call task_init
    call start_kernel
relocate:
    # set ra = ra + PA2VA_OFFSET
    # set sp = sp + PA2VA_OFFSET (If you have set the sp before)

    ###################### 
    #   YOUR CODE HERE   #
    ######################
    // 将ra和sp修改为虚拟内存高地址映射中的地址
    li t0,PA2VA_OFFSET;
    add ra,ra,t0;
    add sp,sp,t0;
    # set satp with early_pgtbl
    # 设置satp模式为rv39
    add t1,zero,zero;
    ori t1,t1,1;
    slli t1,t1,63;
    # 设置satp中的ppn
    la t0,early_pgtbl;
    srli t0,t0,12;
    add t0,t0,t1
    # 开启虚拟内存
    csrw satp,t0;
    ###################### 
    #   YOUR CODE HERE   #
    ######################

    # flush tlb
    sfence.vma zero, zero
	fence.i
    ret
    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top:
