/**
 * @file frogger_game.c
 * @brief Frogger game implementation
 */

#include "frogger_game.h"
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
#include <math.h>

static const char *TAG = "frogger";

// Game state
static frogger_state_t game;

// Button state tracking
static bool button_pressed[6] = {false};
static uint32_t button_last_press[6] = {0};
#define BUTTON_DEBOUNCE_MS 150

// Level configurations (lanes from bottom to top)
static const lane_config_t level_lanes[GRID_HEIGHT] = {
    {LANE_SAFE_START, DIR_RIGHT, 0.0f, OBJ_NONE, 0},          // Row 0: Start
    {LANE_SAFE_START, DIR_RIGHT, 0.0f, OBJ_NONE, 0},          // Row 1: Start
    {LANE_ROAD, DIR_RIGHT, 0.8f, OBJ_CAR_RED, 5},              // Row 2
    {LANE_ROAD, DIR_LEFT, 1.2f, OBJ_CAR_BLUE, 6},             // Row 3
    {LANE_ROAD, DIR_RIGHT, 1.5f, OBJ_CAR_RED, 4},             // Row 4
    {LANE_ROAD, DIR_LEFT, 0.6f, OBJ_TRUCK, 8},                // Row 5
    {LANE_ROAD, DIR_RIGHT, 1.0f, OBJ_CAR_BLUE, 5},            // Row 6
    {LANE_SAFE_MID, DIR_RIGHT, 0.0f, OBJ_NONE, 0},            // Row 7: Median
    {LANE_RIVER, DIR_LEFT, 0.8f, OBJ_LOG_SHORT, 6},           // Row 8
    {LANE_RIVER, DIR_RIGHT, 1.0f, OBJ_LOG_MEDIUM, 7},         // Row 9
    {LANE_RIVER, DIR_LEFT, 1.2f, OBJ_TURTLE, 5},              // Row 10
    {LANE_RIVER, DIR_RIGHT, 0.7f, OBJ_LOG_LONG, 9},           // Row 11
    {LANE_RIVER, DIR_LEFT, 1.3f, OBJ_LOG_SHORT, 4},           // Row 12
    {LANE_RIVER, DIR_RIGHT, 0.9f, OBJ_LOG_MEDIUM, 6},         // Row 13
    {LANE_SAFE_END, DIR_RIGHT, 0.0f, OBJ_NONE, 0},            // Row 14: Goals
};

// Forward declarations
static void init_buttons(void);
static bool read_button(int gpio_num, int btn_idx);
static void init_level(void);
static void spawn_objects(void);
static void update_objects(void);
static void check_collisions(void);
static void move_frog(int dx, int dy);
static void kill_frog(void);
static void draw_lane(int y);
static void draw_object(game_object_t *obj);
static void draw_frog(void);
static void draw_ui(void);
static int grid_to_screen_x(int gx);
static int grid_to_screen_y(int gy);

esp_err_t frogger_init(void) {
    ESP_LOGI(TAG, "Initializing Frogger game");
    
    // Initialize LCD
    ESP_ERROR_CHECK(lcd_init());
    
    // Initialize buttons
    init_buttons();
    
    // Reset game state
    frogger_reset_game();
    
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
    int level = gpio_get_level(gpio_num);
    
    if (level == 0) {  // Active low
        if (!button_pressed[btn_idx]) {
            if (now - button_last_press[btn_idx] > BUTTON_DEBOUNCE_MS) {
                button_pressed[btn_idx] = true;
                button_last_press[btn_idx] = now;
                ESP_LOGI(TAG, "Button on GPIO %d pressed (idx=%d)", gpio_num, btn_idx);
                return true;
            }
        }
    } else {
        button_pressed[btn_idx] = false;
    }
    return false;
}

void frogger_reset_game(void) {
    ESP_LOGI(TAG, "Resetting game");
    
    // Initialize game state
    game.score = 0;
    game.lives = 3;
    game.level = 1;
    game.game_over = false;
    game.level_complete = false;
    game.paused = false;
    game.game_tick = 0;
    game.time_tick = 0;
    game.time_remaining = 60;  // 60 seconds per level
    
    // Clear goals
    memset(game.goals, 0, sizeof(game.goals));
    
    // Initialize frog
    game.frog.x = GRID_WIDTH / 2;
    game.frog.y = 1;
    game.frog.alive = true;
    game.frog.on_platform = false;
    game.frog.anim_frame = 0;
    
    ESP_LOGI(TAG, "Frog initialized at position (%d, %d)", game.frog.x, game.frog.y);
    
    // Initialize level
    init_level();
    
    // Clear screen first
    lcd_fill_screen(COLOR_BLACK);
    
    // Draw static background once
    for (int y = 0; y < GRID_HEIGHT; y++) {
        draw_lane(y);
    }
}

static void init_level(void) {
    ESP_LOGI(TAG, "Initializing level %d", game.level);
    
    // Copy lane configuration
    memcpy(game.lanes, level_lanes, sizeof(level_lanes));
    
    // Adjust speeds based on level
    for (int i = 0; i < GRID_HEIGHT; i++) {
        if (game.lanes[i].type == LANE_ROAD || game.lanes[i].type == LANE_RIVER) {
            game.lanes[i].speed *= (1.0f + (game.level - 1) * 0.2f);
        }
    }
    
    // Spawn objects
    spawn_objects();
}

static void spawn_objects(void) {
    game.object_count = 0;
    
    for (int y = 0; y < GRID_HEIGHT; y++) {
        lane_config_t *lane = &game.lanes[y];
        
        if (lane->type == LANE_ROAD || lane->type == LANE_RIVER) {
            // Spawn 2-3 objects per lane
            int num_objects = 2 + (esp_random() % 2);
            
            for (int i = 0; i < num_objects && game.object_count < 50; i++) {
                game_object_t *obj = &game.objects[game.object_count++];
                
                obj->type = lane->obj_type;
                obj->y = y;
                obj->dir = lane->dir;
                obj->speed = lane->speed;
                
                // Set width based on type
                switch (obj->type) {
                    case OBJ_CAR_RED:
                    case OBJ_CAR_BLUE:
                        obj->width = 2;
                        obj->color = (obj->type == OBJ_CAR_RED) ? COLOR_RED : COLOR_BLUE;
                        break;
                    case OBJ_TRUCK:
                        obj->width = 3;
                        obj->color = COLOR_BROWN;
                        break;
                    case OBJ_LOG_SHORT:
                        obj->width = 2;
                        obj->color = COLOR_BROWN;
                        break;
                    case OBJ_LOG_MEDIUM:
                        obj->width = 3;
                        obj->color = COLOR_BROWN;
                        break;
                    case OBJ_LOG_LONG:
                        obj->width = 4;
                        obj->color = COLOR_BROWN;
                        break;
                    case OBJ_TURTLE:
                        obj->width = 2;
                        obj->color = COLOR_GREEN;
                        break;
                    default:
                        obj->width = 2;
                        obj->color = COLOR_GRAY;
                }
                
                // Position with spacing
                obj->x = i * lane->obj_spacing;
                if (lane->dir == DIR_LEFT) {
                    obj->x += GRID_WIDTH / 2;
                }
            }
        }
    }
}

static void update_objects(void) {
    for (int i = 0; i < game.object_count; i++) {
        game_object_t *obj = &game.objects[i];
        
        // Move object
        if (obj->dir == DIR_LEFT) {
            obj->x -= obj->speed * 0.016f;  // Pixels per frame at 60fps
            if (obj->x + obj->width < 0) {
                obj->x = GRID_WIDTH;
            }
        } else {
            obj->x += obj->speed * 0.016f;
            if (obj->x > GRID_WIDTH) {
                obj->x = -obj->width;
            }
        }
    }
}

static void check_collisions(void) {
    if (!game.frog.alive) return;
    
    int frog_y = game.frog.y;
    int frog_x = game.frog.x;
    lane_type_t lane_type = game.lanes[frog_y].type;
    
    // Check if in river
    if (lane_type == LANE_RIVER) {
        game.frog.on_platform = false;
        
        // Check if on log/turtle
        for (int i = 0; i < game.object_count; i++) {
            game_object_t *obj = &game.objects[i];
            
            if (obj->y == frog_y) {
                // Check if frog is on this platform
                if (frog_x >= (int)obj->x && frog_x < (int)(obj->x + obj->width)) {
                    game.frog.on_platform = true;
                    
                    // Move with platform
                    if (obj->dir == DIR_LEFT) {
                        game.frog.x -= obj->speed * 0.016f;
                    } else {
                        game.frog.x += obj->speed * 0.016f;
                    }
                    
                    // Check if pushed off screen
                    if (game.frog.x < 0 || game.frog.x >= GRID_WIDTH) {
                        kill_frog();
                    }
                    break;
                }
            }
        }
        
        // Not on platform - drown!
        if (!game.frog.on_platform) {
            kill_frog();
        }
    }
    // Check if on road
    else if (lane_type == LANE_ROAD) {
        // Check collision with vehicles
        for (int i = 0; i < game.object_count; i++) {
            game_object_t *obj = &game.objects[i];
            
            if (obj->y == frog_y) {
                // Check overlap
                if (frog_x >= (int)obj->x && frog_x < (int)(obj->x + obj->width)) {
                    kill_frog();
                    return;
                }
            }
        }
    }
    // Check if reached goal
    else if (lane_type == LANE_SAFE_END) {
        // Calculate which goal spot (5 spots across top)
        int goal_index = frog_x / 3;  // Divide grid into 5 zones
        if (goal_index >= 0 && goal_index < 5 && !game.goals[goal_index]) {
            game.goals[goal_index] = true;
            game.score += 100;
            ESP_LOGI(TAG, "Goal reached! Score: %lu", (unsigned long)game.score);
            
            // Check if all goals reached
            bool all_goals = true;
            for (int i = 0; i < 5; i++) {
                if (!game.goals[i]) {
                    all_goals = false;
                    break;
                }
            }
            
            if (all_goals) {
                game.level_complete = true;
                ESP_LOGI(TAG, "Level complete!");
            }
            
            // Respawn frog
            game.frog.x = GRID_WIDTH / 2;
            game.frog.y = 1;
        }
    }
}

static void move_frog(int dx, int dy) {
    if (!game.frog.alive || game.game_over || game.level_complete) return;
    
    int new_x = game.frog.x + dx;
    int new_y = game.frog.y + dy;
    
    // Check boundaries
    if (new_x < 0 || new_x >= GRID_WIDTH) return;
    if (new_y < 0 || new_y >= GRID_HEIGHT) return;
    
    // Move frog
    game.frog.x = new_x;
    game.frog.y = new_y;
    
    // Score for forward progress
    if (dy > 0) {
        game.score += 10;
    }
}

static void kill_frog(void) {
    ESP_LOGI(TAG, "Frog died!");
    game.frog.alive = false;
    game.lives--;
    
    if (game.lives <= 0) {
        game.game_over = true;
        ESP_LOGI(TAG, "Game Over! Final score: %lu", (unsigned long)game.score);
    } else {
        // Respawn after delay
        vTaskDelay(pdMS_TO_TICKS(1000));
        game.frog.x = GRID_WIDTH / 2;
        game.frog.y = 1;
        game.frog.alive = true;
        game.frog.on_platform = false;
    }
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

void frogger_handle_input(void) {
    if (game.game_over) {
        if (read_button(BTN_B, 5)) {
            ESP_LOGI(TAG, "Button B pressed - returning to launcher");
            return_to_launcher();
        }
        return;
    }
    
    if (game.level_complete) {
        // Button A - Next level
        if (read_button(BTN_A, 4)) {
            game.level++;
            game.level_complete = false;
            game.time_remaining = 60;
            memset(game.goals, 0, sizeof(game.goals));
            game.frog.x = GRID_WIDTH / 2;
            game.frog.y = 1;
            init_level();
        }
        // Button B - Return to launcher
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
    
    // Movement
    if (read_button(BTN_UP, 0)) {
        ESP_LOGI(TAG, "UP pressed - moving frog");
        move_frog(0, 1);
    }
    else if (read_button(BTN_DOWN, 1)) {
        ESP_LOGI(TAG, "DOWN pressed - moving frog");
        move_frog(0, -1);
    }
    else if (read_button(BTN_LEFT, 2)) {
        ESP_LOGI(TAG, "LEFT pressed - moving frog");
        move_frog(-1, 0);
    }
    else if (read_button(BTN_RIGHT, 3)) {
        ESP_LOGI(TAG, "RIGHT pressed - moving frog");
        move_frog(1, 0);
    }
    
    // Pause game (Button B during gameplay)
    if (read_button(BTN_B, 5)) {
        ESP_LOGI(TAG, "Button B pressed - pausing game");
        game.paused = true;
    }
}

void frogger_update(void) {
    if (game.game_over || game.level_complete || game.paused) return;
    
    game.game_tick++;
    
    // Update timer (every 60 ticks = 1 second)
    if (game.game_tick % 60 == 0) {
        if (game.time_remaining > 0) {
            game.time_remaining--;
        } else {
            kill_frog();
        }
    }
    
    // Update objects
    update_objects();
    
    // Check collisions
    check_collisions();
    
    // Animation
    if (game.game_tick % 15 == 0) {
        game.frog.anim_frame = (game.frog.anim_frame + 1) % 2;
    }
}

void frogger_render(void) {
    // Track last frog position to clear trails
    static int last_frog_x = -1;
    static int last_frog_y = -1;
    
    // Clear old frog position if it moved
    if (last_frog_x >= 0 && (last_frog_x != game.frog.x || last_frog_y != game.frog.y)) {
        int last_sy = grid_to_screen_y(last_frog_y);
        // Get lane color to clear with
        lane_type_t type = game.lanes[last_frog_y].type;
        uint16_t color = (type == LANE_ROAD) ? COLOR_GRAY : 
                        (type == LANE_RIVER) ? COLOR_BLUE : COLOR_DARK_GREEN;
        int last_sx = grid_to_screen_x(last_frog_x);
        lcd_fill_rect(last_sx, last_sy, GRID_SIZE, GRID_SIZE, color);
    }
    
    // Draw frog at new position
    if (game.frog.alive) {
        draw_frog();
        last_frog_x = game.frog.x;
        last_frog_y = game.frog.y;
    } else {
        last_frog_x = -1;
        last_frog_y = -1;
    }
    
    // Update UI only every 10 frames to save time
    static int ui_counter = 0;
    if (++ui_counter >= 10) {
        ui_counter = 0;
        draw_ui();
    }
}

static void draw_lane(int y) {
    int sy = grid_to_screen_y(y);
    lane_type_t type = game.lanes[y].type;
    uint16_t color;
    
    switch (type) {
        case LANE_SAFE_START:
        case LANE_SAFE_MID:
            color = COLOR_DARK_GREEN;
            break;
        case LANE_ROAD:
            color = COLOR_GRAY;
            break;
        case LANE_RIVER:
            color = COLOR_BLUE;
            break;
        case LANE_SAFE_END:
            color = COLOR_GREEN;
            break;
        default:
            color = COLOR_BLACK;
    }
    
    lcd_fill_rect(0, sy, SCREEN_WIDTH, GRID_SIZE, color);
    
    // Draw goals
    if (type == LANE_SAFE_END) {
        for (int i = 0; i < 5; i++) {
            int gx = i * 3 + 1;
            int gsx = grid_to_screen_x(gx);
            lcd_fill_rect(gsx, sy, GRID_SIZE * 2, GRID_SIZE, 
                         game.goals[i] ? COLOR_YELLOW : COLOR_DARK_GREEN);
            lcd_draw_rect(gsx, sy, GRID_SIZE * 2, GRID_SIZE, COLOR_WHITE);
        }
    }
}

static void draw_object(game_object_t *obj) {
    int sx = grid_to_screen_x((int)obj->x);
    int sy = grid_to_screen_y(obj->y);
    int width = obj->width * GRID_SIZE;
    
    // Draw based on type
    if (obj->type >= OBJ_LOG_SHORT && obj->type <= OBJ_LOG_LONG) {
        // Logs
        lcd_fill_rect(sx, sy + 2, width, GRID_SIZE - 4, COLOR_BROWN);
        lcd_draw_rect(sx, sy + 2, width, GRID_SIZE - 4, COLOR_BLACK);
    } else if (obj->type == OBJ_TURTLE) {
        // Turtles
        for (int i = 0; i < obj->width; i++) {
            lcd_fill_circle(sx + i * GRID_SIZE + GRID_SIZE/2, sy + GRID_SIZE/2, 
                           GRID_SIZE/2 - 2, COLOR_GREEN);
        }
    } else {
        // Cars/trucks
        lcd_fill_rect(sx, sy + 3, width, GRID_SIZE - 6, obj->color);
        lcd_draw_rect(sx, sy + 3, width, GRID_SIZE - 6, COLOR_BLACK);
        // Windows
        lcd_fill_rect(sx + 2, sy + 5, width - 4, 4, COLOR_LIGHT_BLUE);
    }
}

static void draw_frog(void) {
    int sx = grid_to_screen_x(game.frog.x);
    int sy = grid_to_screen_y(game.frog.y);
    
    // ULTRA BRIGHT GREEN - use explicit RGB565 value
    // RGB565: Green = 0b00000_111111_00000 = 0x07E0
    uint16_t bright_green = 0x07E0;
    
    // Large filled square with bright green
    lcd_fill_rect(sx + 1, sy + 1, GRID_SIZE - 2, GRID_SIZE - 2, bright_green);
    
    // Thick white border to make it stand out
    lcd_draw_rect(sx, sy, GRID_SIZE, GRID_SIZE, 0xFFFF);
    lcd_draw_rect(sx + 1, sy + 1, GRID_SIZE - 2, GRID_SIZE - 2, 0xFFFF);
    
    // Big yellow eyes
    lcd_fill_rect(sx + 3, sy + 3, 4, 4, 0xFFE0);
    lcd_fill_rect(sx + GRID_SIZE - 7, sy + 3, 4, 4, 0xFFE0);
}

static void draw_ui(void) {
    char buf[32];
    
    // Black bar at bottom for UI
    lcd_fill_rect(0, 0, SCREEN_WIDTH, UI_HEIGHT, COLOR_BLACK);
    
    // Score
    snprintf(buf, sizeof(buf), "S:%lu", (unsigned long)game.score);
    lcd_draw_string(5, 4, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Lives
    snprintf(buf, sizeof(buf), "L:%d", game.lives);
    lcd_draw_string(80, 4, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Time
    snprintf(buf, sizeof(buf), "T:%lu", (unsigned long)game.time_remaining);
    lcd_draw_string(130, 4, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Level
    snprintf(buf, sizeof(buf), "LV:%d", game.level);
    lcd_draw_string(190, 4, buf, COLOR_WHITE, COLOR_BLACK);
    
    // Game state messages
    if (game.game_over) {
        lcd_fill_rect(40, SCREEN_HEIGHT/2 - 30, 160, 65, COLOR_BLACK);
        lcd_draw_rect(40, SCREEN_HEIGHT/2 - 30, 160, 65, COLOR_RED);
        lcd_draw_string(70, SCREEN_HEIGHT/2 - 20, "GAME OVER", COLOR_RED, COLOR_BLACK);
        snprintf(buf, sizeof(buf), "SCORE:%lu", (unsigned long)game.score);
        lcd_draw_string(60, SCREEN_HEIGHT/2 - 5, buf, COLOR_WHITE, COLOR_BLACK);
        lcd_draw_string(45, SCREEN_HEIGHT/2 + 10, "Press B to", COLOR_WHITE, COLOR_BLACK);
        lcd_draw_string(40, SCREEN_HEIGHT/2 + 25, "return to menu", COLOR_WHITE, COLOR_BLACK);
    } else if (game.level_complete) {
        lcd_fill_rect(40, SCREEN_HEIGHT/2 - 30, 160, 70, COLOR_BLACK);
        lcd_draw_rect(40, SCREEN_HEIGHT/2 - 30, 160, 70, COLOR_GREEN);
        lcd_draw_string(50, SCREEN_HEIGHT/2 - 20, "LEVEL DONE!", COLOR_GREEN, COLOR_BLACK);
        lcd_draw_string(50, SCREEN_HEIGHT/2 - 5, "A: Next level", COLOR_WHITE, COLOR_BLACK);
        lcd_draw_string(50, SCREEN_HEIGHT/2 + 10, "B: Exit to menu", COLOR_WHITE, COLOR_BLACK);
    } else if (game.paused) {
        lcd_fill_rect(40, SCREEN_HEIGHT/2 - 30, 160, 60, COLOR_BLACK);
        lcd_draw_rect(40, SCREEN_HEIGHT/2 - 30, 160, 60, COLOR_YELLOW);
        lcd_draw_string(75, SCREEN_HEIGHT/2 - 20, "PAUSED", COLOR_YELLOW, COLOR_BLACK);
        lcd_draw_string(50, SCREEN_HEIGHT/2 - 5, "A: Resume", COLOR_WHITE, COLOR_BLACK);
        lcd_draw_string(50, SCREEN_HEIGHT/2 + 10, "B: Exit to menu", COLOR_WHITE, COLOR_BLACK);
    }
}

static int grid_to_screen_x(int gx) {
    return gx * GRID_SIZE;
}

static int grid_to_screen_y(int gy) {
    // Flip Y so row 0 is at bottom
    return SCREEN_HEIGHT - (gy + 1) * GRID_SIZE;
}

void frogger_game_loop(void) {
    static int loop_count = 0;
    if (++loop_count % 60 == 0) {
        ESP_LOGI(TAG, "Game loop running... (frame %d)", loop_count);
    }
    
    frogger_handle_input();
    frogger_update();
    frogger_render();
}
