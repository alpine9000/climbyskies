USE_GCC=1
USE_BIG=1
SFX=0
PLACEHOLDER_MUSIC=0
GAME_NAME=climbyskies

# userstack used by bootblock
USERSTACK_ADDRESS=7fffc
# if you change this you must also change link.script.x
BASE_ADDRESS=4000
# note: this must be high enough not to conflict with MFMbufE
BOOTBLOCK_ASM=alpine_bootblock.s
C_SRCS=init.c game.c gfx.c screen.c mouse.c tile.c sprite.c player.c cloud.c palette.c level.c disk.c enemy.c item.c
SRCS=hw_asm.s trackloader.s palette_asm.s  P6112-Play.s  blittext.s music.s


MAX_P61_SIZE=$(shell ls -Sl assets/P61.climby* | awk '{print $$5}' | head -1)

IMAGES=sprite.png font8x8.png
IMAGEDATA=$(addprefix out/, $(IMAGES:.png=.bin))
ASSETS=$(IMAGEDATA) out/fade_in.s

LINKER_OPTIONS=-T link.script.x
VASM_ARGS=-DUSE_GCC=$(USE_GCC) -Fhunk -quiet -esc -DPLACEHOLDER_MUSIC=$(PLACEHOLDER_MUSIC) -DSFX=$(SFX) -allmp -DMAX_P61_SIZE=$(MAX_P61_SIZE)
VASM_C_ARGS=-ldots -Fhunk -quiet -no-opt
AMIGA_INCLUDE=/usr/local/amiga/m68k-amigaos/ndk/include

ADF=bin/$(GAME_NAME).adf
MODULE=$(GAME_NAME).s
PROGRAM_BIN=out/main.bin

VASM=/usr/local/amiga/vgcc/bin/vasmm68k_mot
LD=/usr/local/amiga/vgcc/bin/vlink
EXTRA_DEFINES=

ifeq ($(USE_GCC), 1)
CC=/usr/local/amiga/vgcc/bin/m68k-amigaos-gcc
REGPARAM=-mregparm=2
#VASM_OPTS=-Wa,-no-opt
VASM_OPTS=-Wa,-opt-allbra -Wa,-opt-brajmp -Wa,-opt-lsl  -Wa,-opt-movem -Wa,-opt-mul -Wa,-opt-div -Wa,-opt-pea #-Wa,-opt-speed

ifeq ($(USE_BIG), 1)
WHOLE_PROGRAM=-fwhole-program
endif

C_OPT=-O3 $(WHOLE_PROGRAM)
CFLAGS=$(C_OPT) $(REGPARAM) $(VASM_OPTS) -fomit-frame-pointer -noixemul -mcpu=68000 -m68000 -pedantic-errors -Wfatal-errors -Wall -Werror -Wextra -Wshadow -I /usr/local/amiga/m68k-amigaos/ndk/include/ $(EXTRA_DEFINES)
C_LIBS=libgcc/out/gcc.lib
else
CC=vc
CFLAGS=-warn=-1 -dontwarn=81 -dontwarn=170 -dontwarn=306 -dontwarn=166 -dontwarn=167  -dontwarn=307 -dontwarn=208 -maxoptpasses=100 -c99 -O3 $(EXTRA_DEFINES) #-fastcall
C_LIBS=/usr/local/amiga/m68k-amigaos/vbcc/lib/vcs.lib
endif

GCC_CHECK_CFLAGS= -pedantic-errors -Wfatal-errors -Wall -Werror -Wextra -Wshadow -o /tmp/blah.o -c -I /usr/local/amiga/m68k-amigaos/ndk/include/ -DGCC_CHECK

ASM_OBJS=$(SRCS:.s=.o)
C_OBJS=$(C_SRCS:.c=.o)
ifeq ($(USE_BIG), 1)
OBJS=$(ASM_OBJS) big.o #$(C_OBJS) 
else
OBJS=$(ASM_OBJS) $(C_OBJS) 
endif
ADF_C_OBJS=$(addprefix out/adf/, $(C_OBJS)) 
ADF_OBJS=$(addprefix out/adf/, $(OBJS)) 
WORKBENCH_C_OBJS=$(addprefix out/workbench/, $(C_OBJS)) 
WORKBENCH_OBJS=$(addprefix out/workbench/, $(OBJS)) 
ALL_OBJS=out/adf/main.o $(ADF_OBJS)
ALL_WORKBENCH_OBJS=out/workbench/main.o $(WORKBENCH_OBJS)
ALL_DEPENDS=$(ALL_OBJS:.o=.d) $(ALL_WORKBENCH_OBJS:.o=.d)


all: adf 

include ../shared/externals.mk

adf: libgcc/out/gcc.lib version bin out $(ALL_TOOLS) $(ADF)

.PHONY: version
version:
	@echo $(ALL_TOOLS)
	#./version/version.sh

gdrive: all
	cp $(ADF) ~/Google\ Drive

test: all
	cp $(ADF) ~/Projects/amiga/test.adf

libgcc/out/gcc.lib:
	make -C libgcc

go: test
	 $(RUN_SCRIPT)

goaga: test
	 $(A1200T_RUN_SCRIPT)

list:
	m68k-amigaos-objdump  -b binary --disassemble-all out/bootblock.bin -m m68k > out/bootblock.txt

bin:
	mkdir bin

out:
	mkdir out
	mkdir out/adf
	mkdir out/workbench

$(ADF): out/bootblock.bin
	$(MAKEADF) out/bootblock.bin > $(ADF)
	@ls -lh out/bootblock.bin

wb: version libgcc/out/gcc.lib bin/$(GAME_NAME)

out/adf/level.o: out/background-map.c level.c
out/workbench/level.o: out/background-map.c level.c

out/background-map.c: assets/background.tmx
	../tools/mapgen/out/mapgen --c --depth=5 --input=$<
	@mv background-map.c out
	@mv item-map.c out
	@rm background-indexes.c
	@rm item-indexes.c

out/sprite.bin: assets/sprite.png palette.pal
	$(IMAGECON) --quantize --colors=32 --full-color-palette --input=assets/sprite.png --output=out/sprite --output-bitplanes --output-png --output-palette --output-palette-asm --use-palette=palette.pal --output-mask --transparent-color=000,144,224
out/bootblock.bin: out/bootblock.o
	$(LD) -brawbin1 $< -o $@

out/font%.bin: assets/font%.png palette.pal
	$(IMAGECON) --input $< $(IMAGECON_ARGS) --output-bitplanes --output-grey-palette-asm --output-palette-asm --output-palette --output out/font$* --colors=2 --use-palette=palette.pal --full-color-palette-file --output-mask --transparent-color=0,0,0 --output-png

out/fade_in.s: ./palette.pal
	../tools/fade/out/fade --steps=64 --colors=32 --from-black --to=palette.pal --output=fadeIn > $@

out/bootblock.o: $(BOOTBLOCK_ASM) $(PROGRAM_BIN)
	$(VASM) $(VASM_ARGS)  -DUSERSTACK_ADDRESS="\$$$(USERSTACK_ADDRESS)" -DBASE_ADDRESS="\$$$(BASE_ADDRESS)"  $< -o $@ -I$(AMIGA_INCLUDE)

out/adf/main.o: $(MODULE) $(EXTRA) $(ASSETS) $(IMAGEDATA)
	$(VASM)  -DTRACKLOADER=1  $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE)
	@$(VASM) -depend=make -DTRACKLOADER=1 $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE) > $*.d

out/workbench/main.o: $(MODULE) $(EXTRA) $(ASSETS)
	$(VASM)  -DTRACKLOADER=0 $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE)
	@$(VASM) -depend=make -DTRACKLOADER=0 $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE) > $*.d

out/adf/%.o: %.s
	$(VASM) $(VASM_ARGS)  -DTRACKLOADER=1 $< -o $@ -I$(AMIGA_INCLUDE)
	@$(VASM) -depend=make -DTRACKLOADER=1 $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE) > out/adf/$*.d

out/adf/%.o: out/%.c
ifeq ($(USE_GCC), 1)
	-@$(CC) -S $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) $< -o out/adf/$*.s 
	$(CC) -c $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) $< -o out/adf/$*.o
else
	$(CC) -S $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE)  $< -o out/adf/$*.s 
	$(CC) $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) -c $< -o $@
endif


out/workbench/%.o: out/%.c
	$(CC) $(CFLAGS) -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) -c $< -o $@

out/adf/big.o: $(ADF_C_OBJS)$(C_SRCS) out/background-map.c
	cat $(C_SRCS) > big.c
	$(CC) -c $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) big.c -o $*.o
	-@$(CC) -c $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) big.c -S -o out/adf/big.s
	-@rm big.c

out/workbench/big.o: $(WORKBENCH_C_OBJS)$(C_SRCS) out/background-map.c
	cat $(C_SRCS) > big.c
	$(CC) -c $(CFLAGS) -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) big.c -o $*.o
	-@rm big.c

out/adf/%.o: %.c
ifeq ($(USE_GCC), 1)
	-@$(CC) -S $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) $< -o out/adf/$*.s 
	$(CC) -c $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) $< -o out/adf/$*.o
	@$(CC) -MM $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) $< -o out/adf/$*.d
	@mv -f out/adf/$*.d $*.d.tmp
	@sed -e 's|.*:|out/adf/$*.o:|' < $*.d.tmp > out/adf/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/:/' >> out/adf/$*.d
	@rm -f $*.d.tmp
else
	gcc $< $(EXTRA_DEFINES) -DTRACKLOADER=1 $(GCC_CHECK_CFLAGS)
	-@$(CC) -deps $(CFLAGS) -S -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) $< -o out/adf/$*.s > /dev/null 2> /dev/null
	-@/bin/echo -n "out/adf/" | cat - $*.dep > out/adf/$*.d
	-@rm $*.dep
	$(CC) $(CFLAGS) -DTRACKLOADER=1 -I$(AMIGA_INCLUDE) -c $< -o $@
endif

out/workbench/%.o: %.c
ifeq ($(USE_GCC), 1)
	$(CC) -c $(CFLAGS) -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) $< -o out/workbench/$*.o
	@$(CC) -MM $(CFLAGS) -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) $< -o out/workbench/$*.d
	@mv -f out/workbench/$*.d $*.d.tmp
	@sed -e 's|.*:|out/workbench/$*.o:|' < $*.d.tmp > out/workbench/$*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/:/' >> out/workbench/$*.d
	@rm -f $*.d.tmp
	-@$(CC) -S $(CFLAGS) -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) $< -o out/workbench/$*.s 
else
	gcc $< $(EXTRA_DEFINES) -DTRACKLOADER=0 $(GCC_CHECK_CFLAGS)
	-@$(CC) -deps $(CFLAGS) -S -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) $< -o out/workbench/$*.s > /dev/null 2> /dev/null
	-@/bin/echo -n "out/workbench/" | cat - $*.dep > out/workbench/$*.d
	-@rm $*.dep
	$(CC) $(CFLAGS) -DTRACKLOADER=0 -I$(AMIGA_INCLUDE) -c $< -o $@
endif

out/workbench/%.o: %.s
	$(VASM)  -DTRACKLOADER=0 $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE)
	@$(VASM) -depend=make  -DTRACKLOADER=1 $(VASM_ARGS) $< -o $@ -I$(AMIGA_INCLUDE) > out/workbench/$*.d

out/main.bin: out/adf/main.o $(ADF_OBJS)
	$(LD) $(LINKER_OPTIONS) -brawbin1 $< $(ADF_OBJS) $(C_LIBS) -o $@ 
	@$(LD) $(LINKER_OPTIONS) -brawbin1 $< $(ADF_OBJS) $(C_LIBS) -o $@  -M -o /tmp/main.bin > out/adf/main.txt
	@$(LD) $(LINKER_OPTIONS) -brawbin1 $< $(ADF_OBJS) $(C_LIBS) -M -o /tmp/main.bin | grep ", value " | cut -d " " -f3,7 | cut -d "," -f1 > $@.symbols
	@cp $@.symbols ~/Projects/amiga/debugger.syms
	@echo "RAM USAGE:" $$((16#`cat ~/Projects/amiga/debugger.syms | grep endRam | sed 's/0x//' | sed 's/endRam: //'`)) bytes
ifeq ($(USE_BIG), 1)	
	-@rm out/adf/big.o
endif

bin/$(GAME_NAME): bin out $(ALL_TOOLS)  $(ALL_WORKBENCH_OBJS) 
	$(LD) $(C_LIBS)	-hunkattr bss_c=2 -hunkattr code_c=2 -hunkattr data_c=2  $(ALL_WORKBENCH_OBJS) -o $@
ifeq ($(USE_BIG), 1)	
	-@rm out/workbench/big.o
endif
	cp $@ ~/A1200/BlockySkies
	cp $@ ~/A4000/BlockySkies
        #cp assets/BlockySkies.info bin
        #cp assets/BlockySkiesDrawer.info bin
        #cp assets/BlockySkies.info ~/A1200
        #cp assets/BlockySkies.info ~/A4000
        #ls -lh bin/BlockySkies

.PHONY: clean
clean:	
	rm -rf out bin *~
	make -C libgcc clean

depend:
	echo $(ALL_DEPENDS)

-include $(ALL_DEPENDS)
$(ALL_OBJS): Makefile
$(ALL_WORKBENCH_OBJS): Makefile


include ../shared/run.mk