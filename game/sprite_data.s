	xdef	_spriteBackground0
	xdef	_spriteBackground1	

	align 4
_spriteBackground0:
	dc.w	$1d40,$ff02
	incbin	"out/sprite_background-0.bin"
	dc.w	0,0
_spriteBackground1:
	dc.w	$1d48,$ff02
	incbin	"out/sprite_background-1.bin"
	dc.w	0,0	