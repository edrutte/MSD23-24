#ifndef CHESS_LCD_I2C_H
#define CHESS_LCD_I2C_H

#include <stdint.h>

#define LCD_BACKLIGHT 0x8
#define LCD_EN 0x04
#define LCD_ISDATA 1

void lcd_init(int i2c_fd);
void lcd_send(int i2c_fd, uint8_t data, uint8_t isdata);
void lcd_cmd(int i2c_fd, uint8_t cmd);
void lcd_printf(int i2c_fd, const char *str, int len);
void lcd_putc(int i2c_fd, char chr);

#endif //CHESS_LCD_I2C_H
