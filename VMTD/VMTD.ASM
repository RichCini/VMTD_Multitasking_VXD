PAGE 58,132
;******************************************************************************
TITLE VMTD - MultiTasking DOS VxD
;******************************************************************************
;
;   Title:      VMTD.ASM - MultiTasking DOS VxD (JulieEli)
;
;   Version:    1.10
;  
;   VxD that allows running/switching VMs without the Windows GUI
;
;   All rights reserved, James Eli  2/10/96
;
;   WARNING: Not tested with Windows95, and come to think of it, not tested
;   much at all!  Use at your own risk.  Also, uses nonregistered Device ID!
;
;   Compile with:
;   Link with:
;
;   To utilize this:  First rename COMMAND.COM > KRNL386.EXE, and place it in
;   the \WINDOWS\SYSTEM directory (save the actual KRNL386.EXE file).  Make
;   sure your SYSTEM.INI utilizes the standard MS VDDVGA VxD and drivers for
;   the video (this is important, or when switching back to the SysVM things
;   will be blank!).  Add a line in the SYSTEM.INI file under the [386Enh]
;   section like this: 'device=c:\WINDOWS\VMTD.386'  Start up windows 3.xx 
;   as usual with WIN.COM, but instead of seeing the familiar windows GUI,
;   you'll be looking at the old DOS prompt, yet its all there; VMM, etc.
;   Just run the CVM.EXE program in the SysVM (SHELL VxD provides no V86 API),
;   and a new VM will be created (run VMID for proof).  Use CTRL-PgUp hotkey
;   combo to hot-switch between VMs.  You've now got a multitasking DOS OS 
;   without all of the windows overhead--exactly what Windows is all about!
;
; 2019/09/19 - revised to add defined constants to MT_HotKey_Handler
; 2023/11/03 - added random commentary.
;
;==============================================================================

	.386p

;******************************************************************************
;                             I N C L U D E S
;******************************************************************************

        .XLIST
        INCLUDE VMM.INC
        INCLUDE VKD.INC
	INCLUDE VDD.INC
	INCLUDE DEBUG.INC
        .LIST

;******************************************************************************
;                               E Q U A T E S
;******************************************************************************
MT_DEVICE_ID	EQU 03C8Dh	;; my VXDID
MAJOR_VERSION	EQU 1
MINOR_VERSION	EQU 1

;******************************************************************************
;              V I R T U A L   D E V I C E   D E C L A R A T I O N
;------------------------------------------------------------------------------
; The VxD declaration statement defines the VxD name, version number,
; control proc entry point, VxD ID, initialization order, and VM API 
; entry points. What follows is a minimal VxD declaration, defining 
; only the name, version, control proc, and an undefined ID. Depending
; on the requirements of the VxD, the following may be added:
;
; - Defined VxD ID: See VXDID.TXT for more information
; - Init order: If your Vxd MUST load before or after a specific VxD,
;               the init order can be defined. See VMM.INC for the
;               definition of the init order of the standard devices.
; - V86,PM API: You may wish to allow application or library code running
;               in a virtual machine to communicate with your VxD directly.
;               See the chapter entitled "VxD APIs (Call-Ins)" in the
;               Virtual Device Adaptation Guide.
;               
;******************************************************************************
Declare_Virtual_Device JulieEli, MAJOR_VERSION, MINOR_VERSION, \
    MT_Control_Proc, MT_DEVICE_ID, Undefined_Init_Order, \
    MT_PM_API_Proc, MT_PM_API_Proc

;******************************************************************************
;                                D A T A
;******************************************************************************
;----------- VXD IDATA
VxD_IDATA_SEG
;       Initialization data here - discarded after Init_Complete
VxD_IDATA_ENDS


;----------- VXD DATA
VxD_DATA_SEG

ghhkCtrlPgUp	dd 00000000h               ; Hotkey handle

MT_PM_Call_Table LABEL DWORD
	dd      OFFSET32 MT_Get_Version     ; PMAPI #0
	dd      OFFSET32 MT_HotKey_Handler  ; PMAPI #1
Max_MT_PM_Service equ ($-MT_PM_Call_Table)/4

VxD_DATA_ENDS


;----------- VXD LOCKED_DATA
VxD_LOCKED_DATA_SEG
;       Pagelocked data here - try to keep this to a minimum.
VxD_LOCKED_DATA_ENDS


;******************************************************************************
;                               C O D E
;------------------------------------------------------------------------------
; The 'body' of the VxD would typically be in the standard code segment.
;******************************************************************************
VxD_CODE_SEG
;******************************************************************************
;
;   VXD_PM_API_Proc
;
;   DESCRIPTION:
;       This is the PM_API dispatch routine which dispatches API calls to the
;	proper API routine. 
;
;   ENTRY:
;	EBX: Current VM handle
;	EBP -> client register structure
;		.AX: function code
;
;   EXIT:
;       Carry set on error; client_EAX set to -1
;
;   USES:
;
;==============================================================================
BeginProc MT_PM_API_Proc, PUBLIC
	movzx	eax,[ebp.Client_AX]		; function in AX
	cmp 	eax,Max_MT_PM_Service           ; illegal call?
	jae 	SHORT PMCallBad
	and 	[ebp.Client_EFLAGS],NOT CF_Mask ; clear carry
	call 	MT_PM_Call_Table[eax*4]		; call service
	ret

PMCallBad:
	mov	[ebp.Client_EAX],0FFFFFFFFh
	or  	[ebp.Client_EFLAGS],CF_Mask     ; set carry
	ret
EndProc MT_PM_API_Proc

;******************************************************************************
;
;   VXD_Get_Version
;
;   DESCRIPTION:
;       Get driver version.
;
;   ENTRY:
;
;   EXIT:
;
;   USES:
;       flags
;
;==============================================================================
BeginProc MT_Get_Version
	VMMcall Get_Cur_VM_Handle           
	mov 	eax,MAJOR_VERSION SHL 8 or MINOR_VERSION
	mov 	[ebp.Client_CX],ax              ; cx = vxd version number
	mov 	[ebp.Client_EAX],ebx            ; eax = current vm handle
	ret 
EndProc MT_Get_Version

;******************************************************************************
;
;   VXD_HotKey_Handler
;
;   DESCRIPTION:
;       Handle hot keys
;
;   ENTRY:
;
;   EXIT:
;
;   USES:
;       flags
; NOTE: check to see if the call to TimeSlicePriority compiles with OK values.
;==============================================================================
BeginProc MT_HotKey_Handler
	VMMcall Get_Cur_VM_Handle
	VMMcall Get_Next_VM_Handle		; saves to EBX
	mov 	eax,1
    	push 	eax
	mov	ecx,BLOCK_SVC_INTS+BLOCK_ENABLE_INTS	;05h
    	VMMcall Begin_Critical_Section
    	mov 	eax,SET_DEVICE_FOCUS		;0fh
    	mov 	edx,VKD_DEVICE_ID		;0dh
    	VMMcall System_Control
   	mov 	eax,SET_DEVICE_FOCUS		;0fh
    	mov 	edx,REBOOT_DEVICE_ID		;0ah
    	VMMcall System_Control
    	mov 	eax,SET_DEVICE_FOCUS		;0fh
    	mov 	edx,VDD_DEVICE_ID		;0ch
    	VMMcall System_Control
    	push 	ebx
    	VMMcall Get_Sys_VM_Handle
    	VMMcall Get_Time_Slice_Priority
    	and 	eax, NOT(VMSTAT_High_Pri_Back)	;0fffffdffh
    	or 	eax,VMSTAT_Background		;2
    	VMMcall Set_Time_Slice_Priority
    	pop 	ebx
    	VMMcall Set_Execution_Focus
    	VMMcall Get_Sys_VM_Handle
    	VMMcall End_Critical_Section
    	pop 	eax
    	ret
EndProc MT_HotKey_Handler

VxD_CODE_ENDS

;******************************************************************************
;                      P A G E   L O C K E D   C O D E
;------------------------------------------------------------------------------
;       Memory is a scarce resource. Use this only where necessary.
;******************************************************************************
VxD_LOCKED_CODE_SEG
;******************************************************************************
;
;   VXD_Control
;
;   DESCRIPTION:
;
;       This is a call-back routine to handle the messages that are sent
;       to VxD's to control system operation. Every VxD needs this function
;       regardless if messages are processed or not. The control proc must
;       be in the LOCKED code segment.
;
;       The Control_Dispatch macro used in this procedure simplifies
;       the handling of messages. To handle a particular message, add
;       a Control_Dispatch statement with the message name, followed
;       by the procedure that should handle the message. 
;
;       A VxD is not required to handle any messages.
;
;   ENTRY:
;       EAX = Message number
;       EBX = VM Handle
;
;==============================================================================
BeginProc MT_Control_Proc
	Control_Dispatch Sys_Critical_Init,	MT_Sys_Critical_Init
	Control_Dispatch Device_Init, 		MT_Device_Init
	Control_Dispatch Init_Complete,		VMTD_Init_Complete
	clc
	ret
EndProc MT_Control_Proc

VxD_LOCKED_CODE_ENDS


;******************************************************************************
;                  I N I T I A L I Z A T I O N   C O D E
;------------------------------------------------------------------------------
;    Code in the initialization segment is discarded after Init_Complete
;******************************************************************************
VxD_ICODE_SEG
;******************************************************************************
;
;   VXD_Sys_Critical_Init
;
;   DESCRIPTION:
;       This code is called during the first phase of PM initialization.
;
;   ENTRY:
;       EBX = System VM handle
;
;   EXIT:
;       Carry clear to indicate load success
;       Carry set to abort loading this VxD
;
;   USES:
;       flags
;
;==============================================================================
BeginProc MT_Sys_Critical_Init
	Trace_Out "VMTD: Sys Critical Init"
	clc                                 ; nothing to do here
	ret
EndProc MT_Sys_Critical_Init


;******************************************************************************
;
;   VXD_Device_Init
;
;   DESCRIPTION:
;       Code called during the second phase of PM initialization.
;
;   ENTRY:
;       EBX = System VM handle
;
;   EXIT:
;       Carry clear to indicate load success
;       Carry set to abort loading this VxD
;
;   USES:
;       flags
;
;==============================================================================
BeginProc MT_Device_Init
	Trace_Out "VMTD: VxD Device Init"
    	VMMcall Get_VMM_Version 		; check VMM version
    	cmp 	ax,30ah
    	jb 	SHORT ExitFailure 		; 3.0 or earlier, so bug out...
    	VxDcall VDD_Get_Version 		; check VDD version
    	cmp 	ax,30ah 			; we're looking for MS VDDVGA.386
    	jne 	SHORT NoModifyVDDflag           
    	mov 	eax,[esi] 			; check VDD 8-byte ID string
    	cmp 	eax,'EDIV'			; should be = VIDEOVGA
    	jne 	SHORT NoModifyVDDflag
    	mov 	eax,[esi+04]
    	cmp 	eax,'AGVO'
    	jne 	SHORT NoModifyVDDflag
    	VMMcall Get_Sys_VM_Handle		; don't ask...   
    	and 	byte ptr [ebx+2e8h],0feh 	; alter NoSaveResSysVM flag value

NoModifyVDDflag:    
	mov 	al,49h 				; raw scan code of main key - hotkey=CTRL-PGUP
;    	mov 	al,0fh 				; hotkey = ALT-TAB
    	mov 	ah,ExtendedKey_B		; all standard hot key stuff...
;	ShiftState macro equates to mov ebx, HKSS_Ctrl
    	ShiftState <SS_Toggle_mask+SS_Either_Ctrl>,<SS_Ctrl>
    	mov 	cl,CallOnPress			; operation flags
    	mov 	esi,OFFSET32 MT_HotKey_Handler	; callback procedure
    	xor 	edx,edx				; reference data
    	xor 	edi,edi				; notification delay if cl=PriorityNotify
    	VxDcall VKD_Define_Hot_Key
    	jc  	SHORT ExitFailure
    	mov 	ghhkCtrlPgUp,eax		; handle
   	clc
    	ret

ExitFailure:
    	stc
    	ret
EndProc MT_Device_Init


;******************************************************************************
;
;   VXD_Init_Complete
;
;   DESCRIPTION:
;	Called during the final phase of PM init.
;
;   ENTRY:
;	EBX: Sys VM handle
;
;
;==============================================================================
BeginProc VMTD_Init_Complete

	Trace_Out "VMTD: Init Complete"
	clc
	ret

EndProc VMTD_Init_Complete
VxD_ICODE_ENDS

;******************************************************************************
;                       R E A L   M O D E   C O D E
;******************************************************************************

;******************************************************************************
;
;       Real mode initialization code
;
;   DESCRIPTION:
;       This code is called when the system is still in real mode, and
;       the VxDs are being loaded.
;
;   ENTRY:
;       AX = VMM Version
;       BX = Flags
;               Bit 0: duplicate device ID already loaded 
;               Bit 1: duplicate ID was from the INT 2F device list
;               Bit 2: this device is from the INT 2F device list
;       EDX = Reference data from INT 2F response, or 0
;       SI = Environment segment, passed from MS-DOS
;
;   EXIT:
;       BX = ptr to list of pages to exclude (0, if none)
;       SI = ptr to list of instance data items (0, if none)
;       EDX = DWORD of reference data to be passed to protect mode init
;
;==============================================================================
VXD_REAL_INIT_SEG

real_init	proc	near
    	mov 	ah,9				; Say hi, just for kicks
    	mov 	dx,offset SignOn1
    	int 	21h				; OK to use DOS interrupts here
    	mov 	ah,9
    	mov 	dx,offset SignOn2
    	int 	21h
    	xor 	bx,bx
    	xor 	si,si
    	xor 	edx,edx
    	mov 	ax,Device_Load_Ok
    	ret

SignOn1	db	"JulieEli VxD version 1.10",0dh,0ah,'$'
SignOn2	db	"All rights reserved, James Eli 1996",0dh,0ah,'$'

real_init	endp

VXD_REAL_INIT_ENDS

    END
