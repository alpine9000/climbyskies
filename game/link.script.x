MEMORY 
{
    disk: org = 0x4000, len = 901120-0x4000
    ram: org = 0x00000, len = 0x80000
}

SECTIONS
{
    load : { 
        startCode = .;
        _startCode = .;
        *(CODE)
        *(.text) 
        *(.data)
	*(data_c)
        *(DATA)
        *(DATA_C)
	*(CHIP_DATA)
	*(COMMON)
        endCode = .;
    } > disk

    noload ALIGN(512) : {
        startData = .;
        *(.noload)
        *(noload)
        endData = .;
    } > disk

    bss (NOLOAD) : {
        . = endCode;
	_startBSS = .;
        *(.bss)
	*(bss)
	*(bss_c)
        *(BSS)
        *(BSS_C)
	*(CHIP_BSS)
	_endBSS = .;
	endRam = .;
    } > ram;
}