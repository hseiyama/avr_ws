	ORG	0000h
START:
	IN	A,(03h)
	ADD	A,4
	OUT	(03h),A
	JR	START
	END
