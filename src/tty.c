/**************************************************************************
 * Test application to test USB-UART tty
 **************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#define BYTETOBINARYPATTERN "%d%d%d%d%d%d%d%d"
#define BYTETOBINARY(byte)  \
  (byte & 0x80 ? 1 : 0), \
  (byte & 0x40 ? 1 : 0), \
  (byte & 0x20 ? 1 : 0), \
  (byte & 0x10 ? 1 : 0), \
  (byte & 0x08 ? 1 : 0), \
  (byte & 0x04 ? 1 : 0), \
  (byte & 0x02 ? 1 : 0), \
  (byte & 0x01 ? 1 : 0)

int main()
{
	int fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror("can't open:");
		return -1;
	}

	unsigned int errors = 0;

	struct termios options;
	tcgetattr(fd, &options);

	options.c_iflag &= ~ISTRIP;
	options.c_oflag &= ~ISTRIP;

	options.c_iflag |= IGNPAR;
	//options.c_oflag |= IGNPAR;

	options.c_cflag &= ~(PARENB|PARODD);
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	if (tcsetattr(fd, TCSANOW, &options) < 0) {
		perror("can't set terminal options:");
		return -1;
	}

	unsigned char wr_b = 0;
	unsigned char rd_b = 0;

	for (wr_b = 0; 1; ++wr_b) {
		if (write(fd, &wr_b, 1) <= 0) {
			perror("can't write: ");
			return -1;
		}
		if (read(fd, &rd_b, 1) <= 0) {
			perror("can't read: ");
			return -1;
		}

		if (wr_b != rd_b) {
			printf("write: 0x%02hhx, b" BYTETOBINARYPATTERN "\n",
				   wr_b, BYTETOBINARY(wr_b));
			printf("read : 0x%02hhx, b" BYTETOBINARYPATTERN "\n",
				   rd_b, BYTETOBINARY(rd_b));
			printf("\n");

			++errors;
		}

		if (wr_b == 0xff)
			break;
	}

	printf("\nerrors: %u\n", errors);
	return 0;
}
