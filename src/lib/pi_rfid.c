#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "../MFRC630/mfrc630.h"
#include "pi_rfid.h"
#include "pi_spi.h"

static int rfid_fd = -1;

void mfrc630_SPI_select() {
	rfid_fd = open(pi_spi_device, O_RDWR);
	if (rfid_fd < 0) {
		perror("open");
		return;
	}
}

void mfrc630_SPI_unselect() {
	close(rfid_fd);
	rfid_fd = -1;
}

void mfrc630_SPI_transfer(const uint8_t* tx, uint8_t* rx, uint16_t len) {
	send_spi(rfid_fd, tx, rx, (size_t) len);
}

void init_rfid() {
	mfrc630_AN1102_recommended_registers(MFRC630_PROTO_ISO14443A_106_MILLER_MANCHESTER);
}

void debug_block_until_tag_and_dump() {
	init_rfid();
	if (mfrc630_read_reg(0x7f) == 0x1a) {
		printf("mfrc read good\n");
	} else {
		printf("cry\n");
		exit(1);
	}
	uint16_t atqa = 0;
	while (!atqa) {
		atqa = mfrc630_iso14443a_REQA();
		if (atqa) {
			uint8_t sak;
			uint8_t uid[10] = {0};
			uint8_t uid_len = mfrc630_iso14443a_select(uid, &sak);
			if (uid_len != 0) {
				printf("UID of %hhu bytes (SAK: %hhu):\n", uid_len, sak);
				for (int i = 0; i < uid_len; i++) {
					printf("%hhu ", uid[i]);
				}
				printf("\n");
			}
		}
	}
}