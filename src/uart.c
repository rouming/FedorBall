/**************************************************************************
 * UART interrupt-driven implementation
 **************************************************************************/

#include <avr/io.h>
#include <util/atomic.h>

#include <stdio.h>

#include "uart.h"
#include "ring_buffer.h"

/*************************************************************************/

static struct ring_buffer s_rx_rbuf;
static struct ring_buffer s_tx_rbuf;

static struct
{
	uart_rx_cb rx_cb;
	void* rx_cb_data;

} s_uart_ctx;

/*************************************************************************/

ISR(USART_RXC_vect)
{
	/* Read byte to avoid next interrupt */
	unsigned char rx_byte = UDR;

	/* No data? Lost byte */
	if (!ring_buffer_free_size(&s_rx_rbuf))
		return;

	void *p1, *p2;
	uint16_t sz1, sz2;

	/* Write one byte to ring buffer and commit it */
	ring_buffer_write_ptr(&s_rx_rbuf, 1, &p1, &sz1, &p2, &sz2);
	*(unsigned char*)p1 = rx_byte;
	ring_buffer_write_advance(&s_rx_rbuf, 1);

	/* RX cb call */
	if (s_uart_ctx.rx_cb)
		s_uart_ctx.rx_cb(s_uart_ctx.rx_cb_data);
}

ISR(USART_UDRE_vect)
{
	/* Ring buffer is empty */
	if (!ring_buffer_used_size(&s_tx_rbuf)) {
		/* Reset TX buffer */
		ring_buffer_reset(&s_tx_rbuf);
		/* Turn off UDRE interrupt */
		UCSRB &= ~(1 << UDRIE);
		return;
	}

	void *p1, *p2;
	uint16_t sz1, sz2;

	/* Get one byte from ring buffer and commit it */
	ring_buffer_read_ptr(&s_tx_rbuf, 1, &p1, &sz1, &p2, &sz2);
	UDR = *(unsigned char*)p1;
	ring_buffer_read_advance(&s_tx_rbuf, 1);
}

/*************************************************************************/

void uart_init(uint16_t bauds,
			   uint8_t* rx_buff, uint16_t rx_sz,
			   uint8_t* tx_buff, uint16_t tx_sz,
			   uart_rx_cb cb, void* user_data)
{
	uint16_t ubrr = F_CLK/16/bauds - 1;

	/* Disable uart rx/tx first */
	UCSRB = 0;

	/* Init UART ctx */
	s_uart_ctx.rx_cb = cb;
	s_uart_ctx.rx_cb_data = user_data;

	/* Init ring buffers */
	ring_buffer_init(&s_rx_rbuf, rx_buff, rx_sz);
	ring_buffer_init(&s_tx_rbuf, tx_buff, tx_sz);

	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Set frame format: 8data, 1stop bit */
	UCSRC = (1<<URSEL)|(3<<UCSZ0);
	/* Enable receiver, transmitter and RX interrupt */
	UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}

void uart_rx_ptr(void** p1, uint16_t* sz1)
{
	void *p2;
	uint16_t sz2;

	uint16_t used = ring_buffer_used_size(&s_rx_rbuf);
	ring_buffer_read_ptr(&s_rx_rbuf, used, p1, sz1, &p2, &sz2);
}

void uart_tx_ptr(void** p1, uint16_t* sz1)
{
	void *p2;
	uint16_t sz2;

	uint16_t used = ring_buffer_free_size(&s_tx_rbuf);
	ring_buffer_write_ptr(&s_tx_rbuf, used, p1, sz1, &p2, &sz2);
}

void uart_rx_advance(uint16_t sz)
{
	ring_buffer_read_advance(&s_rx_rbuf, sz);
}

void uart_tx_advance(uint16_t sz)
{
	if (sz == 0)
		return;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		/* Commit TX data */
		ring_buffer_write_advance(&s_tx_rbuf, sz);

		/* Enable UDRE interrupt if TX is not in progress */
		if (UCSRA & (1<<UDRE)) {
			UCSRB |= (1<<UDRIE);
			void *p1, *p2;
			uint16_t sz1, sz2;

			/* Get one byte from ring buffer */
			ring_buffer_read_ptr(&s_tx_rbuf, 1, &p1, &sz1, &p2, &sz2);
			UDR = *(unsigned char*)p1;
			ring_buffer_read_advance(&s_tx_rbuf, 1);
		}
	}
}

uint16_t uart_printf_pgm_async(const char* fmt, ...)
{
	va_list args;
	char* p;
	uint16_t sz;

	uart_tx_ptr((void**)&p, &sz);

	/* What can we do? */
	if (!sz)
		return 0;

	va_start(args, fmt);
	sz = vsnprintf_P(p, sz, fmt, args);
	va_end(args);

	uart_tx_advance(sz);

	return sz;
}

/*************************************************************************/
