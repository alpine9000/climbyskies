	xdef	_spriteBackground0
	xdef	_spriteBackground1
	xdef	_spriteBackground2
	xdef	_spriteBackground3
	xdef	_spriteBackground4
	xdef	_spriteBackground5
	xdef	_nullSprite
	
	align 4
_spriteBackground0:
	dc.w	0,0
	incbin	"out/sprite_background-0.bin"
	dc.w	0,0
_spriteBackground1:
	dc.w    0,0
	incbin	"out/sprite_background-1.bin"
	dc.w	0,0
_spriteBackground2:
	dc.w    0,0
	incbin	"out/sprite_background-2.bin"
	dc.w	0,0
_spriteBackground3:
	dc.w	0,0
	incbin	"out/sprite_background-3.bin"
	dc.w	0,0
_spriteBackground4:
	dc.w	$1d40,$ff02
	incbin	"out/sprite_background-4.bin"
	dc.w	0,0
_spriteBackground5:
	dc.w	$1dA8,$ff02
	incbin	"out/sprite_background-4.bin"
	dc.w	0,0
_nullSprite:
	dc.l	0
