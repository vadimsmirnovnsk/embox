/**
 * @file
 * @brief Implements serial driver for Raspberry PI
 *
 * @date 02.07.15
 * @author Vita Loginova
 */

#include <hal/reg.h>
#include <stdint.h>
#include <drivers/diag.h>

#define BAUD_RATE OPTION_GET(NUMBER, baud_rate)

#define GPIO_OFFSET  0x20200000

// Controls actuation of pull up/down to ALL GPIO pins.
#define GPPUD        GPIO_OFFSET + 0x94

// Controls actuation of pull up/down for specific GPIO pin.
#define GPPUDCLK0    GPIO_OFFSET + 0x98

#define UART0_OFFSET GPIO_OFFSET + 0x00001000

// The offsets for reach register for the UART.
#define UART0_DR     UART0_OFFSET + 0x00
#define UART0_FR     UART0_OFFSET + 0x18
#define UART0_IBRD   UART0_OFFSET + 0x24
#define UART0_FBRD   UART0_OFFSET + 0x28
#define UART0_LCRH   UART0_OFFSET + 0x2C
#define UART0_CR     UART0_OFFSET + 0x30
#define UART0_IMSC   UART0_OFFSET + 0x38
#define UART0_ICR    UART0_OFFSET + 0x44


static void delay(int32_t count) {
	asm volatile("1: subs %[count], %[count], #1; bne 1b"
			 : : [count]"r"(count));
}


static int raspi_diag_setup(const struct diag *diag) {
	// Disable UART0.
	REG_STORE(UART0_CR, 0x00000000);
	// Setup the GPIO pin 14 && 15.

	// Disable pull up/down for all GPIO pins & delay for 150 cycles.
	REG_STORE(GPPUD, 0x00000000);
	delay(150);

	// Disable pull up/down for pin 14,15 & delay for 150 cycles.
	REG_STORE(GPPUDCLK0, (1 << 14) | (1 << 15));
	delay(150);

	// Write 0 to GPPUDCLK0 to make it take effect.
	REG_STORE(GPPUDCLK0, 0x00000000);

	// Clear pending interrupts.
	REG_STORE(UART0_ICR, 0x7FF);

	// Set integer & fractional part of baud rate.
	// Divider = UART_CLOCK/(16 * Baud)
	// Fraction part register = (Fractional part * 64) + 0.5
	// UART_CLOCK = 3000000; Baud = 115200.

	// Divider = 3000000/(16 * 115200) = 1.627 = ~1.
	// Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
	REG_STORE(UART0_IBRD, 1);
	REG_STORE(UART0_FBRD, 40);

	// Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
	REG_STORE(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

	// Mask all interrupts.
	REG_STORE(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) |
			(1 << 6) | (1 << 7) | (1 << 8) |
			(1 << 9) | (1 << 10));

	// Enable UART0, receive & transfer part of UART.
	REG_STORE(UART0_CR, (1 << 0) | (1 << 8) | (1 << 9));

	return 0;
}

static void raspi_diag_putc(const struct diag *diag, char ch) {
	while (REG_LOAD(UART0_FR) & (1 << 5))
		;
	REG_STORE(UART0_DR, ch);
}

static char raspi_diag_getc(const struct diag *diag) {
	return REG_LOAD(UART0_DR);
}

static int raspi_diag_hasrx(const struct diag *diag) {
	return !(REG_LOAD(UART0_FR) & (1 << 4));
}

DIAG_OPS_DECLARE(
		.init = raspi_diag_setup,
		.putc = raspi_diag_putc,
		.getc = raspi_diag_getc,
		.kbhit = raspi_diag_hasrx,
);
