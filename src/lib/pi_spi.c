#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include "pi_spi.h"

int send_spi(int fd, uint8_t const *tx, uint8_t *rx, size_t len) {
#ifdef VERBOSE
	printf("SPI send: ");
	for (int i = 0; i < len; i++) {
		printf("0x%02x ", tx[i]);
	}
	printf("\n");
#endif
	struct spi_ioc_transfer spi_tx = {.tx_buf = (unsigned long) tx, .rx_buf = (unsigned long) rx, .len = len, .bits_per_word = 8, .speed_hz = 100000, .delay_usecs = 0, .cs_change=1, .tx_nbits = 0, .rx_nbits = 0, .word_delay_usecs=1};
	if (ioctl(fd, SPI_IOC_MESSAGE(1), &spi_tx) < 1) {
		perror("send_spi");
		return 1;
	}
	return 0;
}
