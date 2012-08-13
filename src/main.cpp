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

//#define FACES_WALKING_TEST
#include "coords.h"

#include "std.h"
#include "raytri.h"

/* Size must be 2^N */
#define UART_RX_BUFF_SZ (1<<2)
/* Size must be 2^N */
#define UART_TX_BUFF_SZ (1<<8)

/* Set RGB color for tlc */
#define TLC_SET_RGB(tlc, tlc_vert, r, g, b)				\
	do {												\
		tlc.set((tlc_vert)[0], (r) << 4);				\
		tlc.set((tlc_vert)[1], (g) << 4);				\
		tlc.set((tlc_vert)[2], (b) << 4);				\
	} while (0)

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

#ifdef FACES_WALKING_TEST
static void do_faces_walking_test()
{
	Tlc5940 tlc;
	tlc.init();

	while (1) {
		for (unsigned int face = 0;
			 ARRAY_SIZE(s_face_coords_middle_test); ++face) {
			fp_t x = s_face_coords_middle_test[face * 3 + 0];
			fp_t y = s_face_coords_middle_test[face * 3 + 1];
			fp_t z = s_face_coords_middle_test[face * 3 + 2];

			fp_t orig[] = {FPZERO, FPZERO, FPZERO};
			fp_t dir[] = {x, y, z};

			// Walk through every face triangle
			for (unsigned int i = 0;
				 i < ARRAY_SIZE(s_vert_tri_faces);
				 i += 3) {
				fp_t vert0[] = {
					s_coords_vert[s_vert_tri_faces[i + 0] * 3 + 0],
					s_coords_vert[s_vert_tri_faces[i + 0] * 3 + 1],
					s_coords_vert[s_vert_tri_faces[i + 0] * 3 + 2]};
				fp_t vert1[] = {
					s_coords_vert[s_vert_tri_faces[i + 1] * 3 + 0],
					s_coords_vert[s_vert_tri_faces[i + 1] * 3 + 1],
					s_coords_vert[s_vert_tri_faces[i + 1] * 3 + 2]};
				fp_t vert2[] = {
					s_coords_vert[s_vert_tri_faces[i + 2] * 3 + 0],
					s_coords_vert[s_vert_tri_faces[i + 2] * 3 + 1],
					s_coords_vert[s_vert_tri_faces[i + 2] * 3 + 2]};

				//Count intersection with triangle
				fp_t t, u, v;
				if (fixed_intersect_triangle(orig, dir,
											 vert0, vert1, vert2,
											 &t, &u, &v) && t < 0) {
					uint8_t face_idx = i/9*9;
					tlc.clear();

					// Set color for every tlc led in pentagon face
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 0] * 3],
								0xff, 0x00, 0x00);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 1] * 3],
								0xff, 0x00, 0x00);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 2] * 3],
								0xff, 0x00, 0x00);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 7] * 3],
								0xff, 0x00, 0x00);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 8] * 3],
								0xff, 0x00, 0x00);

					// Commit
					tlc.update();

					LOG("face %u (must be %u), v1=%u, v2=%u, v3=%u\n\n",
						i/9, face,
						s_vert_tri_faces[i + 0],
						s_vert_tri_faces[i + 1],
						s_vert_tri_faces[i + 2]);

					break;
				}
			}

			_delay_ms(5000);
		}
	}
}
#endif /* FACES_WALKING_TEST */

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
		x = y = z = 0;
		error = MMA7455_xyz(&x, &y, &z); // get the accelerometer values.
		if (error != 0)
			LOG("xyz err: %x\n", error);
		else
			LOG("xyz g-force: x=%d y=%d z=%d\n", x, y, z);

		fp_t orig[] = {FPZERO, FPZERO, FPZERO};
		fp_t dir[] = {ITOFP(y), -ITOFP(z), ITOFP(x)};

		// Walk through every face triangle
		for (unsigned int i = 0;
			 error == 0 &&
			 i < ARRAY_SIZE(s_vert_tri_faces);
			 i += 3) {
			fp_t vert0[] = {
				s_coords_vert[s_vert_tri_faces[i + 0] * 3 + 0],
				s_coords_vert[s_vert_tri_faces[i + 0] * 3 + 1],
				s_coords_vert[s_vert_tri_faces[i + 0] * 3 + 2]};
			fp_t vert1[] = {
				s_coords_vert[s_vert_tri_faces[i + 1] * 3 + 0],
				s_coords_vert[s_vert_tri_faces[i + 1] * 3 + 1],
				s_coords_vert[s_vert_tri_faces[i + 1] * 3 + 2]};
			fp_t vert2[] = {
				s_coords_vert[s_vert_tri_faces[i + 2] * 3 + 0],
				s_coords_vert[s_vert_tri_faces[i + 2] * 3 + 1],
				s_coords_vert[s_vert_tri_faces[i + 2] * 3 + 2]};

			//Count intersection with triangle
			fp_t t, u, v;
			if (fixed_intersect_triangle(orig, dir,
										 vert0, vert1, vert2,
										 &t, &u, &v) && t < 0) {
				uint8_t face_idx = i/9*9;
				tlc.clear();

				// Set color for every tlc led in pentagon face
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 0] * 3],
							0xff, 0x00, 0x00);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 1] * 3],
							0xff, 0x00, 0x00);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 2] * 3],
							0xff, 0x00, 0x00);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 7] * 3],
							0xff, 0x00, 0x00);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 8] * 3],
							0xff, 0x00, 0x00);

				// Commit
				tlc.update();

				LOG("found face %u, v1=%u, v2=%u, v3=%u\n\n",
					i/9,
					s_vert_tri_faces[i + 0],
					s_vert_tri_faces[i + 1],
					s_vert_tri_faces[i + 2]);

				break;
			}
		}

		_delay_ms(100);
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

#ifdef FACES_WALKING_TEST
	do_faces_walking_test();
	return 0;
#endif

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
