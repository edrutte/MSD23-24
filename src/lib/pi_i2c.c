#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "pi_i2c.h"

/**
 * Initialize an i2c device at address addr on adapter adapter_num
 * @param adapter_num adapter the device is connected to
 * @param addr address of the i2c slave
 * @return -1 on error, a fd for the i2c device otherwise. The returned fd should be closed before program exit
 */
int init_i2c(int adapter_num, int addr) {
	char filename[20];
	snprintf(filename, 19, "/dev/i2c-%d", adapter_num);
	int i2c_fd = open(filename, O_RDWR);
	if (i2c_fd < 0) {
		perror("open");
		return i2c_fd;
	}
	if (ioctl(i2c_fd, I2C_SLAVE, addr) < 0) {
		perror("I2C_SLAVE ioctl");
		close(i2c_fd);
		return -1;
	}
	return i2c_fd;
}