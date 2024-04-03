// Adapted from LiquidCrystal_I2C Arduino Library
#include <unistd.h>
#include "lcd_i2c.h"
#include "pi_gpio.h"

void lcd_init(int i2c_fd) {
	uint8_t bl = LCD_BACKLIGHT;
	write(i2c_fd, &bl, 1);
	delay(1000);
	uint8_t write_byte = 0x30;
	for (int i = 0; i < 4; i++) {// Why...
		write(i2c_fd, &write_byte, 1);
		write_byte |= LCD_EN;
		write(i2c_fd, &write_byte, 1);
		delay(1);
		write_byte &= ~LCD_EN;
		write(i2c_fd, &write_byte, 1);
		delay(5);
		if (i > 2) {
			write_byte = 0x20;// Arduino does it...
		}
	}
	lcd_cmd(i2c_fd, 0x28);// Function set >1 line
	lcd_cmd(i2c_fd, 0x0c);// Display set on
	lcd_cmd(i2c_fd, 0x06);// Entry set rtl
}

void lcd_send(int i2c_fd, uint8_t data, uint8_t isdata) {
	uint8_t write_bytes[2] = {(data & 0xf0) | isdata | LCD_BACKLIGHT, ((data << 4) & 0xf0) | isdata | LCD_BACKLIGHT};
	for (int i = 0; i < 2; i++) {// Yes Arduino writes each byte thrice; No I don't know why
		uint8_t write_byte = write_bytes[i];
		write(i2c_fd, &write_byte, 1);
		write_byte |= LCD_EN;
		write(i2c_fd, &write_byte, 1);
		delay(1);
		write_byte &= ~LCD_EN;
		write(i2c_fd, &write_byte, 1);
		delay(1);
	}
}

void lcd_cmd(int i2c_fd, uint8_t cmd) {
	lcd_send(i2c_fd, cmd, 0);
}

void lcd_putc(int i2c_fd, char chr) {
	lcd_send(i2c_fd, chr, 1);
}