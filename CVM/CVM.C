//
// CVM: Create a VM via Protect Mode DOS
//
// All rights reserved James Eli  2/10/96
//
// DPMI Shell code required from "Undocumented DOS 2E" book 
// by: Andrew Schulman, et al. 1994
//
#include <stdio.h>
#include <dpmish.h>
//
// Some type defines...
//
#define WORD unsigned short
#define DWORD unsigned long
#define SELECTOROF(x) ((unsigned short)((((unsigned long)(x))>>16)&0xffff))
#define OFFSETOF(x) ((WORD)(DWORD)(x))
//
// PIF flags
//
#define fpifExclusive   0x00000001
#define fpifBackground  0x00000002      
#define fpifWindowed    0x00000004
#define fpifALTTABdis   0x00000020
#define fpifALTESCdis   0x00000040
#define fpifALTSPACEdis 0x00000080
#define fpifALTENTERdis 0x00000100
#define fpifALTPRTSCdis 0x00000200
#define fpifPRTSCdis    0x00000400
#define fpifCTRLESCdis  0x00000800
#define fpifPolling     0x00001000
#define fpifNoHMA       0x00002000
#define fpifHasHotKey   0x00004000
#define fpifEMS_Lock    0x00008000
#define fpifXMS_Lock    0x00010000
#define fpifFastPaste   0x00020000
#define fpifV86_Lock    0x00040000
#define fpifClsExit     0x40000000
//
// Display flags
//
#define fdisEmText      0x00000001
#define fdisNoText      0x00000002
#define fdisNoLow       0x00000004
#define fdisNoHigh      0x00000008
#define fdisUnknown     0x00000010
#define fdisRetain      0x00000080
//
//   Undocumented interface to SHELL VxD
//   thanks to Paul Bonneau, Kelly Zytaruk, and NuMega WINICE
//
typedef struct {
    DWORD   ib;
    WORD    sel;
} WFP;

typedef struct {
    DWORD   grfpif;
    DWORD   grfdis;
    WFP     wfpExe;
    WFP     wfpArgs;
    WFP     wfpWork;
    WORD    cpagWant;
    WORD    cpagMin;
    WORD    wForePriority;
    WORD    wBackPriority;
    WORD    ckbMaxEMS;
    WORD    ckbMinEMS;
    WORD    ckbMaxXMS;
    WORD    ckbMinXMS;
    WORD    hWndWinOldAp;
    WORD    wSelWinOldApHeap;
    char    szTitle[128];
} SEB;

typedef void _far* LPVOID;
typedef void far *FP;
static FP Shell_VxD_api = (FP) 0;
static FP JulieEli_API = (FP) 0;

void fail(const char *s, ...) { puts(s); _dos_exit(1); }


FP GetVxDAPI(WORD id) {
    //
    // shamelessly hacked code from Andrew Schulman
    //
    _asm {
	push    di
	mov     ax,1684h
	mov     bx,id
	xor     di,di
	mov     es,di
	int     2fh
	mov     ax,di
	mov     dx,es
	pop     di
    }
}

int real_main(int argc,char *argv[]) {
    unsigned short vmid;

    printf("Create DOS Virtual Machine.\n");
    printf("All rights reserved: James M. Eli, Jan 1996.\n");
    printf("Portions copyright: Andrew Schulman [et. al.], Feb 1993.\n");
    //
    // Check if we are operating in the System VM
    //
    _asm {
        xor bx,bx
        mov ax,0x1683
        int 0x2f
        mov word ptr vmid,bx
    }
    if(vmid!=1) {
        printf("\nError: run only in VMID: 1.  Current VMID: %d\n",vmid);
	return 1;
    }
    return 0;
}

int pmode_main(int argc,char *argv[]) {
    char szPath[128],szExe[128],szBuf[128];
    SEB seb;
    unsigned long hvm;
    unsigned short version;
    //
    // Init SHELL VxD API
    //
    if(!(Shell_VxD_api=GetVxDAPI(0x0017))) {
        printf("\nError: cannot find Shell VxD PM entry point.\n");
	return 0;
    }
    //    
    // check SHELL VxD version
    //
    _asm {
	xor dx,dx
        call dword ptr [Shell_VxD_api]
        mov word ptr version,ax
    }
    if(version!=0x030a) {
        printf("\nError: unrecognized SHELL VxD version.\n");
	return 0;
    }
    //
    // Get path & working directory for the DOS application
    // The following code assumes that COMMAND.COM is located in the root 
    // directory of the C drive! I'm too lazy to do it the right way...
    //
    sprintf(szExe,"C:\\COMMAND.COM\0x00");
    sprintf(szPath,"C:\0x00");
    //
    // fill SHELL VxD data structure
    //
    seb.grfpif=fpifFastPaste | fpifBackground | fpifPolling | fpifHasHotKey;
    seb.grfdis=fdisUnknown | fdisEmText | fdisNoText | fdisNoLow | fdisNoHigh;
    seb.wfpExe.ib=(WORD)szExe;
    seb.wfpExe.sel=SELECTOROF((LPVOID)&seb);
    seb.wfpArgs.ib=(WORD)"";
    seb.wfpArgs.sel=seb.wfpExe.sel;
    seb.wfpWork.ib=(WORD)szPath;
    seb.wfpWork.sel=seb.wfpExe.sel;
    seb.cpagWant=0x0280;
    seb.cpagMin=0x00c0;
    seb.wForePriority=0x0064;
    seb.wBackPriority=0x0032;
    seb.ckbMaxEMS=0x0400;
    seb.ckbMinEMS=0x0000;
    seb.ckbMaxXMS=0x0400;
    seb.ckbMinXMS=0x0000;
    seb.hWndWinOldAp=0x0000;
    seb.wSelWinOldApHeap=0x0000;
    strcpy(seb.szTitle,"JE DOS Prompt");
    //
    // ask SHELL VxD to create VM for us
    //
    _asm {
	lea di,word ptr seb
	mov dx,3
	push es
	push ss
	pop es
        call dword ptr [Shell_VxD_api]
	pop es
	_emit 66h
	mov word ptr hvm,ax
    }
    if(0!=hvm) {
	printf("Attempting to create VM: %08lx.\n",hvm);
        _asm {
            xor ax,ax
            xor cx,cx
            mov dx,0x0005
            _emit 66H
            mov bx,word ptr hvm
            call dword ptr [Shell_VxD_api]
        }
    }
    return 0;
}  
