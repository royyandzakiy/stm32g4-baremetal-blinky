#include <stdbool.h>
#include <stdint.h>

// GPIO and RCC register definitions
#define BIT(x) (1UL << (x))
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) ((pin) & 255)
#define PINBANK(pin) ((pin) >> 8)

typedef struct {
	volatile uint32_t MODER;
	volatile uint32_t OTYPER;
	volatile uint32_t OSPEEDR;
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFRL;
	volatile uint32_t AFRH;
	volatile uint32_t BRR;
} GPIO_TypeDef;

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
} RCC_TypeDef;

#define GPIO(bank) ((GPIO_TypeDef *)(0x48000000 + 0x400 * (bank)))
#define RCC ((RCC_TypeDef *)0x40021000)

enum {
	GPIO_MODE_INPUT,
	GPIO_MODE_OUTPUT,
	GPIO_MODE_AF,
	GPIO_MODE_ANALOG
};

static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
	GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
	int n = PINNO(pin);
	gpio->MODER &= ~(3U << (n * 2));
	gpio->MODER |= (mode & 3U) << (n * 2);
}

static inline void gpio_write(uint16_t pin, bool val) {
	GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
	gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
}

static inline void delay(volatile uint32_t counter) {
	while (counter--) {
		__asm__("nop");
	}
}

// Declare external symbols from linker script
extern uint32_t _sbss, _ebss, _sdata, _edata, _sidata;
extern void _estack(void);
extern int main(void);

// Reset handler - this is the entry point
__attribute__((naked, noreturn)) void _reset(void) {
	// Initialize .bss to zero
	for (uint32_t *dst = &_sbss; dst < &_ebss; dst++) {
		*dst = 0;
	}

	// Copy .data from flash to RAM
	for (uint32_t *dst = &_sdata, *src = &_sidata; dst < &_edata;) {
		*dst++ = *src++;
	}

	// Call main
	main();

	// Infinite loop in case main returns
	for (;;) {
	}
}

// Vector table - STM32G4 has 16 system + 91 STM32 specific interrupts = 107
__attribute__((section(".vectors"))) void (*const vector_table[16 + 91])(void) = {
	_estack, // Initial stack pointer
	_reset,	 // Reset handler
};

int main() {
	uint16_t led = PIN('A', 5);

	// Enable GPIOB clock
	RCC->AHB2ENR |= BIT(PINBANK(led));

	// Configure LED pin as output
	gpio_set_mode(led, GPIO_MODE_OUTPUT);

	for (;;) {
		gpio_write(led, true);
		delay(1000000);
		gpio_write(led, false);
		delay(1000000);
	}
}