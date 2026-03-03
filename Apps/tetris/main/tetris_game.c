/**
 * @file tetris_game.c
 * @brief Tetris game implementation
 */

#include "tetris_game.h"
#include "lcd_driver.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "tetris";

// Game state
static tetris_state_t game;

// Tetromino shapes (4x4 grid, 4 rotations each)
// 1 = filled block, 0 = empty
static const uint8_t tetromino_shapes[TETROMINO_COUNT][4][4][4] = {
    // I piece (Cyan)
    {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}},
        {{0,0,0,0}, {0,0,0,0}, {1,1,1,1}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}}
    },
    // O piece (Yellow)
    {
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
    },
    // T piece (Purple)
    {
        {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // S piece (Green)
    {
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
        {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // Z piece (Red)
    {
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    },
    // J piece (Blue)
    {
        {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
    },
    // L piece (Orange)
    {
        {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
        {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    }
};

// Tetromino colors
static const uint16_t tetromino_colors[TETROMINO_COUNT] = {
    COLOR_CYAN,    // I
    COLOR_YELLOW,  // O
    COLOR_PURPLE,  // T
    COLOR_GREEN,   // S
    COLOR_RED,     // Z
    COLOR_BLUE,    // J
    COLOR_ORANGE   // L
};

// Button state tracking
static bool button_pressed[6] = {false};
static uint32_t button_last_press[6] = {0};
#define BUTTON_DEBOUNCE_MS 50

// Forward declarations
static void init_buttons(void);
static bool read_button(int gpio_num, int btn_idx);
static void spawn_piece(tetromino_t *piece, tetromino_type_t type);
static bool check_collision(tetromino_t *piece);
static void lock_piece(void);
static void clear_lines(void);
static void rotate_piece(void);
static void move_piece(int dx, int dy);
static void hard_drop(void);
static void draw_board(void);
static void draw_piece(tetromino_t *piece, bool erase);
static void draw_next_piece(void);
static void draw_ui(void);
static int get_drop_interval(void);

esp_err_t tetris_init(void) {
    ESP_LOGI(TAG, "Initializing Tetris game");
    
    // Initialize LCD
    ESP_ERROR_CHECK(lcd_init());
    
    // Initialize buttons
    init_buttons();
    
    // Reset game state
    tetris_reset_game();
    
    ESP_LOGI(TAG, "Game initialized successfully");
    return ESP_OK;
}

static void init_buttons(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL<<BTN_UP) | (1ULL<<BTN_DOWN) | 
                        (1ULL<<BTN_LEFT) | (1ULL<<BTN_RIGHT) |
                        (1ULL<<BTN_A) | (1ULL<<BTN_B),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
}

static bool read_button(int gpio_num, int btn_idx) {
    uint32_t now = esp_timer_get_time() / 1000;  // ms
    if (gpio_get_level(gpio_num) == 0) {  // Active low
        if (!button_pressed[btn_idx]) {
            if (now - button_last_press[btn_idx] > BUTTON_DEBOUNCE_MS) {
                button_pressed[btn_idx] = true;
                button_last_press[btn_idx] = now;
                return true;
            }
        }
    } else {
        button_pressed[btn_idx] = false;
    }
    return false;
}

void tetris_reset_game(void) {
    ESP_LOGI(TAG, "Resetting game");
    
    // Clear board
    memset(game.board, 0, sizeof(game.board));
    
    // Initialize game state
    game.score = 0;
    game.lines_cleared = 0;
    game.level = 1;
    game.game_over = false;
    game.paused = false;
    game.drop_timer = 0;
    game.drop_interval = get_drop_interval();
    game.game_tick = 0;
    
    // Spawn first pieces
    spawn_piece(&game.current_piece, esp_random() % TETROMINO_COUNT);
    spawn_piece(&game.next_piece, esp_random() % TETROMINO_COUNT);
    
    // Initial render
    lcd_fill_screen(COLOR_BLACK);
    draw_board();
    draw_ui();
    draw_next_piece();
}

static void spawn_piece(tetromino_t *piece, tetromino_type_t type) {
    piece->type = type;
    piece->x = BOARD_WIDTH / 2 - 2;  // Center horizontally
    piece->y = 0;
    piece->rotation = 0;
    piece->color = tetromino_colors[type];
}

static bool check_collision(tetromino_t *piece) {
    const uint8_t (*shape)[4] = tetromino_shapes[piece->type][piece->rotation];
    
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int board_x = piece->x + x;
                int board_y = piece->y + y;
                
                // Check boundaries
                if (board_x < 0 || board_x >= BOARD_WIDTH || 
                    board_y < 0 || board_y >= BOARD_HEIGHT) {
                    return true;
                }
                
                // Check board collision
                if (game.board[board_y][board_x] != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

static void lock_piece(void) {
    const uint8_t (*shape)[4] = tetromino_shapes[game.current_piece.type][game.current_piece.rotation];
    
    // Add piece to board
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int board_x = game.current_piece.x + x;
                int board_y = game.current_piece.y + y;
                if (board_y >= 0 && board_y < BOARD_HEIGHT && 
                    board_x >= 0 && board_x < BOARD_WIDTH) {
                    game.board[board_y][board_x] = game.current_piece.type + 1;
                }
            }
        }
    }
    
    // Clear completed lines
    clear_lines();
    
    // Spawn next piece
    game.current_piece = game.next_piece;
    spawn_piece(&game.next_piece, esp_random() % TETROMINO_COUNT);
    
    // Check game over
    if (check_collision(&game.current_piece)) {
        game.game_over = true;
        ESP_LOGI(TAG, "Game Over! Score: %lu", (unsigned long)game.score);
    }
}

static void clear_lines(void) {
    int lines_cleared_now = 0;
    
    // Check each row from bottom to top
    for (int y = BOARD_HEIGHT - 1; y >= 0; y--) {
        bool full = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (game.board[y][x] == 0) {
                full = false;
                break;
            }
        }
        
        if (full) {
            lines_cleared_now++;
            
            // Move all rows above down
            for (int yy = y; yy > 0; yy--) {
                for (int x = 0; x < BOARD_WIDTH; x++) {
                    game.board[yy][x] = game.board[yy-1][x];
                }
            }
            
            // Clear top row
            for (int x = 0; x < BOARD_WIDTH; x++) {
                game.board[0][x] = 0;
            }
            
            y++;  // Check this row again
        }
    }
    
    if (lines_cleared_now > 0) {
        // Scoring: 1 line=100, 2=300, 3=500, 4=800
        static const uint32_t line_scores[] = {0, 100, 300, 500, 800};
        game.score += line_scores[lines_cleared_now] * game.level;
        game.lines_cleared += lines_cleared_now;
        
        // Level up every 10 lines
        game.level = game.lines_cleared / 10 + 1;
        game.drop_interval = get_drop_interval();
        
        ESP_LOGI(TAG, "Cleared %d lines! Score: %lu", lines_cleared_now, (unsigned long)game.score);
    }
}

static void rotate_piece(void) {
    tetromino_t test_piece = game.current_piece;
    test_piece.rotation = (test_piece.rotation + 1) % 4;
    
    if (!check_collision(&test_piece)) {
        game.current_piece.rotation = test_piece.rotation;
    }
}

static void move_piece(int dx, int dy) {
    tetromino_t test_piece = game.current_piece;
    test_piece.x += dx;
    test_piece.y += dy;
    
    if (!check_collision(&test_piece)) {
        game.current_piece.x = test_piece.x;
        game.current_piece.y = test_piece.y;
    } else if (dy > 0) {
        // Hit bottom, lock piece
        lock_piece();
        game.drop_timer = 0;
    }
}

static void hard_drop(void) {
    while (true) {
        tetromino_t test_piece = game.current_piece;
        test_piece.y++;
        
        if (check_collision(&test_piece)) {
            lock_piece();
            game.drop_timer = 0;
            game.score += 2;  // Bonus for hard drop
            break;
        }
        game.current_piece.y++;
    }
}

static int get_drop_interval(void) {
    // Drop faster as level increases (60 ticks = 1 second at 60 FPS)
    int base = 60;
    int reduction = (game.level - 1) * 5;
    int interval = base - reduction;
    return (interval < 10) ? 10 : interval;  // Minimum 10 ticks
}

static void return_to_launcher(void) {
    ESP_LOGI(TAG, "Returning to launcher...");
    
    // Show message
    lcd_fill_screen(COLOR_BLACK);
    lcd_draw_string(20, 140, "Returning to", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(40, 160, "Launcher...", COLOR_WHITE, COLOR_BLACK);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    // Find factory partition (where launcher is)
    const esp_partition_t* factory = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP,
        ESP_PARTITION_SUBTYPE_APP_FACTORY,
        NULL
    );
    
    if (factory != NULL) {
        esp_ota_set_boot_partition(factory);
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Factory partition not found!");
        lcd_draw_string(20, 200, "Error: Can't find", COLOR_RED, COLOR_BLACK);
        lcd_draw_string(20, 220, "launcher partition", COLOR_RED, COLOR_BLACK);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void tetris_handle_input(void) {
    if (game.game_over) {
        // Button B returns to launcher
        if (read_button(BTN_B, 5)) {
            return_to_launcher();
        }
        return;
    }
    
    if (game.paused) {
        // Button A - unpause
        if (read_button(BTN_A, 4)) {
            game.paused = false;
        }
        // Button B - return to launcher
        if (read_button(BTN_B, 5)) {
            return_to_launcher();
        }
        return;
    }
    
    // Rotate
    if (read_button(BTN_UP, 0)) {
        rotate_piece();
    }
    
    // Move left
    if (read_button(BTN_LEFT, 2)) {
        move_piece(-1, 0);
    }
    
    // Move right
    if (read_button(BTN_RIGHT, 3)) {
        move_piece(1, 0);
    }
    
    // Soft drop (move down faster)
    if (read_button(BTN_DOWN, 1)) {
        move_piece(0, 1);
        game.score += 1;  // Small bonus for soft drop
    }
    
    // Hard drop
    if (read_button(BTN_A, 4)) {
        hard_drop();
    }
    
    // Pause game (Button B during gameplay)
    if (read_button(BTN_B, 5)) {
        game.paused = true;
    }
}

void tetris_update(void) {
    if (game.game_over || game.paused) return;
    
    game.game_tick++;
    game.drop_timer++;
    
    // Auto-drop piece
    if (game.drop_timer >= game.drop_interval) {
        move_piece(0, 1);
        game.drop_timer = 0;
    }
}

void tetris_render(void) {
    // Redraw board
    draw_board();
    
    // Draw current piece
    draw_piece(&game.current_piece, false);
    
    // Draw UI
    draw_ui();
    draw_next_piece();
}

static void draw_board(void) {
    // Draw border
    lcd_draw_rect(BOARD_OFFSET_X - 2, BOARD_OFFSET_Y - 2, 
                  BOARD_WIDTH * BLOCK_SIZE + 4, BOARD_HEIGHT * BLOCK_SIZE + 4, 
                  COLOR_WHITE);
    
    // Draw blocks
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int px = BOARD_OFFSET_X + x * BLOCK_SIZE;
            int py = BOARD_OFFSET_Y + y * BLOCK_SIZE;
            
            if (game.board[y][x] == 0) {
                lcd_fill_rect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_BLACK);
            } else {
                uint16_t color = tetromino_colors[game.board[y][x] - 1];
                lcd_fill_rect(px, py, BLOCK_SIZE, BLOCK_SIZE, color);
                lcd_draw_rect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_WHITE);
            }
        }
    }
}

static void draw_piece(tetromino_t *piece, bool erase) {
    const uint8_t (*shape)[4] = tetromino_shapes[piece->type][piece->rotation];
    
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int board_x = piece->x + x;
                int board_y = piece->y + y;
                
                if (board_x >= 0 && board_x < BOARD_WIDTH && 
                    board_y >= 0 && board_y < BOARD_HEIGHT) {
                    int px = BOARD_OFFSET_X + board_x * BLOCK_SIZE;
                    int py = BOARD_OFFSET_Y + board_y * BLOCK_SIZE;
                    
                    if (erase) {
                        lcd_fill_rect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_BLACK);
                    } else {
                        lcd_fill_rect(px, py, BLOCK_SIZE, BLOCK_SIZE, piece->color);
                        lcd_draw_rect(px, py, BLOCK_SIZE, BLOCK_SIZE, COLOR_WHITE);
                    }
                }
            }
        }
    }
}

static void draw_next_piece(void) {
    // Next piece preview area (top right)
    int preview_x = 170;
    int preview_y = 50;
    
    lcd_draw_string(preview_x, preview_y - 15, "NEXT:", COLOR_WHITE, COLOR_BLACK);
    
    // Clear preview area
    lcd_fill_rect(preview_x, preview_y, 4 * BLOCK_SIZE, 4 * BLOCK_SIZE, COLOR_BLACK);
    
    // Draw next piece
    const uint8_t (*shape)[4] = tetromino_shapes[game.next_piece.type][0];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (shape[y][x]) {
                int px = preview_x + x * BLOCK_SIZE;
                int py = preview_y + y * BLOCK_SIZE;
                lcd_fill_rect(px, py, BLOCK_SIZE - 1, BLOCK_SIZE - 1, game.next_piece.color);
            }
        }
    }
}

static void draw_ui(void) {
    // Draw score
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE:%lu", (unsigned long)game.score);
    lcd_draw_string(10, 10, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Draw lines
    snprintf(buf, sizeof(buf), "LINES:%lu", (unsigned long)game.lines_cleared);
    lcd_draw_string(10, 25, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Draw level
    snprintf(buf, sizeof(buf), "LVL:%lu", (unsigned long)game.level);
    lcd_draw_string(170, 10, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Game state messages
    if (game.game_over) {
        lcd_fill_rect(40, SCREEN_HEIGHT/2 - 20, 160, 50, COLOR_BLACK);
        lcd_draw_rect(40, SCREEN_HEIGHT/2 - 20, 160, 50, COLOR_RED);
        lcd_draw_string(60, SCREEN_HEIGHT/2 - 10, "GAME OVER", COLOR_RED, COLOR_BLACK);
        lcd_draw_string(50, SCREEN_HEIGHT/2 + 5, "PRESS B TO", COLOR_WHITE, COLOR_BLACK);
        lcd_draw_string(60, SCREEN_HEIGHT/2 + 20, "RESTART", COLOR_WHITE, COLOR_BLACK);
    } else if (game.paused) {
        lcd_fill_rect(60, SCREEN_HEIGHT/2 - 10, 120, 25, COLOR_BLACK);
        lcd_draw_rect(60, SCREEN_HEIGHT/2 - 10, 120, 25, COLOR_WHITE);
        lcd_draw_string(80, SCREEN_HEIGHT/2, "PAUSED", COLOR_YELLOW, COLOR_BLACK);
    }
}

void tetris_game_loop(void) {
    tetris_handle_input();
    tetris_update();
    tetris_render();
}
