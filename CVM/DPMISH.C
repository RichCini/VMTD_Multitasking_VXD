/* 
DPMISH.C
Shell to run a simple C program in protected mode under DPMI
Andrew Schulman, February 1993
from "Undocumented DOS", 2nd edition (Addison-Wesley, 1993)
(Changed version of DPMISHEL, from MSJ, Oct. 1992, Dec. 1992)

Does Ctrl-C handling: see CTRL_C.ASM
Must be compiled small model to use C run-time library
Calls real_main(), switches into protected mode, then calls 
pmode_main()

To build a DPMI app:
bcc -2 -DDPMI_APP foo.c dpmish.c ctrl_c.asm
*/

#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include "dpmish.h"

void _dos_exit(int retval)
{
    _asm mov ah, 04ch
    _asm mov al, byte ptr retval
    _asm int 21h
}

// Call the DPMI Mode Detection function (INT 2Fh AX=1686h) to see
// if we are *already* running in protected mode under DPMI.
// See 3.1 DDK _Device Driver Adaptation Guide_, pp. 585-586
int dpmi_present(void)
{
    unsigned _ax;
    _asm mov ax, 1686h
    _asm int 2Fh
    _asm mov _ax, ax
    return (! _ax);
}

// Call the DPMI function for Obtaining the Real to Protected Mode
// Switch Entry Point (INT 2Fh AX=1687h), to determine if DPMI is
// available and, if so, switch into protected mode by calling
// the Switch Entry Point.  See DPMI 0.9 spec.
int dpmi_init(void)
{
    void (far *dpmi)();
    unsigned hostdata_seg, hostdata_para, dpmi_flags;
    _asm push si
    _asm push di
    _asm mov ax, 1687h           /* test for DPMI presence */
    _asm int 2Fh
    _asm and ax, ax
    _asm jz gotdpmi              /* if (AX == 0) DPMI is present */
    _asm jmp nodpmi
gotdpmi:        
    _asm mov dpmi_flags, bx
    _asm mov hostdata_para, si   /* paras for DPMI host private data */
    _asm mov word ptr dpmi, di
    _asm mov word ptr dpmi+2, es /* DPMI protected mode switch entry point */
    _asm pop di
    _asm pop si

    if (_dos_allocmem(hostdata_para, &hostdata_seg) != 0)
        fail("can't allocate memory");
    
    dpmi_flags &= ~1;   /* this is a 16-bit protected mode program */
    
    /* enter protected mode */
    _asm mov ax, hostdata_seg
    _asm mov es, ax
    _asm mov ax, dpmi_flags
    (*dpmi)();
	_asm jc nodpmi
	return dpmi_present();	/* double check */
    /* in protected mode now:  segment registers changed! */
nodpmi:
    return 0;
}

void dpmi_setprotvect(int intno, void (interrupt far *func)(void))
{
    _asm mov ax, 0205h
    _asm mov bl, byte ptr intno
    _asm mov cx, cs     /* word ptr func+2 */
    _asm mov dx, word ptr func
    _asm int 31h
}

// INT 23h handler under DPMI can't do the usual DOS INT 23h stuff
// needs to be on page locked with 31/0600??
// problem:  compiler has hard-wired (real mode) _loadds!

#if 1
#ifdef __cplusplus
extern "C" void interrupt far ctrl_c(void);
#else
extern void interrupt far ctrl_c(void);
#endif
#else
int ctrl_c_hit = 0;

void interrupt far ctrl_c(void)
{
    ctrl_c_hit++;
}
#endif

main(int argc, char *argv[])
{
    int ret;

    // actually, if already in pmode, real_main() still runs
    // (this is for debugging under 286|DOS-Extender)
    if (real_main(argc, argv) != 0)
        return 1;
    flushall(); // flush all buffers before switch into protected mode

    if (dpmi_present())        // if not already in protected mode...
        printf("Already in protected mode\n");
    else if (dpmi_init())      // switch into protected mode, via DPMI
        printf("Switched into protected mode via DPMI\n");
    else
        fail("This program requires DPMI");   
        
    // now in protected mode: segment registers have changed
        
    dpmi_setprotvect(0x23, ctrl_c); // install Ctrl-C handler
    ret = pmode_main(argc, argv);   // call the application's pmode_main
    flushall();                     // flush all buffers before exiting
    _dos_exit(ret);                 // must exit via 21/4C!
}
