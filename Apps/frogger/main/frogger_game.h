/**
 * @file frogger_game.h
 * @brief Frogger game for ESP32-S3 e-Badge
 * 
 * Hardware: ILI9341 240x320 LCD, 6-button input
 * Controls: D-pad to move frog, A to start, B to restart
 */

#ifndef FROGGER_GAME_H
#define FROGGER_GAME_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"

// Button GPIO Definitions (Rotated 90° for portrait mode)
// Physical: Right->Up, Left->Down, Up->Left, Down->Right
#define BTN_UP     15  // Was RIGHT (GPIO 15)
#define BTN_DOWN   14  // Was LEFT (GPIO 14)
#define BTN_LEFT   17  // Was UP (GPIO 17)
#define BTN_RIGHT  16  // Was DOWN (GPIO 16)
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
#define GRID_SIZE     16   // Each cell is 16x16 pixels
#define GRID_WIDTH    15   // 15 columns (240/16)
#define GRID_HEIGHT   20   // 20 rows (320/16)

// Game area
#define GAME_OFFSET_Y 0
#define UI_HEIGHT     16

// Colors (RGB565)
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_GREEN     0x07E0
#define COLOR_DARK_GREEN 0x0320
#define COLOR_BLUE      0x001F
#define COLOR_LIGHT_BLUE 0x3D9F
#define COLOR_YELLOW    0xFFE0
#define COLOR_RED       0xF800
#define COLOR_BROWN     0x8200
#define COLOR_GRAY      0x8410
#define COLOR_PURPLE    0x780F

// Lane types
typedef enum {
    LANE_SAFE_START = 0,   // Starting safe zone
    LANE_ROAD = 1,         // Road with cars
    LANE_SAFE_MID = 2,     // Middle safe zone
    LANE_RIVER = 3,        // River with logs/turtles
    LANE_SAFE_END = 4      // Goal zone
} lane_type_t;

// Vehicle/obstacle types
typedef enum {
    OBJ_NONE = 0,
    OBJ_CAR_RED = 1,
    OBJ_CAR_BLUE = 2,
    OBJ_TRUCK = 3,
    OBJ_LOG_SHORT = 4,
    OBJ_LOG_MEDIUM = 5,
    OBJ_LOG_LONG = 6,
    OBJ_TURTLE = 7
} object_type_t;

// Direction
typedef enum {
    DIR_LEFT = 0,
    DIR_RIGHT = 1
} direction_t;

// Obstacle/platform structure
typedef struct {
    object_type_t type;
    float x;           // Position (in grid units)
    int y;             // Lane number
    int width;         // Width in grid units
    direction_t dir;   // Movement direction
    float speed;       // Movement speed
    uint16_t color;
} game_object_t;

// Lane configuration
typedef struct {
    lane_type_t type;
    direction_t dir;
    float speed;
    object_type_t obj_type;
    int obj_spacing;   // Spacing between objects
} lane_config_t;

// Frog player
typedef struct {
    int x;             // Grid position
    int y;
    bool alive;
    bool on_platform;  // Standing on log/turtle
    int anim_frame;    // Animation frame
} frog_t;

// Game state
typedef struct {
    frog_t frog;
    game_object_t objects[50];  // All moving objects
    int object_count;
    lane_config_t lanes[GRID_HEIGHT];
    bool goals[5];     // 5 goal spots at top
    uint32_t score;
    uint8_t lives;
    uint8_t level;
    uint32_t time_remaining;
    bool game_over;
    bool level_complete;
    bool paused;
    uint32_t game_tick;
    uint32_t time_tick;
} frogger_state_t;

// Function declarations

/**
 * @brief Initialize the Frogger game
 * @return ESP_OK on success
 */
esp_err_t frogger_init(void);

/**
 * @brief Main game loop (call repeatedly)
 */
void frogger_game_loop(void);

/**
 * @brief Reset game to initial state
 */
void frogger_reset_game(void);

/**
 * @brief Handle button input
 */
void frogger_handle_input(void);

/**
 * @brief Update game logic
 */
void frogger_update(void);

/**
 * @brief Render the game
 */
void frogger_render(void);

#endif // FROGGER_GAME_H
