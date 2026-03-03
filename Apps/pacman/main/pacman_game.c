/**
 * @file pacman_game.c
 * @brief Pac-Man game implementation
 */

#include "pacman_game.h"
#include "lcd_driver.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static const char *TAG = "pacman";

// Game state
static game_state_t game;

// Classic Pac-Man maze layout (1=wall, 2=dot, 3=power pellet, 0=empty)
static const uint8_t initial_maze[MAZE_HEIGHT][MAZE_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,1},
    {1,3,1,1,2,1,1,1,2,1,2,1,1,1,2,1,1,3,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
    {1,1,1,1,2,1,1,1,0,1,0,1,1,1,2,1,1,1,1},
    {1,0,0,1,2,1,0,0,0,0,0,0,0,1,2,1,0,0,1},
    {1,1,1,1,2,1,0,1,1,0,1,1,0,1,2,1,1,1,1},
    {1,2,2,2,2,2,0,1,0,0,0,1,0,2,2,2,2,2,1},
    {1,2,1,1,2,1,0,1,1,1,1,1,0,1,2,1,1,2,1},
    {1,2,2,2,2,1,2,2,2,1,2,2,2,1,2,2,2,2,1},
    {1,2,1,1,2,1,2,1,1,1,1,1,2,1,2,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

// Button state tracking
static bool button_pressed[6] = {false};
static uint32_t button_last_press[6] = {0};
#define BUTTON_DEBOUNCE_MS 50

// Forward declarations
static void init_buttons(void);
static bool read_button(int gpio_num, int btn_idx);
static void init_entities(void);
static void update_pacman(void);
static void update_ghosts(void);
static void check_collisions(void);
static void draw_maze(void);
static void draw_entity(entity_t *entity, bool is_pacman);
static void draw_ui(void);
static int tile_to_screen_x(int tx);
static int tile_to_screen_y(int ty);
static bool can_move(int tx, int ty);
static direction_t get_opposite_dir(direction_t dir);

esp_err_t pacman_init(void) {
    ESP_LOGI(TAG, "Initializing Pac-Man game");
    
    // Initialize LCD
    ESP_ERROR_CHECK(lcd_init());
    
    // Initialize buttons
    init_buttons();
    
    // Reset game state
    pacman_reset_game();
    
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

void pacman_reset_game(void) {
    ESP_LOGI(TAG, "Resetting game");
    
    // Copy maze
    memcpy(game.maze, initial_maze, sizeof(game.maze));
    
    // Count dots
    game.dots_remaining = 0;
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (game.maze[y][x] == TILE_DOT || game.maze[y][x] == TILE_POWER) {
                game.dots_remaining++;
            }
        }
    }
    
    // Initialize game state
    game.score = 0;
    game.lives = 3;
    game.game_over = false;
    game.paused = false;
    game.power_timer = 0;
    game.game_tick = 0;
    game.level = 1;
    
    // Initialize entities
    init_entities();
    
    // Initial render
    lcd_fill_screen(COLOR_BLACK);
    draw_maze();
    draw_ui();
}

static void init_entities(void) {
    // Initialize Pac-Man (center-ish of maze)
    game.pacman.x = 9.0f;
    game.pacman.y = 11.0f;
    game.pacman.dir = DIR_NONE;
    game.pacman.next_dir = DIR_NONE;
    game.pacman.color = COLOR_YELLOW;
    game.pacman.move_timer = 0;
    game.pacman.active = true;
    
    // Initialize ghosts
    uint16_t ghost_colors[] = {COLOR_RED, COLOR_PINK, COLOR_CYAN, COLOR_ORANGE};
    int ghost_positions[4][2] = {{7, 7}, {11, 7}, {7, 9}, {11, 9}};
    
    for (int i = 0; i < 4; i++) {
        game.ghosts[i].x = ghost_positions[i][0];
        game.ghosts[i].y = ghost_positions[i][1];
        game.ghosts[i].dir = DIR_RIGHT;
        game.ghosts[i].next_dir = DIR_RIGHT;
        game.ghosts[i].color = ghost_colors[i];
        game.ghosts[i].mode = GHOST_CHASE;
        game.ghosts[i].move_timer = 0;
        game.ghosts[i].active = true;
    }
}

void pacman_handle_input(void) {
    if (game.game_over || game.paused) {
        // Only check restart button
        if (read_button(BTN_B, 5)) {
            ESP_LOGI(TAG, "Restart button pressed");
            pacman_reset_game();
        }
        return;
    }
    
    // Check pause
    if (read_button(BTN_A, 4)) {
        game.paused = !game.paused;
        ESP_LOGI(TAG, "Pause toggled: %d", game.paused);
        return;
    }
    
    // D-pad controls - set next direction
    if (read_button(BTN_UP, 0)) {
        game.pacman.next_dir = DIR_UP;
    }
    else if (read_button(BTN_DOWN, 1)) {
        game.pacman.next_dir = DIR_DOWN;
    }
    else if (read_button(BTN_LEFT, 2)) {
        game.pacman.next_dir = DIR_LEFT;
    }
    else if (read_button(BTN_RIGHT, 3)) {
        game.pacman.next_dir = DIR_RIGHT;
    }
}

void pacman_update(void) {
    if (game.game_over || game.paused) return;
    
    game.game_tick++;
    
    // Update power pellet timer
    if (game.power_timer > 0) {
        game.power_timer--;
        if (game.power_timer == 0) {
            // Power mode ended, restore ghost colors
            uint16_t ghost_colors[] = {COLOR_RED, COLOR_PINK, COLOR_CYAN, COLOR_ORANGE};
            for (int i = 0; i < 4; i++) {
                if (game.ghosts[i].mode == GHOST_FRIGHTENED) {
                    game.ghosts[i].mode = GHOST_CHASE;
                    game.ghosts[i].color = ghost_colors[i];
                }
            }
        }
    }
    
    update_pacman();
    update_ghosts();
    check_collisions();
    
    // Check win condition
    if (game.dots_remaining == 0) {
        ESP_LOGI(TAG, "Level complete!");
        game.level++;
        vTaskDelay(pdMS_TO_TICKS(2000));
        pacman_reset_game();
    }
}

static void update_pacman(void) {
    game.pacman.move_timer++;
    
    // Move every 8 game ticks (~133ms)
    if (game.pacman.move_timer < 8) return;
    game.pacman.move_timer = 0;
    
    // Try to turn in next direction
    int next_x = (int)roundf(game.pacman.x);
    int next_y = (int)roundf(game.pacman.y);
    
    switch (game.pacman.next_dir) {
        case DIR_UP: next_y--; break;
        case DIR_DOWN: next_y++; break;
        case DIR_LEFT: next_x--; break;
        case DIR_RIGHT: next_x++; break;
        default: break;
    }
    
    if (game.pacman.next_dir != DIR_NONE && can_move(next_x, next_y)) {
        game.pacman.dir = game.pacman.next_dir;
    }
    
    // Move in current direction
    next_x = (int)roundf(game.pacman.x);
    next_y = (int)roundf(game.pacman.y);
    
    switch (game.pacman.dir) {
        case DIR_UP: next_y--; break;
        case DIR_DOWN: next_y++; break;
        case DIR_LEFT: next_x--; break;
        case DIR_RIGHT: next_x++; break;
        default: return;
    }
    
    if (can_move(next_x, next_y)) {
        game.pacman.x = next_x;
        game.pacman.y = next_y;
        
        // Eat dot/power pellet
        uint8_t tile = game.maze[next_y][next_x];
        if (tile == TILE_DOT) {
            game.maze[next_y][next_x] = TILE_EMPTY;
            game.score += 10;
            game.dots_remaining--;
        } else if (tile == TILE_POWER) {
            game.maze[next_y][next_x] = TILE_EMPTY;
            game.score += 50;
            game.dots_remaining--;
            game.power_timer = 600;  // ~10 seconds
            
            // Frighten ghosts
            for (int i = 0; i < 4; i++) {
                game.ghosts[i].mode = GHOST_FRIGHTENED;
                game.ghosts[i].color = COLOR_BLUE;
                game.ghosts[i].dir = get_opposite_dir(game.ghosts[i].dir);
            }
        }
    }
}

static void update_ghosts(void) {
    for (int i = 0; i < 4; i++) {
        if (!game.ghosts[i].active) continue;
        
        game.ghosts[i].move_timer++;
        
        // Ghosts move slower (every 10 ticks)
        int move_delay = (game.ghosts[i].mode == GHOST_FRIGHTENED) ? 12 : 10;
        if (game.ghosts[i].move_timer < move_delay) continue;
        game.ghosts[i].move_timer = 0;
        
        // Simple AI: try to move toward Pac-Man
        int gx = (int)roundf(game.ghosts[i].x);
        int gy = (int)roundf(game.ghosts[i].y);
        int px = (int)roundf(game.pacman.x);
        int py = (int)roundf(game.pacman.y);
        
        direction_t possible_dirs[4];
        int possible_count = 0;
        
        // If frightened, move away from Pac-Man
        bool flee = (game.ghosts[i].mode == GHOST_FRIGHTENED);
        
        // Check all directions
        if (can_move(gx, gy - 1) && game.ghosts[i].dir != DIR_DOWN) {
            possible_dirs[possible_count++] = DIR_UP;
        }
        if (can_move(gx, gy + 1) && game.ghosts[i].dir != DIR_UP) {
            possible_dirs[possible_count++] = DIR_DOWN;
        }
        if (can_move(gx - 1, gy) && game.ghosts[i].dir != DIR_RIGHT) {
            possible_dirs[possible_count++] = DIR_LEFT;
        }
        if (can_move(gx + 1, gy) && game.ghosts[i].dir != DIR_LEFT) {
            possible_dirs[possible_count++] = DIR_RIGHT;
        }
        
        // Choose best direction
        if (possible_count > 0) {
            direction_t best_dir = possible_dirs[0];
            float best_dist = 999999.0f;
            
            for (int d = 0; d < possible_count; d++) {
                int test_x = gx, test_y = gy;
                switch (possible_dirs[d]) {
                    case DIR_UP: test_y--; break;
                    case DIR_DOWN: test_y++; break;
                    case DIR_LEFT: test_x--; break;
                    case DIR_RIGHT: test_x++; break;
                    default: break;
                }
                
                float dist = sqrtf((test_x - px) * (test_x - px) + (test_y - py) * (test_y - py));
                
                if (flee) {
                    // Flee: maximize distance
                    if (dist > best_dist) {
                        best_dist = dist;
                        best_dir = possible_dirs[d];
                    }
                } else {
                    // Chase: minimize distance
                    if (dist < best_dist) {
                        best_dist = dist;
                        best_dir = possible_dirs[d];
                    }
                }
            }
            
            game.ghosts[i].dir = best_dir;
        }
        
        // Move ghost
        switch (game.ghosts[i].dir) {
            case DIR_UP: gy--; break;
            case DIR_DOWN: gy++; break;
            case DIR_LEFT: gx--; break;
            case DIR_RIGHT: gx++; break;
            default: break;
        }
        
        if (can_move(gx, gy)) {
            game.ghosts[i].x = gx;
            game.ghosts[i].y = gy;
        }
    }
}

static void check_collisions(void) {
    int px = (int)roundf(game.pacman.x);
    int py = (int)roundf(game.pacman.y);
    
    for (int i = 0; i < 4; i++) {
        if (!game.ghosts[i].active) continue;
        
        int gx = (int)roundf(game.ghosts[i].x);
        int gy = (int)roundf(game.ghosts[i].y);
        
        if (px == gx && py == gy) {
            if (game.ghosts[i].mode == GHOST_FRIGHTENED) {
                // Eat ghost
                game.score += 200;
                game.ghosts[i].active = false;
                ESP_LOGI(TAG, "Ghost eaten!");
            } else {
                // Lose life
                game.lives--;
                ESP_LOGI(TAG, "Hit by ghost! Lives remaining: %d", game.lives);
                
                if (game.lives <= 0) {
                    game.game_over = true;
                    ESP_LOGI(TAG, "Game Over! Final score: %lu", (unsigned long)game.score);
                } else {
                    // Reset positions
                    init_entities();
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
                return;
            }
        }
    }
}

void pacman_render(void) {
    // Only redraw changed parts for better performance
    draw_maze();
    
    // Draw Pac-Man
    draw_entity(&game.pacman, true);
    
    // Draw ghosts
    for (int i = 0; i < 4; i++) {
        if (game.ghosts[i].active) {
            draw_entity(&game.ghosts[i], false);
        }
    }
    
    draw_ui();
}

static void draw_maze(void) {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            int sx = tile_to_screen_x(x);
            int sy = tile_to_screen_y(y);
            
            uint8_t tile = game.maze[y][x];
            
            switch (tile) {
                case TILE_WALL:
                    lcd_fill_rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOR_WALL);
                    break;
                case TILE_DOT:
                    lcd_fill_rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOR_BLACK);
                    lcd_fill_circle(sx + TILE_SIZE/2, sy + TILE_SIZE/2, 2, COLOR_WHITE);
                    break;
                case TILE_POWER:
                    lcd_fill_rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOR_BLACK);
                    lcd_fill_circle(sx + TILE_SIZE/2, sy + TILE_SIZE/2, 4, COLOR_WHITE);
                    break;
                case TILE_EMPTY:
                    lcd_fill_rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOR_BLACK);
                    break;
            }
        }
    }
}

static void draw_entity(entity_t *entity, bool is_pacman) {
    int sx = tile_to_screen_x((int)roundf(entity->x));
    int sy = tile_to_screen_y((int)roundf(entity->y));
    
    // Clear old position (draw maze tile)
    lcd_fill_rect(sx, sy, TILE_SIZE, TILE_SIZE, COLOR_BLACK);
    
    // Draw entity
    int radius = TILE_SIZE / 2 - 1;
    lcd_fill_circle(sx + radius + 1, sy + radius + 1, radius, entity->color);
    
    if (is_pacman) {
        // Draw mouth
        int mx = sx + radius + 1;
        int my = sy + radius + 1;
        switch (entity->dir) {
            case DIR_RIGHT:
                lcd_draw_pixel(mx + radius - 1, my, COLOR_BLACK);
                lcd_draw_pixel(mx + radius - 2, my - 1, COLOR_BLACK);
                lcd_draw_pixel(mx + radius - 2, my + 1, COLOR_BLACK);
                break;
            case DIR_LEFT:
                lcd_draw_pixel(mx - radius + 1, my, COLOR_BLACK);
                lcd_draw_pixel(mx - radius + 2, my - 1, COLOR_BLACK);
                lcd_draw_pixel(mx - radius + 2, my + 1, COLOR_BLACK);
                break;
            case DIR_UP:
                lcd_draw_pixel(mx, my - radius + 1, COLOR_BLACK);
                lcd_draw_pixel(mx - 1, my - radius + 2, COLOR_BLACK);
                lcd_draw_pixel(mx + 1, my - radius + 2, COLOR_BLACK);
                break;
            case DIR_DOWN:
                lcd_draw_pixel(mx, my + radius - 1, COLOR_BLACK);
                lcd_draw_pixel(mx - 1, my + radius - 2, COLOR_BLACK);
                lcd_draw_pixel(mx + 1, my + radius - 2, COLOR_BLACK);
                break;
            default:
                break;
        }
    }
}

static void draw_ui(void) {
    // Draw score
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE:%lu", (unsigned long)game.score);
    lcd_draw_string(10, 10, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Draw lives
    snprintf(buf, sizeof(buf), "LIVES:%d", game.lives);
    lcd_draw_string(10, 25, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Draw level
    snprintf(buf, sizeof(buf), "LVL:%lu", (unsigned long)game.level);
    lcd_draw_string(160, 10, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Draw status
    if (game.game_over) {
        lcd_draw_string(60, SCREEN_HEIGHT/2, "GAME OVER", COLOR_RED, COLOR_BLACK);
        lcd_draw_string(40, SCREEN_HEIGHT/2 + 15, "PRESS B RESTART", COLOR_WHITE, COLOR_BLACK);
    } else if (game.paused) {
        lcd_draw_string(80, SCREEN_HEIGHT/2, "PAUSED", COLOR_YELLOW, COLOR_BLACK);
    }
}

static int tile_to_screen_x(int tx) {
    return GAME_OFFSET_X + tx * TILE_SIZE;
}

static int tile_to_screen_y(int ty) {
    return GAME_OFFSET_Y + ty * TILE_SIZE;
}

static bool can_move(int tx, int ty) {
    if (tx < 0 || tx >= MAZE_WIDTH || ty < 0 || ty >= MAZE_HEIGHT) {
        return false;
    }
    return game.maze[ty][tx] != TILE_WALL;
}

static direction_t get_opposite_dir(direction_t dir) {
    switch (dir) {
        case DIR_UP: return DIR_DOWN;
        case DIR_DOWN: return DIR_UP;
        case DIR_LEFT: return DIR_RIGHT;
        case DIR_RIGHT: return DIR_LEFT;
        default: return DIR_NONE;
    }
}

void pacman_game_loop(void) {
    pacman_handle_input();
    pacman_update();
    pacman_render();
}
