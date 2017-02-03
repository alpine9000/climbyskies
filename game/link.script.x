MEMORY 
{
    disk: org = 0x4000, len = 901120-0x4000
    ram: org = 0x00000, len = 0x160000
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
        *(.bss)
	*(bss)
	*(bss_c)
        *(BSS)
        *(BSS_C)
	*(CHIP_BSS)
	endRam = .;
    } > ram;
}