/* CREATEVM: Creates a hidden VM via undocumented SHELL VxD call
   All rights reserved, James Eli  2/10/96
   I'm not responsible for all of this...
*/

#include <windows.h>
#include <stdlib.h>
#include <string.h>
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
//   with help from Paul Bonneau, Kelly Zytaruk and NuMega WINICE
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

typedef DWORD (FAR * LPFN)(void);

int PASCAL _export WinMain(HINSTANCE hins,HINSTANCE hinsPrev,LPSTR lpsz, int wShow) {
    char szPath[128],szExe[128],szBuf[128];
//    char *pch;
    LPFN lpfn;
    SEB seb;
    DWORD hvm;
    //
    // Get Shell VxD PM entry point
    //
    _asm {
	mov ax,1684h
	mov bx,0017h
	int 2fh
	mov word ptr [lpfn],di
	mov word ptr [lpfn+2],es
    }
    if(NULL==lpfn)
	return 0;
    //
    // Check SHELL VxD version number
    //
    _asm mov dx,0;
    if(0x030a>(*lpfn)())
	return 0;
    //
    // Get path & working directory for DOS application
    // put file name of your DOS app here...
    //
    wsprintf(szExe,"C:\\COMMAND.COM\0x00");
    wsprintf(szPath,"C:\0x00");
    //
    // fill SHELL VxD data structure
    //
    seb.grfpif=fpifClsExit | fpifFastPaste | fpifBackground | fpifALTTABdis;
    //seb.grfpif=0x40021000;
    seb.grfdis=fdisUnknown | fdisEmText | fdisNoText | fdisNoLow;
    //seb.grfdis=0x0000001f;
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
    lstrcpy(seb.szTitle,"MS-DOS Prompt");
    //
    // ask SHELL VxD to create VM
    //
    _asm {
	lea di,word ptr seb;
	mov dx,3;
	push es;
	push ss;
	pop es;
    }
    (*lpfn)();
    _asm pop es;
    _asm _emit 66h;
    _asm mov word ptr hvm,ax
    wsprintf(szBuf,"VM: %lx created.",hvm);
    MessageBox(NULL,szBuf,"SHELL VxD HIDDEN VM CREATOR",MB_OK);
    return 0;
}
