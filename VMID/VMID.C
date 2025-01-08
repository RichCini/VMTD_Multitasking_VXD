//
// Get Virtual Machine ID
//
// All rights reserved, James M. Eli  2/10/1996
//

#include <stdio.h>

#define MK_FP(seg,ofs) ((void far *)(((unsigned long)(seg)<<16)|(ofs)))

( _far *lpDOSMGRv86Api) (void);

void ( _far *GetDOSMGRApi(void)) (void) {
    _asm {
        mov ax,0x1684
        mov bx,0x0015
        int 0x2f
        mov ax,di
        mov dx,es
    }
}

void main(void) {
    char _far *vmid1;
    unsigned short vmid2,segment,off;
    //
    // Is Windows Enhanced mode running ??
    //
    _asm {
        mov ax,1600h
        int 2fh
        test al,7fh
        jnz IsWin386
    }
    printf("Error: VMID requires Windows Enhanced Mode.\n");
    return;
    _asm {
       IsWin386:
    }
    //
    // Get DOSMGR VxD V86 API address
    //
    lpDOSMGRv86Api=GetDOSMGRApi();
    if(lpDOSMGRv86Api) {
        //
        // Call DOSMGR VxD to get VMID
        //
        _asm {
            mov ax,0x0004
            call lpDOSMGRv86Api
            mov ax,di
            mov word ptr off,ax
            mov ax,es
            mov word ptr segment,ax
        }
        vmid1=(char _far *)MK_FP(segment,off);
//      printf("\nDOSMGR VxD VM ID Byte(%04X:%04X): %d\",segment,off,*vmid1);
    }
    else
        printf("Cannot find DOSMGR VxD API\n");
    //
    //  Backup DOSMGR with int 2fh get VMID
    //
    _asm {
        xor bx,bx
        mov ax,1683h
        int 2fh
        mov word ptr vmid2,bx
    }
//  printf("INT 2Fh ax=1683h VM ID: %d\n",vmid2);
    if(*vmid1==vmid2)
        printf("VMID: %d\n",vmid2);
    else
        printf("DOSMGR VMID byte and INT 2fh ax=1683h conflict.\n");
}  
