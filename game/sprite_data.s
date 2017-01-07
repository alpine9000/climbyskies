	xdef	_spriteBackground0
	xdef	_spriteBackground1
	xdef	_spriteBackground2
	xdef	_spriteBackground3	

	align 4
_spriteBackground0:
	dc.w	$1d48,$ff02
	incbin	"out/sprite_background-0.bin"
	dc.w	0,0
_spriteBackground1:
	dc.w	$1d50,$ff02
	incbin	"out/sprite_background-1.bin"
	dc.w	0,0
_spriteBackground2:
	dc.w	$1d58,$ff02
	incbin	"out/sprite_background-2.bin"
	dc.w	0,0
_spriteBackground3:
	dc.w	$1d60,$ff02
	incbin	"out/sprite_background-3.bin"
	dc.w	0,0			