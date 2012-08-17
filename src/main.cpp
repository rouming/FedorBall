/*
 * FedorBall is a toy-ball which detects current top position
 * with accelerometer MMA7455 and turns on leds in special order
 * using 4 led drivers TLC5940.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
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

/* 2g, pulse mode */
#define MMA7455_2G_PULSE_MODE (_BV(MMA7455_GLVL0) | \
							   _BV(MMA7455_MODE0) | \
							   _BV(MMA7455_MODE1))
/* 2g, measurement mode */
#define MMA7455_2G_MEASUREMENT_MODE (_BV(MMA7455_GLVL0) | \
									 _BV(MMA7455_MODE0))

/* Rate in Hz, value must be big enough to store the result
   of F_CPU/TIMER0_RATE in 8-bit timer register */
#define TIMER0_RATE 100000UL

/* Size must be 2^N */
#define UART_RX_BUFF_SZ (1<<2)
/* Size must be 2^N */
#define UART_TX_BUFF_SZ (1<<8)

#define ABS(m) ((m) > 0 ? (m) : -(m))

/* Extract RGB components */
#define R_RGB(rgb) (((rgb)>>16) & 0xff)
#define G_RGB(rgb) (((rgb)>>8) & 0xff)
#define B_RGB(rgb) ((rgb) & 0xff)

/* Set RGB color for tlc */
#define TLC_SET_RGB(tlc, tlc_vert, rgb)					\
	do {												\
		tlc.set((tlc_vert)[0], R_RGB(rgb) << 4);		\
		tlc.set((tlc_vert)[1], G_RGB(rgb) << 4);		\
		tlc.set((tlc_vert)[2], B_RGB(rgb) << 4);		\
	} while (0)

static unsigned char s_uart_rx_buff[UART_RX_BUFF_SZ];
static unsigned char s_uart_tx_buff[UART_TX_BUFF_SZ];
static volatile uint32_t s_msecs;

/* Forward RX to TX */
static void uart_rx_to_tx(void* rx_cb_data)
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

/* Init external INT1 interrupt to handle level
   detection from accelerometer. */
static void int1_init()
{
	/* enable rising edge on INT1 */
	MCUCR |= (1<<ISC10) | (1<<ISC11);

	/* enable INT1 */
	GICR |= (1<<INT1);
}

/* Level interrupt from MMA7455 which wakes up MCU from sleep */
ISR(INT1_vect)
{
}

/* set up Timer 0 for ball idle counting */
static void timer0_init()
{
	/* Set CTC mode (Clear Timer on Compare Match) (p.76) */
	TCCR0 |= (1 << WGM01);

	/* No prescaler (p.85) */
	TCCR0 |= (1 << CS00);

	/* Set the compare register (OCR0).
	   Do not forget about -1, because we count from 0 */
	OCR0 = F_CPU / TIMER0_RATE - 1;

	/* Enable Output Compare Match Interrupt when TCNT0 == OCR0 */
	TIMSK |= (1 << OCIE0);
}

/* TIMER0 Output Compare Match Interrupt service routine
   works on TIMER0_RATE */
ISR(TIMER0_COMP_vect)
{
	/* Global variable to count the number timer fires  */
	static uint16_t s_timer_fires = 0;
	/* Count msecs */
	if (++s_timer_fires == TIMER0_RATE / 1000UL) {
		s_timer_fires = 0;
		++s_msecs;
	}
}

/* put MMA to double pulse mode, put MCU to power down and wait
   for external interrupt */
static void goto_sleep()
{
	int err;

	/* put mma to 2g, pulse detection, skip calibration */
	err = MMA7455_init(MMA7455_2G_PULSE_MODE, false);
	if (err != 0) {
		LOG("Error: put to pulse mode failed, %u\n", err);
		return;
	}

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	cli();
	sleep_enable();
	sei();
	sleep_cpu();
	sleep_disable();

	/* put mma to 2g, measurement detection, skip calibration */
	err = MMA7455_init(MMA7455_2G_MEASUREMENT_MODE, false);
	if (err != 0) {
		LOG("Error: put to measurement mode failed, %u\n", err);
	}
}

#ifdef FACES_WALKING_TEST
static void do_faces_walking_test()
{
	Tlc5940 tlc;
	tlc.init();

	while (1) {
		for (unsigned int face_i = 0;
			 face_i < ARRAY_SIZE(s_faces_walking_test); ++face_i) {
			uint8_t face = s_faces_walking_test[face_i];
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
											 &t, &u, &v) && t > 0) {
					uint8_t face_idx = i/9*9;
					// dim red
					uint32 rgb = 0x400000;

					tlc.clear();

					// Set color for every tlc led in pentagon face
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 0] * 3],
								rgb);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 1] * 3],
								rgb);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 2] * 3],
								rgb);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 7] * 3],
								rgb);
					TLC_SET_RGB(tlc,
								&s_leds_vert[s_vert_tri_faces[face_idx + 8] * 3],
								rgb);

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

static void ball_loop()
{
	int error;
	uint8_t c;

	// Initialize the MMA7455, and do calibration
	error = MMA7455_init(MMA7455_2G_MEASUREMENT_MODE, 1);
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

	int prev_inited = 0;
	int	prev_x = 0, prev_y = 0, prev_z = 0;
	uint32_t msecs = 0;

	while (1) {
		int x,y,z, error;
		x = y = z = 0;
		error = MMA7455_xyz(&x, &y, &z); // get the accelerometer values.
		if (error != 0) {
			LOG("xyz err: %x\n", error);
			_delay_ms(100);
			continue;
		}

		if (prev_inited &&
			ABS(prev_x - x) < 5 &&
			ABS(prev_y - y) < 5 &&
			ABS(prev_z - z) < 5) {
			/* Goto sleep if ball was not touched for 30 secs */
			if (s_msecs - msecs > 30000) {
				/* Drop all leds before sleep */
				tlc.clear();
				tlc.update();
				/* Sleep now */
				goto_sleep();
				msecs = s_msecs;
				continue;
			}
		}
		else {
			msecs = s_msecs;

			prev_inited = 1;
			prev_x = x;
			prev_y = y;
			prev_z = z;
		}

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
				// dim red
				uint32_t rgb = 0x400000;

				tlc.clear();

				// Set color for every tlc led in pentagon face
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 0] * 3],
							rgb);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 1] * 3],
							rgb);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 2] * 3],
							rgb);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 7] * 3],
							rgb);
				TLC_SET_RGB(tlc,
							&s_leds_vert[s_vert_tri_faces[face_idx + 8] * 3],
							rgb);

				// Commit
				tlc.update();

				break;
			}
		}

		// 50 Hz
		_delay_ms(20);
	}
}

int main()
{
	/* Init UART */
	uart_init(9600, s_uart_rx_buff, UART_RX_BUFF_SZ,
			  s_uart_tx_buff, UART_TX_BUFF_SZ, uart_rx_to_tx, NULL);

	/* Init TWI(I2C) */
	twi_init(TWI_FREQ);

	/* Init timer0 */
	timer0_init();

	/* Init external interrupt INT1 */
	int1_init();

	// enable iterrupts
	sei();

#ifdef FACES_WALKING_TEST
	do_faces_walking_test();
	return 0;
#endif

	ball_loop();

	return 0;
}
