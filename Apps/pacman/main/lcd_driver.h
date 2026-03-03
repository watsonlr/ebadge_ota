/**
 * @file lcd_driver.h
 * @brief ILI9341 LCD display driver
 */

#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>
#include "driver/spi_master.h"
#include "esp_err.h"

// LCD dimensions
#define LCD_WIDTH  240
#define LCD_HEIGHT 320

// Initialize LCD
esp_err_t lcd_init(void);

// Basic drawing functions
void lcd_fill_screen(uint16_t color);
void lcd_draw_pixel(int x, int y, uint16_t color);
void lcd_draw_rect(int x, int y, int w, int h, uint16_t color);
void lcd_fill_rect(int x, int y, int w, int h, uint16_t color);
void lcd_draw_circle(int x, int y, int radius, uint16_t color);
void lcd_fill_circle(int x, int y, int radius, uint16_t color);
void lcd_draw_char(int x, int y, char c, uint16_t color, uint16_t bg);
void lcd_draw_string(int x, int y, const char* str, uint16_t color, uint16_t bg);
void lcd_draw_number(int x, int y, uint32_t num, uint16_t color, uint16_t bg);

// Buffer operations
void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void lcd_write_data_buffer(const uint16_t* data, uint32_t len);

#endif // LCD_DRIVER_H
