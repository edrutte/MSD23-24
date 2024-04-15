#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "Adafruit_MFRC630.h"
extern "C" {
#include "pi_gpio.h"
#include "pi_rfid.h"
#include "pi_spi.h"
}
static int rfid_fd = -1;
Adafruit_MFRC630 rfid = Adafruit_MFRC630(rfid_fd, 10, 6);

void init_rfid() {
	rfid_fd = open(pi_spi_device, O_RDWR);
	if (rfid_fd < 0) {
		perror("open");
		exit(1);
	}
	pinMode(10, OUTPUT);
	pinMode(6, OUTPUT);
	rfid = Adafruit_MFRC630(rfid_fd, 10, 6);

	if (!(rfid.begin())) {
		printf("RFID error\n");
		exit(1);
	}
	rfid.softReset();
	rfid.configRadio(MFRC630_RADIOCFG_ISO1443A_106);
	return;
}

void debug_block_until_tag_and_dump() {
	init_rfid();
	uint16_t atqa = 0;
	do {
		atqa = rfid.iso14443aRequest();
		/* Looks like we found a tag, move on to selection. */
		if (atqa) {
			uint8_t uid[10] = { 0 };
			uint8_t uidlen;
			uint8_t sak;

			/* Retrieve the UID and SAK values. */
			uidlen = rfid.iso14443aSelect(uid, &sak);
			printf("Found a tag with UUID ");
			for (uint8_t i = 0; i < uidlen; i++) {
				printf("%x ", uid[i]);
			}
			printf("\n");
			if (uidlen > 0) {
				printf("Tag is valid\n");
			}
		}
	} while (atqa == 0);
	close(rfid_fd);
	rfid_fd = -1;
}
