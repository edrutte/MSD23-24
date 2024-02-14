#ifndef CHESS_PI_SPI_H
#define CHESS_PI_SPI_H
#include <linux/spi/spidev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>

static const char *pi_spi_device = "/dev/spidev0.0";

int send_spi(int fd, uint8_t const *tx, uint8_t *rx, size_t len);
#endif //CHESS_PI_SPI_H
