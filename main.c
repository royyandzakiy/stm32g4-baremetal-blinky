#include <stdbool.h>
#include <stdint.h>

// GPIO and RCC register definitions
#define CONVERT_TO_32BIT(x) (1UL << (x))

/*
	uint16_t led = CREATE_PIN_MASK('B', 7);

	Step by step:
	('B' - 'A') = 1          	(bank index)
	1 << 8 = 0x100           	(bank in upper byte)
	0x100 | 7 = 0x107        	(pin number in lower byte)
	led = 0x0107

	GET_PIN_NO(0x0107) = 0x07 = 7    (pin number)
	GET_PIN_BANK(0x0107) = 0x01 = 1  (bank B)
*/
#define CREATE_PIN_MASK(bank, num) ((((bank) - 'A') << 8) | (num))
#define GET_PIN_NO(pin) ((pin) & 255)	// Extract pin number (lower 8 bits)
#define GET_PIN_BANK(pin) ((pin) >> 8)	// Extract bank index (upper 8 bits)

/*
	Returns pointer to GPIO bank's registers in the memory-mapped I/O region.
	bank: 0=GPIOA, 1=GPIOB, etc.
	Each GPIO bank occupies 1KB (0x400 bytes) of address space.

	calculates address for the selected GPIO Bank based on base pointer

	GPIO_Registers* gpioa = GET_GPIO_PORT(GET_PIN_BANK('A'));
	GPIO_Registers* gpioa = GET_GPIO_PORT(0);
	GPIO_Registers* gpioa = ((GPIO_Registers*)(0x48000000 + (0x400 * 0)));
*/

#define BASE_GPIO_REG 	0x48000000
#define GPIO_REG_OFFSET 0x400		// 1KB offset between GPIO banks
#define BASE_RCC_REG 	0x40021000

#define GET_GPIO_PORT(bank)	((GPIO_Registers *)(BASE_GPIO_REG + (GPIO_REG_OFFSET * (bank))))
#define GET_RCC				((RCC_Registers *)BASE_RCC_REG)

enum {
	GPIO_MODE_INPUT = 0,
	GPIO_MODE_OUTPUT,
	GPIO_MODE_AF,
	GPIO_MODE_ANALOG
};

// Memory-mapped I/O structures: Each member has fixed offset from the to be given base address
// these types are used to represent and with it read/wrire directly to the hardware registers
/*
	gpio->MODER;  // Actually accesses address 0x48000000 + 0x00
	gpio->OTYPER; // Actually accesses address 0x48000000 + 0x04
	gpio->OSPEEDR;// Actually accesses address 0x48000000 + 0x08
	gpio->BSRR;   // Actually accesses address 0x48000000 + 0x18
*/
typedef struct {
	volatile uint32_t MODER;	// Offset 0x00: Mode register
	volatile uint32_t OTYPER;	// Offset 0x04: Output Type register
	volatile uint32_t OSPEEDR;	// ...
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFRL;
	volatile uint32_t AFRH;
	volatile uint32_t BRR;
} GPIO_Registers;

typedef struct {
	volatile uint32_t CR;
	volatile uint32_t ICSCR;
	volatile uint32_t CFGR;
	volatile uint32_t PLLCFGR;
	uint32_t RESERVED0[2];
	volatile uint32_t CIER;
	volatile uint32_t CIFR;
	volatile uint32_t CICR;
	uint32_t RESERVED1;
	volatile uint32_t AHB1RSTR;
	volatile uint32_t AHB2RSTR;
	volatile uint32_t AHB3RSTR;
	uint32_t RESERVED2;
	volatile uint32_t APB1RSTR1;
	volatile uint32_t APB1RSTR2;
	volatile uint32_t APB2RSTR;
	uint32_t RESERVED3;
	volatile uint32_t AHB1ENR;
	volatile uint32_t AHB2ENR;
	volatile uint32_t AHB3ENR;
	uint32_t RESERVED4;
	volatile uint32_t APB1ENR1;
	volatile uint32_t APB1ENR2;
	volatile uint32_t APB2ENR;
	uint32_t RESERVED5;
	volatile uint32_t AHB1SMENR;
	volatile uint32_t AHB2SMENR;
	volatile uint32_t AHB3SMENR;
	uint32_t RESERVED6;
	volatile uint32_t APB1SMENR1;
	volatile uint32_t APB1SMENR2;
	volatile uint32_t APB2SMENR;
	uint32_t RESERVED7;
	volatile uint32_t CCIPR;
	uint32_t RESERVED8;
	volatile uint32_t BDCR;
	volatile uint32_t CSR;
	volatile uint32_t CRRCR;
	volatile uint32_t CCIPR2;
} RCC_Registers;

int counter = 42;
char message[] = "Hello";

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
	GPIO_Registers *gpio = GET_GPIO_PORT(GET_PIN_BANK(pin));
	int n = GET_PIN_NO(pin);
	gpio->MODER &= ~(3U << (n * 2));
	gpio->MODER |= (mode & 3U) << (n * 2);
}

static inline void gpio_write(uint16_t pin, bool val) {
	GPIO_Registers *gpio = GET_GPIO_PORT(GET_PIN_BANK(pin));
	gpio->BSRR = (1U << GET_PIN_NO(pin)) << (val ? 0 : 16);
}

// counter is volatile, so will not be optimized by compiler
static inline void delay(volatile uint32_t counter) {
	while (counter--) {
		__asm__("nop");
	}
}

// Declare external symbols from linker script.
// _sbss/_ebss: start/end of .bss in RAM (uninitialized data)
// _sdata/_edata: start/end of .data in RAM (initialized data destination)
// _sidata: start of .data initial values in Flash (copy source)
extern uint32_t _sbss, _ebss, _sdata, _edata, _sidata;

extern void _estack(void); // declaring _estack as a function pointer to comply with the vector_table structure, it is a TRICK for the sake of putting it next to other interrupt handlers. the hardware will read it as a raw 32-bit address placed in the Stack Pointer (SP) register, used as initial stack pointer position right after boot (will change overtime as the Stack Pointer moves around). _estack gets calculated during linking process inside the linker script

extern int main(void);

// Reset handler: this is the entry point ENTRY(_reset) after power-on/reset
__attribute__((naked, noreturn)) void _reset(void) {
	// Initialize .bss to zero
	for (uint32_t *ram_memory_ptr = &_sbss; ram_memory_ptr < &_ebss; ram_memory_ptr++) {
		*ram_memory_ptr = 0;
	}

	// Copy .data from flash to RAM (initialized globals)
	for (uint32_t *ram_memory_ptr = &_sdata, *src = &_sidata; ram_memory_ptr < &_edata;) {
		*ram_memory_ptr++ = *src++;
	}

	// Calls main
	main();

	// Traps into infinite loop in case main returns (should never happen in embedded systems)
	for (;;) {
	}
}

void _default_handler(void) {
	for(;;) {} // trap here for debugging
}

// Vector table: STM32G4 has 16 system + 91 STM32 specific interrupts = 107
// detail: read VECTOR_TABLE.md
__attribute__((section(".vectors"))) void (*const vector_table[16 + 91])(void) = {
	_estack, 			// Pointer to Initial Stack
	_reset,	 			// Pointer to the _reset function (will be used as ENTRY)
  	_default_handler,	// NMI (optional)
	_default_handler,	// HardFault (optional)

	// ...the rest is left empty, meaning vector interrupts are uninitialized. 
	// hence, if we get an interrupt, it simply does nothing
};

int main() {
	uint16_t led = CREATE_PIN_MASK('A', 5); // A5 is the built-in LED for Nucleo-G474RET. turned into binary representation

	// Enable GPIOA clock by activating the correct RCC. activating GPIOA from PIN_BANK A
	// \*(volatile uint32_t)(0x40021000	+ 0x4C) 		|= 0x02
	// 						(RCC_BASE 	+ Correct Bank) |= RCC Bus A Bank
	GET_RCC->AHB2ENR |= CONVERT_TO_32BIT(GET_PIN_BANK(led));

	// Configure LED pin as output
	gpio_set_mode(led, GPIO_MODE_OUTPUT);
	
	gpio_write(led, false);
	if (counter == 42) {
		// verify if counter value is copied correctly in data section. if true, blink quickly 10 times
		for (uint8_t i=0; i<10; i++) {
			gpio_write(led, true);
			delay(100000);
			gpio_write(led, false);
			delay(100000);
		}
		
		// give a 3 sec wait
		delay(3000000);
	}

	for (;;) {
		gpio_write(led, true);
		delay(1000000);
		gpio_write(led, false);
		delay(1000000);
	}
}