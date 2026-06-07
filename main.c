// main.c
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BIT(x) (1UL << x)
#define PIN(bank, num) ((((bank) - 'A') << 8) | (num))
#define PINNO(pin) (pin & 255)
#define PINBANK(pin) (pin >> 8)

typedef struct {
	volatile uint32_t MODER;   // 0x00: GPIO port mode register
	volatile uint32_t OTYPER;  // 0x04: GPIO port output type register
	volatile uint32_t OSPEEDR; // 0x08: GPIO port output speed register
	volatile uint32_t PUPDR;   // 0x0C: GPIO port pull-up/pull-down register
	volatile uint32_t IDR;	   // 0x10: GPIO port input data register
	volatile uint32_t ODR;	   // 0x14: GPIO port output data register
	volatile uint32_t BSRR;	   // 0x18: GPIO port bit set/reset register
	volatile uint32_t LCKR;	   // 0x1C: GPIO port configuration lock register
	volatile uint32_t AFRL;	   // 0x20: GPIO alternate function low register
	volatile uint32_t AFRH;	   // 0x24: GPIO alternate function high register
	volatile uint32_t BRR;	   // 0x28: GPIO port bit reset register
} GPIO_TypeDef;

// value of GPIOA register 0x48000000
#define GPIO(bank) ((GPIO_TypeDef *)(0x48000000 + 0x400 * (bank)))

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
	gpio->MODER |= (mode & 3U) << (n * 2); // set new mode
};

typedef struct {
	volatile uint32_t CR;		  // 0x00: Clock control register
	volatile uint32_t ICSCR;	  // 0x04: Internal clock sources calibration register
	volatile uint32_t CFGR;		  // 0x08: Clock configuration register
	volatile uint32_t PLLCFGR;	  // 0x0C: PLL configuration register
	uint32_t RESERVED0[2];		  // 0x10 - 0x17: Reserved
	volatile uint32_t CIER;		  // 0x18: Clock interrupt enable register
	volatile uint32_t CIFR;		  // 0x1C: Clock interrupt flag register
	volatile uint32_t CICR;		  // 0x20: Clock interrupt clear register
	uint32_t RESERVED1[1];		  // 0x24: Reserved
	volatile uint32_t AHB1RSTR;	  // 0x28: AHB1 peripheral reset register
	volatile uint32_t AHB2RSTR;	  // 0x2C: AHB2 peripheral reset register
	volatile uint32_t AHB3RSTR;	  // 0x30: AHB3 peripheral reset register
	uint32_t RESERVED2[1];		  // 0x34: Reserved
	volatile uint32_t APB1RSTR1;  // 0x38: APB1 peripheral reset register 1
	volatile uint32_t APB1RSTR2;  // 0x3C: APB1 peripheral reset register 2
	volatile uint32_t APB2RSTR;	  // 0x40: APB2 peripheral reset register
	uint32_t RESERVED3[1];		  // 0x44: Reserved
	volatile uint32_t AHB1ENR;	  // 0x48: AHB1 peripheral clock enable register
	volatile uint32_t AHB2ENR;	  // 0x4C: AHB2 peripheral clock enable register
	volatile uint32_t AHB3ENR;	  // 0x50: AHB3 peripheral clock enable register
	uint32_t RESERVED4[1];		  // 0x54: Reserved
	volatile uint32_t APB1ENR1;	  // 0x58: APB1 peripheral clock enable register 1
	volatile uint32_t APB1ENR2;	  // 0x5C: APB1 peripheral clock enable register 2
	volatile uint32_t APB2ENR;	  // 0x60: APB2 peripheral clock enable register
	uint32_t RESERVED5[1];		  // 0x64: Reserved
	volatile uint32_t AHB1SMENR;  // 0x68: AHB1 peripheral sleep mode clock enable register
	volatile uint32_t AHB2SMENR;  // 0x6C: AHB2 peripheral sleep mode clock enable register
	volatile uint32_t AHB3SMENR;  // 0x70: AHB3 peripheral sleep mode clock enable register
	uint32_t RESERVED6[1];		  // 0x74: Reserved
	volatile uint32_t APB1SMENR1; // 0x78: APB1 peripheral sleep mode clock enable register 1
	volatile uint32_t APB1SMENR2; // 0x7C: APB1 peripheral sleep mode clock enable register 2
	volatile uint32_t APB2SMENR;  // 0x80: APB2 peripheral sleep mode clock enable register
	uint32_t RESERVED7[1];		  // 0x84: Reserved
	volatile uint32_t CCIPR;	  // 0x88: Peripherals independent clock configuration register 1
	uint32_t RESERVED8[1];		  // 0x8C: Reserved
	volatile uint32_t BDCR;		  // 0x90: Backup domain control register
	volatile uint32_t CSR;		  // 0x94: Control/status register
	volatile uint32_t CRRCR;	  // 0x98: Clock recovery system register
	volatile uint32_t CCIPR2;	  // 0x9C: Peripherals independent clock configuration register 2
} RCC_TypeDef;

// value of first RCC register 0X40021000
#define RCC ((RCC_TypeDef *)(0X40021000))

// Enable GPIO Peripheral
static inline void gpio_write(uint16_t pin, bool val) {
	GPIO_TypeDef *gpio = GPIO(PINBANK(pin));
	gpio->BSRR = (1U << PINNO(pin)) << (val ? 0 : 16);
};

static inline void delay(volatile uint32_t counter) {
	while (counter--)
		asm("nop");
}

// startup code
__attribute__((naked, noreturn)) void _reset(void) {
	extern long _sbss, _ebss, _sdata, _edata, _sidata;
	for (long *dst = &_sbss; dst < &_ebss; dst++)
		*dst = 0;
	for (long *dst = &_sdata, *src = &_sidata; dst < &_edata;)
		*dst++ = *src++;
}

/**
// main.c
__attribute__((naked, noreturn)) void _reset(void) {
	for (;;)
		(void)0; // infinite loop
}

extern void _estack(void); // define link.ld

// 16 standard & 91 stm32-specific handlers
__attribute__((section(".vectors"))) void (*const tab[16 + 91])(void) = {_estack, _reset};
*/

int main() {
	uint16_t led = PIN('B', 7);
	RCC->AHB1ENR |= BIT(PINBANK(led));
	gpio_set_mode(led, GPIO_MODE_OUTPUT);

	for (;;) {
		gpio_write(led, true);
		delay(100000);
		gpio_write(led, false);
	}
}