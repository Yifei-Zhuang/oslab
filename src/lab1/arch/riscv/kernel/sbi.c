#include "sbi.h"
#include "types.h"

struct sbiret sbi_ecall(int ext, int fid, uint64 arg0, uint64 arg1, uint64 arg2,
                        uint64 arg3, uint64 arg4, uint64 arg5) {
  struct sbiret r;
  __asm__ volatile(
      "mv x17, %[ext]\n"
      "mv x16, %[fid]\n"
      "mv x10, %[arg0]\n"
      "mv x11, %[arg1]\n"
      "mv x12, %[arg2]\n"
      "mv x13, %[arg3]\n"
      "mv x14, %[arg4]\n"
      "mv x15, %[arg5]\n"
      "ecall\n"
      "mv %[sbi_ret],a1\n"
      "mv %[sbi_err],a0\n"
      : [sbi_ret] "=r"(r.value), [sbi_err] "=r"(r.error)
      : [ext] "r"(ext), [fid] "r"(fid), [arg0] "r"(arg0), [arg1] "r"(arg1),
        [arg2] "r"(arg2), [arg3] "r"(arg3), [arg4] "r"(arg4), [arg5] "r"(arg5)
      : "memory");
  return r;
}
