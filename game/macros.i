IntsOff:	macro
	move	#$7fff,INTENA(a6) 	; disable all interrupts
	endm
IntsOn:		macro
	move.w	#(INTF_SETCLR|INTF_VERTB|INTF_INTEN),INTENA(a6)			
	endm

P61Module: macro
	cnop	0,512	
diskmodule\1:
	incbin	\2
	cnop	0,512
enddiskmodule\1:
	endm