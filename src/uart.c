///////////////////////////////////////////////////////////////////////////
// UART interrupt-driven implementation
///////////////////////////////////////////////////////////////////////////

#include <avr/io.h>
#include <util/atomic.h>

#include <stdio.h>

#include "uart.h"

/*************************************************************************/

ISR(USART_RXC_vect)
{
	/* fill ring buff with byte and do read advance */

	/* call rx cb if enabled */
}

ISR(USART_UDRE_vect)
{
	/* blablablah */

	/* write some byte */
	/*XXX*/
	UDR = 0;

	/*
	  if (nothing_was_written)
		  UCSRB &= ~(1 << UDRIE);
	*/
}

/*************************************************************************/

void uart_init(uint16_t bauds, uart_rx_cb cb, void* user_data)
{
	(void)cb;
	(void)user_data;

	uint16_t ubrr = F_CLK/16/bauds - 1;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		/* Set baud rate */
		UBRRH = (unsigned char)(ubrr>>8);
		UBRRL = (unsigned char)ubrr;
		/* Enable receiver, transmitter and RX interrupt */
		UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
		/* Set frame format: 8data, 2stop bit */
		UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	}
}

void uart_rx_ptr(void** ptr, uint32_t* sz)
{
	*ptr = NULL;
	*sz = 0;
}

void uart_tx_ptr(void** ptr, uint32_t* sz)
{
	*ptr = NULL;
	*sz = 0;
}

void uart_rx_advance(uint32_t sz)
{
}

void uart_tx_advance(uint32_t sz)
{
	(void)sz;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		/*
		  XXX: advance ptr
		 */

		/* Enable UDRE interrupt if TX buff is empty */
		if (UCSRA & (1<<UDRE)) {
			UCSRB |= (1<<UDRIE);
			/* Write some byte */
			/*XXX*/
			UDR = 0;
		}
	}
}

uint16_t uart_printf(const char* fmt, ...)
{
	va_list args;
	char* p;
	uint32_t sz;

	uart_tx_ptr((void**)&p, &sz);

	va_start(args, fmt);
	sz = vsnprintf(p, sz, fmt, args);
	va_end(args);

	uart_tx_advance(sz);

	return sz;
}

/*************************************************************************/
