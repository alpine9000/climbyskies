	include 'includes.i'
	xdef _palette_install
	
_palette_install:
	move.l	a6,-(sp)
	lea	CUSTOM,a6
	include	"out/sprite-palette.s"
	move.w  #$09e,COLOR00(a6)
	move.l	(sp)+,a6
	rts