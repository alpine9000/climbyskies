	include "includes.i"

	xdef _hw_waitBlitter
	xdef _hw_waitVerticalBlank
	xdef _hw_waitRaster
	xdef _hw_readJoystick
	xdef _hw_waitForJoystick
	xdef _hw_joystickButton
	xdef _hw_joystickPos	
	xdef _hw_interruptsInit
	xdef _hw_waitScanLines
	
ciaa_pra  = $bfe001
_joy1dat   = $dff00c
potgor    = $dff016
bit_joyb1 = 7
bit_joyb2 = 14
	
_hw_waitBlitter:
	move.l	a6,-(sp)
	lea 	CUSTOM,a6
	tst 	DMACONR(a6)		;for compatibility
.waitblit:
	btst 	#6,DMACONR(a6)
	bne.s 	.waitblit
	move.l	(sp)+,a6
	rts
	
_hw_waitVerticalBlank:	
	movem.l	d0,-(sp)
.loop:
	move.l	$dff004,d0
	and.l	#$1ff00,d0
	cmp.l	#303<<8,d0	; wait for the scan line
	bne.b	.loop
.loop2:
	move.l	$dff004,d0
	and.l	#$1ff00,d0
	cmp.l	#303<<8,d0	; wait for the scan line to pass (A4000 is fast!)
	beq.b	.loop2
	movem.l (sp)+,d0
	rts	


_hw_waitRaster:		;wait for rasterline d0.w. Modifies d0-d2/a0.
	movem.l d0-a6,-(sp)
	move.l #$1ff00,d2
	lsl.l #8,d0
	and.l d2,d0
	lea $dff004,a0
.wr:
	move.l (a0),d1
	and.l d2,d1
	cmp.l d1,d0
	bne.s .wr
	movem.l (sp)+,d0-a6
	rts

WaitScanLines:	 macro
	        if \1 != 0
	        lea     $dff006,a0
	        move.w  #\1-1,d2
	.\@nTimes:
	        move.w  (a0),d0
	        lsr.w   #8,d0
	.\@loop:
	        move.w  (a0),d1
	        lsr.w   #8,d1
	        cmp.w   d0,d1
	        beq     .\@loop
	        dbra    d2,.\@nTimes
	.\@done:
	        endif
	        endm
	
	
_hw_waitScanLines:
		movem.l	d0-d1/a0,-(sp)
	        lea     $dff006,a0
	.nTimes:
	        move.w  (a0),d0
	        lsr.w   #8,d0
	.loop:
	        move.w  (a0),d1
	        lsr.w   #8,d1
	        cmp.w   d0,d1
	        beq     .loop
	        dbra    d2,.nTimes
	.done:
		movem.l	(sp)+,d0-d1/a0
	rts
	
	
_hw_readJoystick:
	;; updates the joystick variables to contina the state of the buttons in bits 8 and 9,
        ;; and the lower byte holds the direction of the stick: 
	movem.l	d0/d1,-(sp)	
	btst    #bit_joyb2&7,potgor
        seq     d0
	add.w   d0,d0
	btst    #bit_joyb1,ciaa_pra
        seq     d0
	add.w   d0,d0
	move.w  _joy1dat,d1
	ror.b   #2,d1
	lsr.w   #6,d1
	and.w   #%1111,d1
	move.b	(.conv,pc,d1.w),d0
	move.w	d0,_hw_joystickButton
	movem.l (sp)+,d0/d1
        rts
.conv:
        dc.b      0,5,4,3,1,0,3,2,8,7,0,1,7,6,5,0

_hw_waitForJoystick:
.joystickPressed:	
	jsr	_hw_readJoystick
	move.w	#5-1,d0
.debounce:
	jsr	_hw_waitVerticalBlank
	dbra	d0,.debounce	; I have a bodgy joystick
	btst.b	#0,_hw_joystickButton
	bne	.joystickPressed
.wait:
	jsr	_hw_readJoystick
	jsr	_hw_waitVerticalBlank
	btst.b	#0,_hw_joystickButton
	beq	.wait
	rts
	
_hw_interruptsInit:
	movem.l	a0/a3/a6,-(sp)
	lea	CUSTOM,a6	
	lea	Level3InterruptHandler,a3
	move.l	vectorBase,a0
 	move.l	a3,LVL3_INT_VECTOR(a0)
	move.w	#(INTF_SETCLR|INTF_VERTB|INTF_INTEN),INTENA(a6)
	movem.l	(sp)+,a0/a3/a6
	rts

Level3InterruptHandler:
	movem.l	d0-a6,-(sp)
	lea	CUSTOM,a6
.checkVerticalBlank:
	move.w	INTREQR(a6),d0
	and.w	#INTF_VERTB,d0	
	beq	.checkCopper

.verticalBlank:
	move.w	#INTF_VERTB,INTREQ(a6)	; clear interrupt bit	
	;; add.l	#1,_verticalBlankCount
	;; MusicPlay
.checkCopper:
	move.w	INTREQR(a6),d0
	and.w	#INTF_COPER,d0	
	beq.s	.interruptComplete
.copperInterrupt:
	move.w	#INTF_COPER,INTREQ(a6)	; clear interrupt bit	
	
.interruptComplete:
	movem.l	(sp)+,d0-a6
	rte

	align 4
	
_hw_joystickButton:
	dc.b	0
_hw_joystickPos:
	dc.b	0


	