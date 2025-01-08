//
// GOVM: switches between VMs by using undocumented int 2fh
//
// All rights reserved, James Eli  2/10/96
//
#include <stdio.h>
#include <stdlib.h>

#define WORD unsigned short
#define DWORD unsigned long

typedef void far *FP;
static FP API = (FP) 0;

FP GetVxDAPI(WORD id) {
    _asm {
        push di
        mov ax,1684h
        mov bx,id
        xor di,di
        mov es,di
        int 2fh
        mov ax,di
        mov dx,es
        pop di
   }
}

int main(int argc, char *argv[]) {
    unsigned short version;
    WORD vmid;
    //
    // Is Windows Enhanced mode running ??
    //
    _asm {
        mov ax,1600h
        int 2fh
        test al,7fh
        jnz IsWin386
    }
    printf("Error: GOVM requires Windows Enhanced Mode.\n");
    return 0;
    _asm {
	IsWin386:
    }
    //
    // check for a VM id # on command line
    //
    if(argc < 2) {
	printf("Usage: GOVM #\n where # is a Virtual Machine number (VMID).\n");
	return 0;
    }
    vmid=atoi(argv[1]);
    _asm {
        mov ax,0x168b 
        mov bx,word ptr vmid
        int 0x2f
    }
    return 0;
}   
