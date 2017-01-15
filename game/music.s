	include "includes.i"
	
	xdef 	_music_play

	align 4
p61Inited:
	dc.w	0


	align 4

_music_setVolume:
	move.w	d0,P61_Master
	rts

	align 4
	
_music_play:
	move.l	(4,a7),d0
	;; d0.w - module index
.wait: 				; In case there is currently  fade in progress
	jsr	_hw_waitVerticalBlank
	cmp.w	#0,P61_Master
	beq	.skip
	cmp.w	#64,P61_Master
	blt	.wait

	cmp.w	currentModule,d0
	beq	.skip
	cmp.w	#-1,currentModule
	beq	.fadeComplete

.fadeOutMusic:
	cmp.w	#0,P61_Master
	beq	.fadeComplete
	sub.w	#1,P61_Master
	jsr	_hw_waitVerticalBlank
	;; jsr	PlayNextSound
	bra	.fadeOutMusic
.fadeComplete:

	move.w	d0,currentModule
	movem.l	d0-a6,-(sp)
	cmp.w	#0,p61Inited
	beq	.skipEnd
	movem.l	d0-a6,-(sp)
	IntsOff
	jsr	P61_End
	IntsOn	
	movem.l	(sp)+,d0-a6	
	move.w	#0,p61Inited	
.skipEnd:
	lea	module,a0
	lea	modules,a1
	lsl.w	#3,d0
	adda.w	d0,a1
	move.l	(a1)+,d0
	move.l	(a1),a1
	jsr	_disk_LoadDiskData
	lea     module,a0
        sub.l   a1,a1
        sub.l   a2,a2
        moveq   #0,d0
	move.w	#64,P61_Master
	jsr     P61_Init
	move.w	#1,p61Inited
	movem.l	(sp)+,d0-a6
.skip:
	rts


currentModule:
	dc.w	-1


modules:
	dc.l	enddiskmoduleA-diskmoduleA
	dc.l	diskmoduleA

	dc.l	enddiskmoduleB-diskmoduleB
	dc.l	diskmoduleB

	dc.l	enddiskmoduleC-diskmoduleC
	dc.l	diskmoduleC
	
	section	bss_c
module:	
	ds.b	MAX_P61_SIZE
	ds.b	512
	ds.b	512	

	section	.noload
moduleDiskData:
	P61Module A,"assets/P61.climbyskies_ingame_00324_a_015.mod"
	P61Module B,"assets/P61.jmd-songB"
	P61Module C,"assets/P61.jmd-songC"
