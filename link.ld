ENTRY(_reset);

MEMORY {
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512K
    RAM(rwx)  : ORIGIN = 0x20000000, LENGTH = 128K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS {
    .vectors : {
        KEEP(*(.vectors))
    } > FLASH
    
    .text : {
        *(.text*)
    } > FLASH
    
    .rodata : {
        *(.rodata*)
    } > FLASH
    
    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH
    _sidata = LOADADDR(.data);
    
    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}