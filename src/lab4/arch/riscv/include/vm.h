
const int VALID = 1;
const int READABLE = 1 << 1;
const int WRITABLE = 1 << 2;
const int EXECUTABLE = 1 << 3;
const int USER = 1 << 4;
const int GLOBAL = 1 << 5;
const int ACCESSED = 1 << 6;
const int DIRTY = 1 << 7;

const int VPN2OFFSET = 30;
const int VPN1OFFSET = 21;
const int VPN0OFFSET = 12;
const int MUSK = 0x1ff;

extern char _etext[];
extern char _stext[];

extern char _erodata[];
extern char _srodata[];

extern char _edata[];
extern char _sdata[];

extern char _sbss[];
extern char _ebss[];
void vmMapping(unsigned long vm_start, unsigned long physical_start,
               unsigned long size, unsigned long permission);
void setup_vm(void);
void create_mapping(unsigned long *pgtbl, unsigned long va, unsigned long pa,
                    unsigned long sz, int perm);
void setup_vm_final(void);