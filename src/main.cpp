/*
 * FedorBall is a toy-ball which detects current top position
 * with accelerometer MMA7455 and turns on leds in special order
 * using 4 led drivers TLC5940.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "tlc5940/Tlc5940.h"

#include "mma7455.h"
#include "uart.h"
#include "twi.h"
#include "logging.h"

/* Size must be 2^N */
#define UART_RX_BUFF_SZ (1<<2)
/* Size must be 2^N */
#define UART_TX_BUFF_SZ (1<<8)

static unsigned char s_uart_rx_buff[UART_RX_BUFF_SZ];
static unsigned char s_uart_tx_buff[UART_TX_BUFF_SZ];

/* Forward RX to TX */
void uart_rx_to_tx(void* rx_cb_data)
{
	(void)rx_cb_data;

	void *rx_p, *tx_p;
	uint16_t rx_sz, tx_sz, sz;
	uart_rx_ptr(&rx_p, &rx_sz);
	uart_tx_ptr(&tx_p, &tx_sz);
	sz = (rx_sz > tx_sz ? tx_sz : rx_sz);
	if (sz == 0)
		return;
	memcpy(tx_p, rx_p, sz);
	uart_rx_advance(sz);
	uart_tx_advance(sz);
}

void mma7455_test()
{
	int error;
	uint8_t c;

	// Initialize the MMA7455, and set the offset.
	error = MMA7455_init();
	LOG("Freescale MMA7455 accelerometer, inited=%s\n",
		(error == 0 ? "OK" : "ERR"));

	// Read the Status Register
	MMA7455_read(MMA7455_STATUS, &c, 1);

	LOG("STATUS : %x\n", c);

	// Read the "Who am I" value
	MMA7455_read(MMA7455_WHOAMI, &c, 1);

	LOG("WHOAMI: %x\n", c);

	// Read the optional temperature output value (I always read zero)
	MMA7455_read(MMA7455_TOUT, &c, 1);

	LOG("TOUT: %d\n", c);

	Tlc5940 tlc;
	tlc.init();

	while (1) {
		int x,y,z, error;

		// The function MMA7455_xyz returns the 'g'-force
		// as an integer in 64 per 'g'.

		// set x,y,z to zero (they are not written in case of an error).
		x = y = z = 0;
		error = MMA7455_xyz(&x, &y, &z); // get the accelerometer values.

		/*
		double dx,dy,dz;
		dx = (double) x / 64.0;          // calculate the 'g' values.
		dy = (double) y / 64.0;
		dz = (double) z / 64.0;
		*/

		int led = (int)((float)y/64.0f*15.0f + 15.0f/2.0f);

		if (error != 0)
			LOG("xyz err: %x\n", error);
		else
			LOG("xyz g-force: x=%d y=%d z=%d, led=%d\n", x, y, z, led);

		tlc.clear();
		tlc.set(led, 4095);
		tlc.update();

		_delay_ms(1000);
	}
}

int main()
{
	/* Init UART */
	uart_init(9600, s_uart_rx_buff, UART_RX_BUFF_SZ,
			  s_uart_tx_buff, UART_TX_BUFF_SZ, uart_rx_to_tx, NULL);

	/* Init TWI(I2C) */
	twi_init(TWI_FREQ);

	// enable iterrupts
	sei();

	mma7455_test();

	return 0;

	//////

	Tlc5940 tlc;
	tlc.init();

	while (1) {
		int direction = 1;
		for (int channel = 0; channel < NUM_TLCS * 16; channel += direction) {

			/* tlc.clear() sets all the grayscale values to zero,
			   but does not send them to the TLCs.  To actually send the data,
			   call tlc.update() */
			tlc.clear();

			/* tlc.set(channel (0-15), value (0-4095)) sets the grayscale value
			   for one channel (15 is OUT15 on the first TLC, if multiple TLCs
			   are daisy-chained, then channel = 16 would be OUT0 of the second
			   TLC, etc.).

			   value goes from off (0) to always on (4095).

			   Like tlc.clear(), this function only sets up the data,
			   tlc.update() will send the data. */
			if (channel == 0)
				direction = 1;
			else
				tlc.set(channel - 1, 1000);

			tlc.set(channel, 4095);

			if (channel != NUM_TLCS * 16 - 1)
				tlc.set(channel + 1, 1000);
			else
				direction = -1;

			/* tlc.update() sends the data to the TLCs.  This is when the
			   LEDs will actually change. */
			tlc.update();

			_delay_ms(200);

			LOG("!!! UPDATE TLC");
		}
	}

	return 0;
}
