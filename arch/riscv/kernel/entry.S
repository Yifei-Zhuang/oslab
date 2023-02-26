
    .section .text.entry
    .align 2
    .global __dummy
__dummy:
    # YOUR CODE HERE
    # la t0, dummy
    # csrw sepc, t0
    csrr t0,sscratch
    csrw sscratch, sp
    mv sp,t0
    sret

    .globl __switch_to
__switch_to:
    # save state to prev process
    # YOUR CODE HERE
    # skip the first 5 virables
    addi t0, a0, 8 * 5 
    sd ra, 0(t0)
    sd sp, 8(t0)
    
    addi t0, t0, 8 * 2
    sd s0, 0*8(t0)
    sd s1, 1*8(t0)
    sd s2, 2*8(t0)
    sd s3, 3*8(t0)
    sd s4, 4*8(t0)
    sd s5, 5*8(t0)
    sd s6, 6*8(t0)
    sd s7, 7*8(t0)
    sd s8, 8*8(t0)
    sd s9, 9*8(t0)
    sd s10, 10*8(t0)
    sd s11, 11*8(t0)
    # save sepc, sstatus and sscratch
    addi t0,t0,12 * 8
    csrr s0,sepc
    sd s0, 0 * 8(t0)
    csrr s0,sstatus
    sd s0,1 * 8(t0)
    csrr s0,sscratch
    sd s0, 2 * 8(t0)

    addi t0,t0,3 * 8
    # save satp
    csrr s0,satp
    sd s0,0(t0)
    # restore state from next process
    # YOUR CODE HERE
    addi t1, a1, 8 * 5 
    ld ra, 0(t1)
    ld sp, 8(t1)
    addi t1, t1, 8 * 2
    ld s0, 0*8(t1)
    ld s1, 1*8(t1)
    ld s2, 2*8(t1)
    ld s3, 3*8(t1)
    ld s4, 4*8(t1)
    ld s5, 5*8(t1)
    ld s6, 6*8(t1)
    ld s7, 7*8(t1)
    ld s8, 8*8(t1)
    ld s9, 9*8(t1)
    ld s10, 10*8(t1)
    ld s11, 11*8(t1)
    # restore sepc, sstatus and sscratch
    addi t1,t1,12 * 8
    ld t2, 0(t1)
    csrw sepc, t2
    ld t2, 8(t1)
    csrw sstatus, t2
    ld t2, 16(t1)
    csrw sscratch, t2
    addi t1,t1,3 * 8;
    ld t2,0(t1)
    csrw satp,t2
    sfence.vma
    fence.i
    
    ret


    .globl _traps 
_traps:
    # YOUR CODE HERE
    # -- -- -- -- -- -
    csrrw sp,sscratch,sp
noSwitchSp:
    addi sp,sp,-34 * 8
    # 1. save 32 registers and sepc to stack
    sd x1, 1 * 8(sp)
    addi x1,sp,34 * 8
    sd x1, 2 * 8(sp)
    sd x3, 3 * 8(sp)
    sd x4, 4 * 8(sp)
    sd x5, 5 * 8(sp)
    sd x6, 6 * 8(sp)
    sd x7, 7 * 8(sp)
    sd x8, 8 * 8(sp)
    sd x9, 9 * 8(sp)
    sd x10, 10 * 8(sp)
    sd x11, 11 * 8(sp)
    sd x12, 12 * 8(sp)
    sd x13, 13 * 8(sp)
    sd x14, 14 * 8(sp)
    sd x15, 15 * 8(sp)
    sd x16, 16 * 8(sp)
    sd x17, 17 * 8(sp)
    sd x18, 18 * 8(sp)
    sd x19, 19 * 8(sp)
    sd x20, 20 * 8(sp)
    sd x21, 21 * 8(sp)
    sd x22, 22 * 8(sp)
    sd x23, 23 * 8(sp)
    sd x24, 24 * 8(sp)
    sd x25, 25 * 8(sp)
    sd x26, 26 * 8(sp)
    sd x27, 27 * 8(sp)
    sd x28, 28 * 8(sp)
    sd x29, 29 * 8(sp)
    sd x30, 30 * 8(sp)
    sd x31, 31 * 8(sp)
    csrr x1, sepc
    sd x1, 32 * 8(sp)
    csrr x1,sstatus
    sd x1, 33 * 8(sp)

    #-- -- -- -- -- -

    # 2. call trap_handler
    csrr a0,scause
    csrr a1,sepc
    addi a2,sp,0
    call trap_handler

    #-- -- -- -- -- -

    # 3. restore sepc and 32 registers(x2(sp) should be restore last) from stack
    ld t0,32 * 8(sp)
    csrw sepc,t0
    ld t0,33 * 8(sp)
    csrw sstatus, t0

    ld x1, 1 * 8(sp)
    ld x3, 3 * 8(sp)
    ld x4, 4 * 8(sp)
    ld x5, 5 * 8(sp)
    ld x6, 6 * 8(sp)
    ld x7, 7 * 8(sp)
    ld x8, 8 * 8(sp)
    ld x9, 9 * 8(sp)
    ld x10, 10 * 8(sp)
    ld x11, 11 * 8(sp)
    ld x12, 12 * 8(sp)
    ld x13, 13 * 8(sp)
    ld x14, 14 * 8(sp)
    ld x15, 15 * 8(sp)
    ld x16, 16 * 8(sp)
    ld x17, 17 * 8(sp)
    ld x18, 18 * 8(sp)
    ld x19, 19 * 8(sp)
    ld x20, 20 * 8(sp)
    ld x21, 21 * 8(sp)
    ld x22, 22 * 8(sp)
    ld x23, 23 * 8(sp)
    ld x24, 24 * 8(sp)
    ld x25, 25 * 8(sp)
    ld x26, 26 * 8(sp)
    ld x27, 27 * 8(sp)
    ld x28, 28 * 8(sp)
    ld x29, 29 * 8(sp)
    ld x30, 30 * 8(sp)
    ld x31, 31 * 8(sp)
    ld sp,2 * 8(sp)
    #-- -- -- -- -- -
    csrrw sp,sscratch,sp
    #-- -- -- -- -- -
finish:
    # 4. return from trap
    sret
    #-- -- -- -- -- -
.global __ret_from_fork
__ret_from_fork:
    ld t0,32 * 8(sp)
    csrw sepc,t0
    ld t0,33 * 8(sp)
    csrw sstatus, t0

    ld x1, 1 * 8(sp)
    ld x3, 3 * 8(sp)
    ld x4, 4 * 8(sp)
    ld x5, 5 * 8(sp)
    ld x6, 6 * 8(sp)
    ld x7, 7 * 8(sp)
    ld x8, 8 * 8(sp)
    ld x9, 9 * 8(sp)
    ld x10, 10 * 8(sp)
    ld x11, 11 * 8(sp)
    ld x12, 12 * 8(sp)
    ld x13, 13 * 8(sp)
    ld x14, 14 * 8(sp)
    ld x15, 15 * 8(sp)
    ld x16, 16 * 8(sp)
    ld x17, 17 * 8(sp)
    ld x18, 18 * 8(sp)
    ld x19, 19 * 8(sp)
    ld x20, 20 * 8(sp)
    ld x21, 21 * 8(sp)
    ld x22, 22 * 8(sp)
    ld x23, 23 * 8(sp)
    ld x24, 24 * 8(sp)
    ld x25, 25 * 8(sp)
    ld x26, 26 * 8(sp)
    ld x27, 27 * 8(sp)
    ld x28, 28 * 8(sp)
    ld x29, 29 * 8(sp)
    ld x30, 30 * 8(sp)
    ld x31, 31 * 8(sp)
    ld sp, 2 * 8(sp)
    #-- -- -- -- -- -
    sret