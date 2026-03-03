/**
 * @file pacman_game.h
 * @brief Pac-Man game for ESP32-S3 e-Badge
 * 
 * Hardware: ILI9341 240x320 LCD, 6-button input
 * Controls: D-pad (Up/Down/Left/Right), A (Pause), B (Restart)
 */

#ifndef PACMAN_GAME_H
#define PACMAN_GAME_H

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
#define GAME_HEIGHT   240  // Square play area

// Game dimensions
#define MAZE_WIDTH    19
#define MAZE_HEIGHT   15
#define TILE_SIZE     12   // 19 * 12 = 228, 15 * 12 = 180
#define GAME_OFFSET_X 6    // Center horizontally
#define GAME_OFFSET_Y 60   // Status bar height

// Colors (RGB565)
#define COLOR_BLACK    0x0000
#define COLOR_WHITE    0xFFFF
#define COLOR_YELLOW   0xFFE0
#define COLOR_BLUE     0x001F
#define COLOR_RED      0xF800
#define COLOR_PINK     0xF81F
#define COLOR_ORANGE   0xFD20
#define COLOR_CYAN     0x07FF
#define COLOR_GREEN    0x07E0
#define COLOR_WALL     0x1084  // Dark blue

// Game tile types
typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL = 1,
    TILE_DOT = 2,
    TILE_POWER = 3
} tile_type_t;

// Direction
typedef enum {
    DIR_NONE = 0,
    DIR_UP = 1,
    DIR_DOWN = 2,
    DIR_LEFT = 3,
    DIR_RIGHT = 4
} direction_t;

// Ghost AI states
typedef enum {
    GHOST_CHASE = 0,
    GHOST_SCATTER = 1,
    GHOST_FRIGHTENED = 2,
    GHOST_EATEN = 3
} ghost_mode_t;

// Entity (Pacman or Ghost)
typedef struct {
    float x;
    float y;
    direction_t dir;
    direction_t next_dir;
    uint16_t color;
    ghost_mode_t mode;
    int target_x;
    int target_y;
    uint32_t move_timer;
    bool active;
} entity_t;

// Game state
typedef struct {
    entity_t pacman;
    entity_t ghosts[4];
    uint8_t maze[MAZE_HEIGHT][MAZE_WIDTH];
    uint32_t score;
    uint8_t lives;
    uint16_t dots_remaining;
    bool game_over;
    bool paused;
    uint32_t power_timer;
    uint32_t game_tick;
    uint32_t level;
} game_state_t;

// Function declarations

/**
 * @brief Initialize the Pac-Man game
 * @return ESP_OK on success
 */
esp_err_t pacman_init(void);

/**
 * @brief Main game loop (call repeatedly)
 */
void pacman_game_loop(void);

/**
 * @brief Reset game to initial state
 */
void pacman_reset_game(void);

/**
 * @brief Handle button input
 */
void pacman_handle_input(void);

/**
 * @brief Update game logic
 */
void pacman_update(void);

/**
 * @brief Render the game
 */
void pacman_render(void);

#endif // PACMAN_GAME_H
