.extern global dummy
.section .text.entry
.align 2
.globl _traps 
_traps:
#YOUR CODE HERE
#-- -- -- -- -- -

# 1. save 32 registers and sepc to stack
addi sp,sp,-33*8
sd x0 ,33*8(sp)
sd x1 ,32*8(sp)
sd x2 ,31*8(sp)
sd x3 ,30*8(sp)
sd x4 ,29*8(sp)
sd x5 ,28*8(sp)
sd x6 ,27*8(sp)
sd x7 ,26*8(sp)
sd x8 ,25*8(sp)
sd x9 ,24*8(sp)
sd x10 ,23*8(sp)
sd x11 ,22*8(sp)
sd x12 ,21*8(sp)
sd x13 ,20*8(sp)
sd x14 ,19*8(sp)
sd x15 ,18*8(sp)
sd x16 ,17*8(sp)
sd x17 ,16*8(sp)
sd x18 ,15*8(sp)
sd x19 ,14*8(sp)
sd x20 ,13*8(sp)
sd x21 ,12*8(sp)
sd x22 ,11*8(sp)
sd x23 ,10*8(sp)
sd x24 ,9*8(sp)
sd x25 ,8*8(sp)
sd x26 ,7*8(sp)
sd x27 ,6*8(sp)
sd x28 ,5*8(sp)
sd x29 ,4*8(sp)
sd x30 ,3*8(sp)
sd x31 ,2*8(sp)
# save spec
csrr t0,sepc
sd t0,8(sp)




#-- -- -- -- -- -

# 2. call trap_handler
csrr a0,scause
csrr a1,sepc
call trap_handler

#-- -- -- -- -- -

# 3. restore sepc and 32 registers(x2(sp) should be restore last) from stack
ld t0, 8(sp)
csrw sepc,t0
ld x0 ,33*8(sp)
ld x1 ,32*8(sp)
ld x2 ,31*8(sp)
ld x3 ,30*8(sp)
ld x4 ,29*8(sp)
ld x5 ,28*8(sp)
ld x6 ,27*8(sp)
ld x7 ,26*8(sp)
ld x8 ,25*8(sp)
ld x9 ,24*8(sp)
ld x10 ,23*8(sp)
ld x11 ,22*8(sp)
ld x12 ,21*8(sp)
ld x13 ,20*8(sp)
ld x14 ,19*8(sp)
ld x15 ,18*8(sp)
ld x16 ,17*8(sp)
ld x17 ,16*8(sp)
ld x18 ,15*8(sp)
ld x19 ,14*8(sp)
ld x20 ,13*8(sp)
ld x21 ,12*8(sp)
ld x22 ,11*8(sp)
ld x23 ,10*8(sp)
ld x24 ,9*8(sp)
ld x25 ,8*8(sp)
ld x26 ,7*8(sp)
ld x27 ,6*8(sp)
ld x28 ,5*8(sp)
ld x29 ,4*8(sp)
ld x30 ,3*8(sp)
ld x31 ,2*8(sp)

#-- -- -- -- -- -

# 4. return from trap
sret
#-- -- -- -- -- -

# __dummy, used for thread init
    .global __dummy
__dummy:
    la t0, dummy
    csrw sepc,t0
    sret


  .globl __switch_to
__switch_to:
    # save state to prev process
    # YOUR CODE HERE
    # here, t0 is current, and t1 is next(
    # TODO( change to a0 and a1?)
    # save ra
    sd ra, 5 * 8(a0)
    # save sp
    sd sp, 6 * 8(a0)
    # save s0 to s11
    sd s0 , 7 * 8(a0)
    sd s1 , 8 * 8(a0)
    sd s2 , 9 * 8(a0)
    sd s3 , 10 * 8(a0)
    sd s4 , 11 * 8(a0)
    sd s5 , 12 * 8(a0)
    sd s6 , 13 * 8(a0)
    sd s7 , 14 * 8(a0)
    sd s8 , 15 * 8(a0)
    sd s9 , 16 * 8(a0)
    sd s10 , 17 * 8(a0)
    sd s11 , 18 * 8(a0)
    # restore state from next process
    # YOUR CODE HERE
    # load ra
    ld ra, 5 * 8(a1)
    # load sp
    ld sp, 6 * 8(a1)
    # load from s0 to s11
    ld s0 , 7 * 8(a1)
    ld s1 , 8 * 8(a1)
    ld s2 , 9 * 8(a1)
    ld s3 , 10 * 8(a1)
    ld s4 , 11 * 8(a1)
    ld s5 , 12 * 8(a1)
    ld s6 , 13 * 8(a1)
    ld s7 , 14 * 8(a1)
    ld s8 , 15 * 8(a1)
    ld s9 , 16 * 8(a1)
    ld s10 , 17 * 8(a1)
    ld s11 , 18 * 8(a1)
    ret