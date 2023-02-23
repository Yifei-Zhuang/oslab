#include "print.h"
#include "sbi.h"


void puts(char *s) {
    int move = 0;
    while(s[move] != '\0'){
        sbi_ecall(0x1, 0x0, s[move++], 0, 0, 0, 0, 0);
    }
}

void puti(uint64 x) {  
    /*int count = 0;
    char temp[20] = {'\0'};
    while(x){
        temp[count++] = x % 10;
        x/=10;
    }
    count--;
    while(count >= 0){
        sbi_ecall(0x1, 0x0, temp[count--], 0, 0, 0, 0, 0);
    }*/
    if(x == 0){
	    return; 
    }
    puti(x / 10);
    sbi_ecall(0x1,0x0,'0' + (x % 10),0,0,0,0,0);
}
