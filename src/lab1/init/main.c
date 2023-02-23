#include "print.h"
#include "sbi.h"
#include "defs.h"

extern void test();

int start_kernel() {
    puti(2022);
    puts(" Hello RISC-V\n");
    // int __trap = 0x81200000;
    // csr_read(sstatus);
    // csr_read(sscratch);
    // csr_write(sscratch,__trap);
    // csr_read(sscratch);
    test(); // DO NOT DELETE !!!
    
	return 0;
}
