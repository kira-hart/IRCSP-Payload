#include "../ts7800v2-utils-master/i2c-dev.h"
#include <linux/pci.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include <errno.h>

#define ACCELEROMETER_CHIP_ADDRESS 0x1c

static int accelerometer_init(void);
static int accelerometer_read(int twifd, uint8_t* data, uint16_t addr, int bytes);
static int accelerometer_write(int twifd, uint8_t* data, uint16_t addr, int bytes);

static int accelerometer_init(void)
{
	static int fd = -1;
	fd = open("/dev/i2c-0", O_RDWR);
	if (fd != -1) {
		if (ioctl(fd, I2C_SLAVE_FORCE, ACCELEROMETER_CHIP_ADDRESS) < 0) {
			perror("Accelerometer did not ACK\n");
			return -1;
		}
	}

	return fd;
}



static int accelerometer_read(int twifd, uint8_t* data, uint16_t addr, int bytes) //two wire interface fd, command, 
{
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg msgs[2];

	msgs[0].addr = ACCELEROMETER_CHIP_ADDRESS;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = (void*)&addr;

	msgs[1].addr = ACCELEROMETER_CHIP_ADDRESS;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = bytes;
	msgs[1].buf = (void*)data;

	packets.msgs = msgs;
	packets.nmsgs = 2;

	if (ioctl(twifd, I2C_RDWR, &packets) < 0) {
		perror("Unable to send data");
		return 1;
	}

	return 0;
}

static int accelerometer_write(int twifd, uint8_t* data, uint16_t addr, int bytes)
{
	struct i2c_rdwr_ioctl_data packets;
	struct i2c_msg msg;
	uint8_t outdata[128];

	outdata[0] = addr;
	memcpy(&outdata[1], data, bytes);

	msg.addr = ACCELEROMETER_CHIP_ADDRESS;
	msg.flags = 0;
	msg.len = 1 + bytes;
	msg.buf = (char*)outdata;

	packets.msgs = &msg;
	packets.nmsgs = 1;

	if (ioctl(twifd, I2C_RDWR, &packets) < 0) {
		return 1;
	}
	return 0;
}