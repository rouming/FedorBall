// MMA7455 Accelerometer
// ---------------------
//
// By arduino.cc user "Krodal".
// May 2012
// Open Source / Public Domain
//
// Using Arduino 1.0.1
// It will not work with an older version, since Wire.endTransmission()
// uses a parameter to hold or release the I2C bus.
//
// Documentation:
//     - The Freescale MMA7455L datasheet
//     - The AN3468 Application Note (programming).
//     - The AN3728 Application Note (calibrating offset).
//
// The MMA7455 can be used by writing and reading a single byte,
// but it is also capable to read and write multiple bytes.
//
// The accuracy is 10-bits.
//
// History:
//   24-07-12 Roman Pen
//     port to atmega16 and custom TWI implementation
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "atomic.h"
#include "mma7455.h"
#include "twi.h"

// --------------------------------------------------------
// MMA7455_init
//
// Initialize the MMA7455.
// Set also the offset, assuming that the accelerometer is
// in flat horizontal position.
//
// Important notes about the offset:
//    The sensor has internal registers to set an offset.
//    But the offset could also be calculated by software.
//    This function uses the internal offset registers
//    of the sensor.
//    That turned out to be bad idea, since setting the
//    offset alters the actual offset of the sensor.
//    A second offset calculation had to be implemented
//    to fine tune the offset.
//    Using software variables for the offset would be
//    much better.
//
//    The offset is influenced by the slightest vibration
//    (like a computer on the table).
//
int MMA7455_init(void)
{
	int x, y, z, error;
	xyz_union xyz;
	uint8_t c1, c2;

	// Initialize the sensor
	//
	// Sensitivity:
	//    2g : GLVL0
	//    4g : GLVL1
	//    8g : GLVL1 | GLVL0
	// Mode:
	//    Standby         : 0
	//    Measurement     : MODE0
	//    Level Detection : MODE1
	//    Pulse Detection : MODE1 | MODE0
	// There was no need to add functions to write and read
	// a single byte. So only the two functions to write
	// and read multiple bytes are used.

	// Set mode for "2g sensitivity" and "Measurement Mode".
	c1 = _BV(MMA7455_GLVL0) | _BV(MMA7455_MODE0);
	error = MMA7455_write(MMA7455_MCTL, &c1, 1);
	if (error != 0)
		return (error);

	// Read it back, to test the sensor and communication.
	error = MMA7455_read(MMA7455_MCTL, &c2, 1);
	if (error != 0)
		return (error);

	if (c1 != c2)
		return (-99);

	// Clear the offset registers.
	// If the Arduino was reset or with a warm-boot,
	// there still could be offset written in the sensor.
	// Only with power-up the offset values of the sensor
	// are zero.
	xyz.value.x = xyz.value.y = xyz.value.z = 0;
	error = MMA7455_write(MMA7455_XOFFL, (uint8_t *) &xyz, 6);
	if (error != 0)
		return (error);

	// The mode has just been set, and the sensor is activated.
	// To get a valid reading, wait some time.
	_delay_ms(100);

	// Calcuate the offset.
	//
	// The values are 16-bits signed integers, but the sensor
	// uses offsets of 11-bits signed integers.
	// However that is not a problem,
	// as long as the value is within the range.

	// Assuming that the sensor is flat horizontal,
	// the 'z'-axis should be 1 'g'. And 1 'g' is
	// a value of 64 (if the 2g most sensitive setting
	// is used).
	// Note that the actual written value should be doubled
	// for this sensor.

	error = MMA7455_xyz (&x, &y, &z); // get the x,y,z values
	if (error != 0)
		return (error);

	xyz.value.x = 2 * -x;        // The sensor wants double values.
	xyz.value.y = 2 * -y;
	xyz.value.z = 2 * -(z-64);   // 64 is for 1 'g' for z-axis.

	error = MMA7455_write(MMA7455_XOFFL, (uint8_t *) &xyz, 6);
	if (error != 0)
		return (error);

	// The offset has been set, and everything should be okay.
	// But by setting the offset, the offset of the sensor
	// changes.
	// A second offset calculation has to be done after
	// a short delay, to compensate for that.
	_delay_ms(200);

	error = MMA7455_xyz (&x, &y, &z);    // get te x,y,z values again
	if (error != 0)
		return (error);

	xyz.value.x += 2 * -x;       // add to previous value
	xyz.value.y += 2 * -y;
	xyz.value.z += 2 * -(z-64);  // 64 is for 1 'g' for z-axis.

	// Write the offset for a second time.
	// This time the offset is fine tuned.
	error = MMA7455_write(MMA7455_XOFFL, (uint8_t *) &xyz, 6);
	if (error != 0)
		return (error);

	return (0);          // return : no error
}


// --------------------------------------------------------
// MMA7455_xyz
//
// Get the 'g' forces.
// The values are with integers as 64 per 'g'.
//
int MMA7455_xyz( int *pX, int *pY, int *pZ)
{
	xyz_union xyz;
	int error;
	uint8_t c;

	// Wait for status bit DRDY to indicate that
	// all 3 axis are valid.
	do
	{
		error = MMA7455_read (MMA7455_STATUS, &c, 1);
	} while ( !bit_is_set(c, MMA7455_DRDY) && error == 0);
	if (error != 0)
		return (error);

	// Read 6 bytes, containing the X,Y,Z information
	// as 10-bit signed integers.
	error = MMA7455_read (MMA7455_XOUTL, (uint8_t *) &xyz, 6);
	if (error != 0)
		return (error);

	// The output is 10-bits and could be negative.
	// To use the output as a 16-bit signed integer,
	// the sign bit (bit 9) is extended for the 16 bits.
	if (xyz.reg.x_msb & 0x02)    // Bit 9 is sign bit.
		xyz.reg.x_msb |= 0xFC;     // Stretch bit 9 over other bits.
	if (xyz.reg.y_msb & 0x02)
		xyz.reg.y_msb |= 0xFC;
	if (xyz.reg.z_msb & 0x02)
		xyz.reg.z_msb |= 0xFC;

	// The result is the g-force in units of 64 per 'g'.
	*pX = xyz.value.x;
	*pY = xyz.value.y;
	*pZ = xyz.value.z;

	return (0);                  // return : no error
}

typedef struct
{
	twi_result res;
	int done;

}  twi_done;

void on_twi_complete(twi_result res, void* data)
{
	twi_done* done = (twi_done*)data;
	done->res = res;
	MEM_BARRIER;
	done->done = 1;
}

// --------------------------------------------------------
// MMA7455_read
//
// This is a common function to read multiple bytes
// from an I2C device.
//
// Only this function is used to read.
// There is no function for a single byte.
//
int MMA7455_read(uint8_t reg, uint8_t *buffer, int size)
{
	/* Setup iov */
	twi_iov iov[2];
	iov[0].dev_addr = MMA7455_I2C_ADDRESS;
	iov[0].op = twi_write;
	iov[0].ptr = &reg;
	iov[0].len = 1;
	iov[1].dev_addr = MMA7455_I2C_ADDRESS;
	iov[1].op = twi_read;
	iov[1].ptr = buffer;
	iov[1].len = size;

	twi_done done = {0, 0};
	twi_result res = twi_submit_iov(iov, 2, on_twi_complete, (void*)&done);
	if (res != twi_ok)
		return -10;

	// Wait for completion
	while (!done.done)
		MEM_BARRIER;

	return (0);                  // return : no error
}


// --------------------------------------------------------
// MMA7455_write
//
// This is a common function to write multiple bytes
// to an I2C device.
//
// Only this function is used to write.
// There is no function for a single byte.
//
int MMA7455_write(uint8_t reg, const uint8_t *buffer, int size)
{
	/* Setup iov */
	twi_iov iov[2];
	iov[0].dev_addr = MMA7455_I2C_ADDRESS;
	iov[0].op = twi_write;
	iov[0].ptr = &reg;
	iov[0].len = 1;
	iov[1].dev_addr = MMA7455_I2C_ADDRESS;
	iov[1].op = twi_write;
	iov[1].ptr = (uint8_t*)buffer;
	iov[1].len = size;

	twi_done done = {0, 0};
	twi_result res = twi_submit_iov(iov, 2, on_twi_complete, (void*)&done);
	if (res != twi_ok)
		return -10;

	// Wait for completion
	while (!done.done)
		MEM_BARRIER;

	return (0);                  // return : no error
}
