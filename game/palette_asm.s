	include 'includes.i'
	xdef _palette_install
        xdef _palette_menuInstall:

	section .text
_palette_install:
	move.l	a6,-(sp)
	lea	CUSTOM,a6
	include	"out/sprite-palette.s"
	move.l	(sp)+,a6
	rts

_palette_menuInstall:
	move.l	a6,-(sp)
	lea	CUSTOM,a6
	include	"out/menu-palette.s"
	move.l	(sp)+,a6
	rts