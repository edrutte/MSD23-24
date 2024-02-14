#include "pi_spi.h"

int send_spi(int fd, uint8_t const *tx, uint8_t *rx, size_t len) {
	struct spi_ioc_transfer spi_tx = {.tx_buf = (unsigned long) tx, .rx_buf = (unsigned long) rx, .len = len};
	if (ioctl(fd, SPI_IOC_MESSAGE(1), &spi_tx) < 1) {
		perror("send_spi");
		return 1;
	}
	return 0;
}
