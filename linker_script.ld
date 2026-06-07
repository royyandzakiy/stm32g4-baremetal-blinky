ENTRY(_reset);

/*
These go into .data
	int counter = 42;
	char message[] = "Hello";

Virtual address (VMA) 	-> Address where things are stored in RAM
	_sdata & _edata   	-> these are RAM addresses
	_sbss & _ebss   	-> these are RAM addresses
Load address (LMA)	  	-> Address where things are stored in Flash
	_sidata				-> flash address

FLASH (0x08000000):
┌──────────────┐
│  .vectors    │
├──────────────┤
│  .text       │  ← Your code
├──────────────┤
│  .rodata     │  ← Constants (read-only)
├──────────────┤
│ .data values │  ← LOADADDR(.data), where _sidata points
│   [42]       │  ← Value of 'counter'
│   ["Hello"]  │  ← Value of 'message'
└──────────────┘

RAM (0x20000000):
┌──────────────┐
│   .data      │  ← _sdata (copied here from Flash)
│   [42]       │  ← 'counter' at runtime
│   ["Hello"]  │  ← 'message' at runtime
├──────────────┤  ← _edata, _sbss (they're at the same address)
│   .bss       │
│   [0]        │  ← 'uninitialized_var' zeroed
│   [0]        │
├──────────────┤  ← _ebss
│              │
│   Stack      │
│      ↓       │
│              │
└──────────────┘ ← _estack

*/

MEMORY {
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512K
    RAM(rwx)  : ORIGIN = 0x20000000, LENGTH = 128K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

/*
.data section symbols
- _sdata   = Start of DATA in RAM (destination for copy)
- _edata   = End of DATA in RAM
- _sidata  = Start of Initialization DATA in Flash (source for copy)

.bss section symbols  
- _sbss    = Start of BSS in RAM
- _ebss    = End of BSS in RAM
- none	   = No _sibss needed! BSS just gets zeroed, no Flash copy needed

alt names
- _sidata     // STM32CubeMX style (most common for STM32)
- __data_load // Some GCC scripts
- _etext      // Some use end of .text as Flash source
- _data_lma   // LMA = Load Memory Address
- _data_flash // Descriptive name
*/

SECTIONS {
	/* Sections below each have their own sizes, hence the order is IMPORTANT! */
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
        _sdata = .; /* Start of .data in RAM */
        *(.data*)	/* Place data HERE in RAM space */
        _edata = .;	/* End of .data in RAM */
    } > RAM AT > FLASH
	
    _sidata = LOADADDR(.data);	/* Where `sidata = Start Initialization Data` .data's initial values are in Flash */
    
	/*
	what `> RAM` (VMA) and `AT > FLASH` (LMA) means
	_sdata = .; --> . refers to VMA
	
	startup code needs to
	- know where in RAM to put _sdata/_edata
	- know where in Flash the inital values _sidata are
	- copy from flash to ram before main() runs
		- LOADADDR() explicitly asks: "Hey linker, what's the LMA (Flash address) you assigned to this section?"

	Flash (LMA):
	0x08001000  ┌────────────┐ ← _sidata points here
				│ counter=42 │
				│ message[0] │
				│ message[1] │
	0x08001050  └────────────┘
				   ↓ COPY ↓
	RAM (VMA):      
	0x20000100  ┌────────────┐ ← _sdata points here
				│ counter=42 │
				│ message[0] │  
				│ message[1] │
	0x20000150  └────────────┘ ← _edata points here

	detail: read FLASH_RAM_COPY.md
	*/

    .bss : {
        _sbss = .;	/* Start of .bss in RAM (after .data) */
        *(.bss*)	/* All uninitialized data */
        *(COMMON)	/* Tentative definitions */
        _ebss = .;	/* End of .bss in RAM */
    } > RAM
}