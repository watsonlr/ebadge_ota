/**
 * @file menu.h
 * @brief Game launcher menu for ESP32-S3 e-Badge
 * 
 * Hardware: ILI9341 240x320 LCD, 6-button input
 * Controls: Up/Down to navigate, A to select, B for options
 */

#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include <stdbool.h>

// Button GPIO Definitions
#define BTN_UP     17
#define BTN_DOWN   16
#define BTN_LEFT   14
#define BTN_RIGHT  15
#define BTN_A      38
#define BTN_B      18

// LCD Pins
#define LCD_MOSI   11
#define LCD_CLK    12
#define LCD_CS     9
#define LCD_DC     13
#define LCD_RST    48

// Display dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// Colors (RGB565)
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_GRAY        0x8410
#define COLOR_LIGHT_GRAY  0xC618
#define COLOR_DARK_GRAY   0x4208
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F
#define COLOR_YELLOW      0xFFE0
#define COLOR_ORANGE      0xFD20
#define COLOR_PURPLE      0x780F

// Menu constants
#define MAX_GAMES 10
#define MENU_ITEM_HEIGHT 60
#define MENU_OFFSET_Y 60

// Game info structure
typedef struct {
    const char *name;
    const char *description;
    const char *ota_url;  // For OTA loading
    uint16_t color;       // Theme color
    bool available;
} game_info_t;

// Menu state
typedef struct {
    game_info_t games[MAX_GAMES];
    int game_count;
    int selected_index;
    int scroll_offset;
    bool in_game_info;
    uint32_t anim_tick;
    bool needs_redraw;
    int last_selected;
    bool full_redraw;  // Force complete redraw
} menu_state_t;

// Function declarations

/**
 * @brief Initialize the game launcher menu
 * @return ESP_OK on success
 */
esp_err_t menu_init(void);

/**
 * @brief Main menu loop (call repeatedly)
 */
void menu_loop(void);

/**
 * @brief Handle button input
 */
void menu_handle_input(void);

/**
 * @brief Update menu state
 */
void menu_update(void);

/**
 * @brief Render the menu
 */
void menu_render(void);

/**
 * @brief Launch selected game
 * @param index Game index to launch
 */
void menu_launch_game(int index);

#endif // MENU_H
