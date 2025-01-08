/*
DPMISH.H -- see DPMISH.C
Andrew Schulman, February 1993
from "Undocumented DOS", 2nd edition (Addison-Wesley, 1993)
*/

#ifdef __BORLANDC__
#ifndef __SMALL__
#error DPMISH requires small model
#endif
#else /* Microsoft C */
#ifndef M_I86SM
#error DPMISH requires small model
#endif
#endif

#ifndef DPMI_APP
#define DPMI_APP
#endif

#ifdef __BORLANDC__
#define _dos_allocmem(x,y)  (allocmem(x,y) != -1)
#endif

void _dos_exit(int retval);
int dpmi_present(void);
int dpmi_init(void);

/* unfortunately, the app has to check Ctrl-C itself! */
extern int ctrl_c_hit;

/* these functions to be defined by app */
extern int real_main(int argc, char *argv[]);
extern int pmode_main(int argc, char *argv[]);
extern void fail(const char *s, ...);  // app's fail() must call _dos_exit()
