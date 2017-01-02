	include "includes.i"

	xdef _custom
	xdef _spriteMask
	xdef _spriteFrameBuffer
	
	if TRACKLOADER=1
byteMap:
	dc.l	Entry
	dc.l	endCode-byteMap
	endif

	include "wbstartup.i"		; does nothing if TRACKLOADER=1
	
Entry:
	if TRACKLOADER=0
	jmp 	StartupFromOS
	else
	lea	userstack,a7	
	endif

Main:
	jsr	_init_amiga
	jsr	_game_init
	jsr	_game_loop
Loop:	
	bra 	Loop

	if TRACKLOADER=0
QuitGame:
	jmp	LongJump
	endif

	include "os.i"

	align 4
_spriteBitplanes:
	incbin	"out/sprite.bin"

spriteMask
	incbin	"out/sprite-mask.bin"
_spriteMask:
	dc.l	spriteMask
_spriteFrameBuffer:
	dc.l	_spriteBitplanes
	align 4
_custom:
	dc.l	CUSTOM
	
	section	.bss	
	align 4
	if TRACKLOADER=1
startUserstack:
	ds.b	1000
userstack:
	endif
	end