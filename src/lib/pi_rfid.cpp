#include "Adafruit_MFRC630.h"
extern "C" {
#include "pi_gpio.h"
#include "pi_rfid.h"
}

Adafruit_MFRC630* rfid;

int init_rfid(int rfid_fd) {
	pinMode(10, OUTPUT);
	pinMode(6, OUTPUT);
	rfid = new Adafruit_MFRC630(rfid_fd, 10, 6);
	if (!(rfid->begin())) {
		return 1;
	}
	rfid->softReset();
	rfid->configRadio(MFRC630_RADIOCFG_ISO1443A_106);
	return 0;
}

void debug_block_until_tag_and_dump() {
	//init_rfid();
	uint16_t atqa = 0;
	do {
		atqa = rfid->iso14443aRequest();
		/* Looks like we found a tag, move on to selection. */
		if (atqa) {
			uint8_t uid[10] = { 0 };
			uint8_t uidlen;
			uint8_t sak;

			/* Retrieve the UID and SAK values. */
			uidlen = rfid->iso14443aSelect(uid, &sak);
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
}
