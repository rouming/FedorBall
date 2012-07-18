/*
 * FedorBall is a toy-ball which detects current top position
 * with accelerometer MMA7455 and turns on leds in special order
 * using 4 led drivers TLC5940.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "tlc5940/Tlc5940.h"

int main()
{
	// enable iterrupts
	sei();

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

			_delay_ms(75);
		}
	}

	return 0;
}
