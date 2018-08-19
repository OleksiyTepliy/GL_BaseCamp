/* In contrast to our previous examples, this one is not library.
 * UART lib is worthless if its string output does not rely on a
 * triple-buffered streams with proper synchronization mechanisms.
 * 
 * The main point of this code is to *illustrate* the core concepts.
 *
 * Thus the code below *should not* be reused in any way.
 * Your specific task being solved deserves own specific code.
 */

#define F_CPU 16000000UL
/* bytes in read and write buffers */
#define BUFFER_LEN (64)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
/* Power management. For more see: https://www.nongnu.org/avr-libc/user-manual/group__avr__power.html */
#include <avr/power.h>
/* Atomic operations */
#include <util/atomic.h>
#include <stdbool.h>

static const uint32_t uart_baudrate = 19200;	/* Baud rate (Baud / second) */

/* Value to be placed in UBRR register. See datasheet for more */
static const uint16_t ubrr_val = (F_CPU / (16 * uart_baudrate)) - 1;
/* Read and write buffers */
uint8_t	rdbuff[BUFFER_LEN] = {'\0'},
		wrbuff[BUFFER_LEN] = {'\0'};
static uint8_t rdind = 0, wrind = 0;	/* Indices */
/* Indicates transfer & reception completion */
volatile bool txcflag = true;
volatile bool rxcflag = false;


set_sleep_mode(SLEEP_MODE_IDLE);
sleep_enable();

/* USART RX Complete interrupt handler */
ISR(USART_RX_vect, ISR_BLOCK)
{
	/* Buffer will contain the last N = <buffer_len> chars read */
	rdbuff[rdind] = UDR0;

	if ('\n' == rdbuff[rdind] || '\r' == rdbuff[rdind]) {
		rdbuff[rdind] = '\0';
		rxcflag = true;
		rdind = 0;
	} else {
		rxcflag = false;
		rdind++;
	}

	if (rdind >= BUFFER_LEN)
		rdind = 0;
}


/* USART TX Complete interrupt handler */
ISR(USART_TX_vect, ISR_BLOCK)
{
	/* When data register is empty and after the shift register contents */
	/* are already transfered, this interrupt is raised */
	UCSR0B &= ~(1 << TXCIE0);
}


/* USART Data Register Empty interrupt handler */
ISR(USART_UDRE_vect, ISR_BLOCK)
{
	if (('\0' == wrbuff[wrind]) || txcflag) {
		/* If nothing to transmit - disable this interrupt and exit */
		UCSR0B &= ~(1 << UDRIE0);
		txcflag = true;
		return;
	}

	UDR0 = wrbuff[wrind++];	

	/* Really we don't need this as long as every string ends with '\0' */
	if (wrind >= BUFFER_LEN)
		wrind = 0;
}


static void uart_init(void)
{
	/* To use USART module, we need to power it on first */
	power_usart0_enable();

	/* Configure prescaler */
	UBRR0L = ubrr_val & 0xFF; /* Lower byte */
	UBRR0H = (ubrr_val >> 8) & 0xFF;   /* Higher byte */
	/* Or we could use UBRR0 (16-bit defined) instead */

	/* Set operation mode */
	/* Asynchronous mode, even parity, 2 stop bits, 8 data bits */
	UCSR0C = (1 << UPM01) | (1 << USBS0) | (1 << UCSZ00) | (1 << UCSZ01);

	/* Continue setup & enable USART in one operation */
	/* RX & TX complete, Data reg empty interrupts enabled */
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
}


static void uart_put(char *str)
{
	/* If buffer contents have not been transfered yet -- put MCU to sleep */
	while(!txcflag)
		sleep_cpu();

	/* No interrupts can occur while this block is executed */
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (uint8_t i = 0; i < BUFFER_LEN; i++) {
			wrbuff[i] = str[i];
			if ('\0' == str[i])
				break;
		}
		wrind = 0;
		txcflag = false; /* programmatic transfer complete flag */
		/* Enable transmitter interrupts */
		UCSR0B |= (1 << TXCIE0) | (1 << UDRIE0);
	}
}


static bool atomic_str_eq(char *str1, char *str2)
{
	bool res = true;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		for (uint8_t i = 0; i < BUFFER_LEN; i++) {
			if (str1[i] != str2[i]) {
				res = false;
				break;
			}
			if ('\0' == str1[i])
				break;
		}
	}
	return res;
}
