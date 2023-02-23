.extern start_kernel
.extern task_init
.extern mm_init
    .section .text.init
    .globl _start
_start:
    la sp,boot_stack_top

#init memory system 
    call mm_init
#la stvec,_traps
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
    .section .bss.stack
    .globl boot_stack
boot_stack:
    .space 4096 # <-- change to your stack size

    .globl boot_stack_top
boot_stack_top:
