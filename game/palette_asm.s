	include 'includes.i'
	xdef _palette_install
	
_palette_install:
	move.l	a6,-(sp)
	lea	CUSTOM,a6
	include	"out/sprite-palette.s"
	move.l	(sp)+,a6
	rts