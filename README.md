# Multitasking DOS VxD
## VMTD
This is a repository of the source files for a Windows 3.1 virtual device driver
(VxD) which essentially hacks Enhanced Mode Windows 3.1 to become a basic 
multitasking version of MS-DOS (although, at best, a proof-of-concept). All of
the underpinnings of Windows are there, but it has a regular COMMAND.COM interface. 

The code, written in 1996 by James Eli, is for several utilities including a 
virtual device driver and four utilities to manage creating and switching 
the virtual machines. It uses INT 2Fh multiplex calls and undocumented calls
to the Shell VxD to work its magic.

I downloaded this code many years ago based on an article in one of my usual
computer programming magazines. Unfortunately, I can neither locate the original
article copy nor remember which magzaine it was in...my guess is either Dr. Dobbs,
PC World, or PC/Magazine. Something this unsanctioned wouldn't be allowed in the
Microsoft Systems Journal!
