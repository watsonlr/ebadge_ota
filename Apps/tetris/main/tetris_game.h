/**
 * @file tetris_game.h
 * @brief Tetris game for ESP32-S3 e-Badge
 * 
 * Hardware: ILI9341 240x320 LCD, 6-button input
 * Controls: Left/Right (move), Down (soft drop), Up (rotate), A (hard drop), B (restart)
 */

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"

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

// Game dimensions
#define BOARD_WIDTH   10
#define BOARD_HEIGHT  20
#define BLOCK_SIZE    12
#define BOARD_OFFSET_X 60   // Center the 10-block wide board (10*12=120, (240-120)/2=60)
#define BOARD_OFFSET_Y 40   // Leave space for score display

// Colors (RGB565)
#define COLOR_BLACK    0x0000
#define COLOR_WHITE    0xFFFF
#define COLOR_GRAY     0x8410
#define COLOR_RED      0xF800
#define COLOR_GREEN    0x07E0
#define COLOR_BLUE     0x001F
#define COLOR_CYAN     0x07FF
#define COLOR_MAGENTA  0xF81F
#define COLOR_YELLOW   0xFFE0
#define COLOR_ORANGE   0xFD20
#define COLOR_PURPLE   0x780F

// Tetromino types
typedef enum {
    TETROMINO_I = 0,  // Cyan
    TETROMINO_O = 1,  // Yellow
    TETROMINO_T = 2,  // Purple
    TETROMINO_S = 3,  // Green
    TETROMINO_Z = 4,  // Red
    TETROMINO_J = 5,  // Blue
    TETROMINO_L = 6,  // Orange
    TETROMINO_COUNT = 7
} tetromino_type_t;

// Tetromino piece structure
typedef struct {
    tetromino_type_t type;
    int x;
    int y;
    int rotation;  // 0-3
    uint16_t color;
} tetromino_t;

// Game state
typedef struct {
    uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];  // 0=empty, 1-7=color
    tetromino_t current_piece;
    tetromino_t next_piece;
    uint32_t score;
    uint32_t lines_cleared;
    uint32_t level;
    bool game_over;
    bool paused;
    uint32_t drop_timer;
    uint32_t drop_interval;  // Decreases with level
    uint32_t game_tick;
} tetris_state_t;

// Function declarations

/**
 * @brief Initialize the Tetris game
 * @return ESP_OK on success
 */
esp_err_t tetris_init(void);

/**
 * @brief Main game loop (call repeatedly)
 */
void tetris_game_loop(void);

/**
 * @brief Reset game to initial state
 */
void tetris_reset_game(void);

/**
 * @brief Handle button input
 */
void tetris_handle_input(void);

/**
 * @brief Update game logic
 */
void tetris_update(void);

/**
 * @brief Render the game
 */
void tetris_render(void);

#endif // TETRIS_GAME_H
