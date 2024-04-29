/*!
 * @file Adafruit_MFRC630.h
 * Modified for use on Raspberry Pi by Evan Ruttenberg
 */

#ifndef __ADAFRUIT_MFRC630_H__
#define __ADAFRUIT_MFRC630_H__

#include "Adafruit_MFRC630_consts.h"
#include "Adafruit_MFRC630_regs.h"
extern "C" {
#include "pi_spi.h"
#include "pi_gpio.h"
}
#include <stddef.h>

/**
 * Driver for the Adafruit MFRC630 RFID front-end.
 */
class Adafruit_MFRC630 {
public:
  /**
   * HW SPI bus constructor
   *
   * @param transport     The transport to use when communicating with the IC
   * @param cs            The CS/Sel pin for HW SPI access.
   * @param pdown_pin     The power down pin number (required)/
   *
   * @note This instance of the constructor requires the 'transport'
   *       parameter to distinguish is from the default I2C version.
   */
  Adafruit_MFRC630(int fd, uint8_t cs,
                   int8_t pdown_pin = -1);

  /**
   * Initialises the IC and performs some simple system checks.
   *
   * @return True if init succeeded, otherwise false.
   */
  bool begin();

  /* FIFO helpers (see section 7.5) */
  /**
   * Returns the number of bytes current in the FIFO buffer.
   *
   * @return The number of bytes in the FIFO buffer.
   */
  int16_t readFIFOLen();

  /**
   * Reads data from the FIFO buffer.
   *
   * @param len       The number of bytes to read
   * @param buffer    The buffer to write data into.
   *
   * @return The actual number of bytes read from the FIFO buffer.
   */
  int16_t readFIFO(uint16_t len, uint8_t *buffer);

  /**
   * Write sdata into the FIFO buffer.
   *
   * @param len       The number of bytes to write.
   * @param buffer    The data to write into the FIFO buffer.
   *
   * @return The actual number of bytes written.
   */
  int16_t writeFIFO(uint16_t len, uint8_t *buffer);

  /**
   * Clears the contents of the FIFO buffer.
   */
  void clearFIFO();

  /* Command wrappers */
  /**
   * Sends an unparameterized command to the IC.
   *
   * @param command   The command register to send.
   */
  void writeCommand(uint8_t command);

  /**
   * Sends a parametrized command to the IC.
   *
   * @param command   The command register to send.
   * @param paramlen  The number of parameter bytes.
   * @param params    The parameter values to send.
   */
  void writeCommand(uint8_t command, uint8_t paramlen, uint8_t *params);

  /* Radio config. */
  /**
   * Configures the radio for the specified protocol.
   *
   * @param cfg   The radio config setup to use.
   *
   * @return True if succeeded, otherwise false.
   */
  bool configRadio(mfrc630radiocfg cfg);

  /**
   * Performs a soft-reset to put the IC into a known state.
   */
  void softReset();

  /* Generic ISO14443a commands (common to any supported card variety). */
  /**
   * Sends the REQA command, requesting an ISO14443A-106 tag.
   *
   * @return The ATQA value if a card was detected.
   */
  uint16_t iso14443aRequest();

  /**
   * Sends the WUPA wakeup command.
   *
   * @return The ATQA value if a card was detected.
   */
  uint16_t iso14443aWakeup();

  /**
   * Selects a detected ISO14443A card, retrieving the UID and SAK.
   *
   * @param uid   Pointer to the buffer where the uid should be written.
   * @param sak   Pointer to the placeholder for the SAK value.
   *
   * @return True if init succeeded, otherwise false.
   */
  uint8_t iso14443aSelect(uint8_t *uid, const uint8_t *sak);

  /* Mifare commands. */
  /**
   * Loads the specified authentication keys on the IC.
   *
   * @param key   Pointer to the buffer containing the key values.
   */
  void mifareLoadKey(uint8_t *key);

  /**
   * Authenticates the selected card using the previously supplied key/
   *
   * @param key_type  Whether to use KEYA or KEYB for authentication.
   * @param blocknum  The block number to authenticate.
   * @param uid       The UID of the card to authenticate.
   *
   * @return True if init succeeded, otherwise false.
   */
  bool mifareAuth(uint8_t key_type, uint8_t blocknum, uint8_t *uid);

  /**
   * Reads the contents of the specified (and previously authenticated)
   * memory block.
   *
   * @param blocknum  The block number to read.
   * @param buf       The buffer the data should be written into.
   *
   * @return The number of bytes read.
   */
  uint16_t mifareReadBlock(uint8_t blocknum, uint8_t *buf);

  /**
   * Writes the supplied data to the previously authenticated
   * memory block.
   *
   * @param blocknum  The block number to read.
   * @param buf       The buffer holding the data to write.
   *
   * @return The number of bytes written.
   */
  uint16_t mifareWriteBlock(uint16_t blocknum, uint8_t *buf);

  /**
   * The default key for fresh Mifare cards.
   */
  uint8_t mifareKeyGlobal[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  /**
   * The default key for NDEF formatted cards.
   */
  uint8_t mifareKeyNDEF[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};

  /* NTAG commands */
  /**
   * Reads the contents of the specified page.
   *
   * @param pagenum   The page number to read.
   * @param buf       The buffer the data should be written into.
   *
   * @return The number of bytes read.
   */
  uint16_t ntagReadPage(uint16_t pagenum, uint8_t *buf);

  /**
   * Writes the supplied content of the specified page.
   *
   * @param pagenum   The page number to write to.
   * @param buf       The data to write to the card.
   *
   * @return The number of bytes written.
   */
  uint16_t ntagWritePage(uint16_t pagenum, uint8_t *buf);

private:
  int8_t _pdown;
  int _fd;
  uint8_t _cs;

  void write8(uint8_t reg, uint8_t value) const;
  void writeBuffer(uint8_t reg, uint16_t len, uint8_t *buffer) const;
  [[nodiscard]] uint8_t read8(uint8_t reg) const;

  void printHex(uint8_t *buf, size_t len);
  void printError(enum mfrc630errors err);

  uint16_t iso14443aCommand(enum iso14443_cmd cmd);
};

#endif
