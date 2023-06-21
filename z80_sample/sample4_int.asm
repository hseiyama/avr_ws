	ORG	0000h
START:
	LD	SP,0100h
	XOR	A		; A <- 0
	LD	I,A
	IM	2
;	IM	1
;	IM	0
	EI
LOOP:
	IN	A,(03h)
	ADD	A,4
	OUT	(03h),A
	LD	(BUFF),A
	JR	LOOP
BUFF:
	DEFB	0FFh
;
	ORG	0030h
INT0:
	EI
	RETI
;
	ORG	0038h
INT1:
	EI
	RETI
;
	ORG	0040h
INT2:
	EI
	RETI
;
	ORG	0050h
VEC:
	DEFW	0040h
;
	ORG	0066h
NMI:
	RETN
	END
