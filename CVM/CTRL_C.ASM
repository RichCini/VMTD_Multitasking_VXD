; CTRL_C.ASM -- for use with DPMISHEL

dosseg
.model small

public _ctrl_c_hit
public _ctrl_c

.data
_ctrl_c_hit dw  0

.code
_ctrl_c proc far
	; DS belongs to app right now
    inc _ctrl_c_hit
    iret
_ctrl_c endp
end
