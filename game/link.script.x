MEMORY 
{
    disk: org = 0x4000, len = 901120-0x4000
    ram: org = 0x00000, len = 0x160000
}

SECTIONS
{
    load : { 
        startCode = .;
        *(.text) 
        *(.data)
	*(data_c)
        *(CODE)
        *(DATA)
	*(CHIP_DATA)
        endCode = .;
    } > disk

    noload ALIGN(512) : {
        startData = .;
        *(.noload)
        endData = .;
    } > disk

    bss (NOLOAD) : {
        . = endCode;
        *(.bss)
	*(bss)
	*(bss_c)
        *(BSS)
	*(CHIP_BSS)
	endRam = .;
    } > ram;
}