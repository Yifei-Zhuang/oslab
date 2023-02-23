#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define SYS_WRITE 64
#define SYS_GETPID 172

#define SYS_MUNMAP 215
#define SYS_CLONE 220 // fork
#define SYS_MMAP 222
#define SYS_MPROTECT 226

struct pt_regs
{
  // unsigned long x[32];
  unsigned long zero;
  unsigned long ra;
  unsigned long sp;
  unsigned long gp;
  unsigned long tp;
  unsigned long t0, t1, t2;
  unsigned long fp;
  unsigned long s1;
  unsigned long a0, a1, a2, a3, a4, a5, a6, a7;
  unsigned long s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
  unsigned long t3, t4, t5, t6;
  unsigned long sepc;
  unsigned long sstatus;
};
int sys_write(unsigned int fd, const char *buf, unsigned long count);
int sys_getpid();
void syscall(struct pt_regs *);
#endif