/**
 * @file menu.c
 * @brief Game launcher menu implementation
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "menu.h"
#include "lcd_driver.h"

static const char *TAG = "MENU";

// Global menu state
static menu_state_t menu_state;

// Button states for debouncing
static uint8_t btn_state[6] = {1, 1, 1, 1, 1, 1};
static uint8_t btn_last[6] = {1, 1, 1, 1, 1, 1};
static uint32_t btn_debounce[6] = {0};

// Game database - add more games here
static game_info_t game_database[] = {
    {
        .name = "PAC-MAN",
        .description = "Maze Chase",
        .ota_url = "http://192.168.1.100:8080/pacman.bin",
        .color = COLOR_YELLOW,
        .available = true
    },
    {
        .name = "TETRIS",
        .description = "Stack Blocks",
        .ota_url = "http://192.168.1.100:8080/tetris.bin",
        .color = COLOR_CYAN,
        .available = true
    },
    {
        .name = "FROGGER",
        .description = "Cross River",
        .ota_url = "http://192.168.1.100:8080/frogger.bin",
        .color = COLOR_GREEN,
        .available = true
    }
};

/**
 * @brief Initialize button GPIOs
 */
static void init_buttons(void) {
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    
    io_conf.pin_bit_mask = (1ULL << BTN_UP) | (1ULL << BTN_DOWN) | 
                           (1ULL << BTN_LEFT) | (1ULL << BTN_RIGHT) |
                           (1ULL << BTN_A) | (1ULL << BTN_B);
    gpio_config(&io_conf);
}

/**
 * @brief Read button with debouncing
 * @param btn_idx Button index (0-5)
 * @param gpio_num GPIO pin number
 * @return true if button just pressed
 */
static bool read_button(int btn_idx, int gpio_num) {
    uint32_t now = xTaskGetTickCount();
    uint8_t current = gpio_get_level(gpio_num);
    
    if (current != btn_last[btn_idx]) {
        btn_debounce[btn_idx] = now;
    }
    btn_last[btn_idx] = current;
    
    if ((now - btn_debounce[btn_idx]) > pdMS_TO_TICKS(50)) {
        if (current != btn_state[btn_idx]) {
            btn_state[btn_idx] = current;
            if (current == 0) {  // Button pressed (active low)
                return true;
            }
        }
    }
    
    return false;
}

/**
 * @brief Draw a game icon
 */
static void draw_game_icon(int x, int y, int game_idx, bool selected) {
    uint16_t color = game_database[game_idx].color;
    uint16_t bg_color = selected ? COLOR_WHITE : COLOR_DARK_GRAY;
    
    // Icon background
    lcd_fill_rect(x, y, 40, 40, bg_color);
    
    // Draw game-specific icon
    switch (game_idx) {
        case 0:  // Pac-Man
            lcd_draw_circle(x + 20, y + 20, 15, color);
            // Mouth
            lcd_fill_rect(x + 20, y + 18, 15, 4, bg_color);
            break;
            
        case 1:  // Tetris
            // Draw T-piece
            lcd_fill_rect(x + 10, y + 15, 8, 8, color);
            lcd_fill_rect(x + 18, y + 15, 8, 8, color);
            lcd_fill_rect(x + 26, y + 15, 8, 8, color);
            lcd_fill_rect(x + 18, y + 23, 8, 8, color);
            break;
            
        case 2:  // Frogger
            // Draw simple frog
            lcd_fill_rect(x + 15, y + 20, 10, 12, color);  // Body
            lcd_fill_rect(x + 10, y + 18, 6, 6, color);    // Left eye
            lcd_fill_rect(x + 28, y + 18, 6, 6, color);    // Right eye
            break;
            
        default:
            // Generic icon
            lcd_fill_rect(x + 10, y + 10, 20, 20, color);
            break;
    }
}

/**
 * @brief Draw menu header
 */
static void draw_header(void) {
    lcd_fill_rect(0, 0, SCREEN_WIDTH, 50, COLOR_BLUE);
    lcd_draw_string(5, 18, "GAME LAUNCHER", COLOR_WHITE, COLOR_BLUE);
}

/**
 * @brief Draw a menu item
 */
static void draw_menu_item(int index, int y_pos, bool selected) {
    if (index >= menu_state.game_count) return;
    
    game_info_t *game = &menu_state.games[index];
    uint16_t bg_color = selected ? COLOR_LIGHT_GRAY : COLOR_BLACK;
    uint16_t text_color = selected ? COLOR_BLACK : COLOR_WHITE;
    uint16_t border_color = selected ? game->color : COLOR_DARK_GRAY;
    
    // Background
    lcd_fill_rect(5, y_pos, SCREEN_WIDTH - 10, MENU_ITEM_HEIGHT - 5, bg_color);
    
    // Border
    for (int i = 0; i < 3; i++) {
        lcd_draw_rect(5 + i, y_pos + i, SCREEN_WIDTH - 10 - 2*i, 
                      MENU_ITEM_HEIGHT - 5 - 2*i, border_color);
    }
    
    // Icon
    draw_game_icon(15, y_pos + 8, index, selected);
    
    // Game name (centered vertically in the box)
    lcd_draw_string(65, y_pos + 20, game->name, text_color, bg_color);
}

/**
 * @brief Draw scrollbar
 */
static void draw_scrollbar(void) {
    if (menu_state.game_count <= 3) return;
    
    int scrollbar_height = (SCREEN_HEIGHT - MENU_OFFSET_Y) * 3 / menu_state.game_count;
    int scrollbar_pos = MENU_OFFSET_Y + 
                       (menu_state.selected_index * (SCREEN_HEIGHT - MENU_OFFSET_Y - scrollbar_height) / 
                        (menu_state.game_count - 1));
    
    lcd_fill_rect(SCREEN_WIDTH - 8, MENU_OFFSET_Y, 6, 
                  SCREEN_HEIGHT - MENU_OFFSET_Y, COLOR_DARK_GRAY);
    lcd_fill_rect(SCREEN_WIDTH - 8, scrollbar_pos, 6, 
                  scrollbar_height, COLOR_CYAN);
}

/**
 * @brief Initialize the menu
 */
esp_err_t menu_init(void) {
    // Initialize LCD
    lcd_init();
    lcd_fill_screen(COLOR_BLACK);
    
    // Initialize buttons
    init_buttons();
    
    // Load game database
    menu_state.game_count = sizeof(game_database) / sizeof(game_info_t);
    memcpy(menu_state.games, game_database, sizeof(game_database));
    
    menu_state.selected_index = 0;
    menu_state.scroll_offset = 0;
    menu_state.in_game_info = false;
    menu_state.anim_tick = 0;
    menu_state.needs_redraw = true;
    menu_state.last_selected = 0;
    menu_state.full_redraw = true;
    
    ESP_LOGI(TAG, "Menu initialized with %d games", menu_state.game_count);
    
    return ESP_OK;
}

/**
 * @brief Handle button input
 */
void menu_handle_input(void) {
    if (read_button(3, BTN_RIGHT)) {  // RIGHT = move up in menu
        if (menu_state.selected_index > 0) {
            menu_state.selected_index--;
            menu_state.needs_redraw = true;
            ESP_LOGI(TAG, "Selected: %d", menu_state.selected_index);
        }
    }
    
    if (read_button(2, BTN_LEFT)) {  // LEFT = move down in menu
        if (menu_state.selected_index < menu_state.game_count - 1) {
            menu_state.selected_index++;
            menu_state.needs_redraw = true;
            ESP_LOGI(TAG, "Selected: %d", menu_state.selected_index);
        }
    }
    
    if (read_button(4, BTN_A)) {
        ESP_LOGI(TAG, "Launching game: %s", 
                 menu_state.games[menu_state.selected_index].name);
        menu_launch_game(menu_state.selected_index);
    }
    
    if (read_button(5, BTN_B)) {
        // Could show options/settings menu
        ESP_LOGI(TAG, "Options button pressed");
    }
}

/**
 * @brief Update menu state
 */
void menu_update(void) {
    menu_state.anim_tick++;
    
    // Auto-scroll to keep selected item visible
    int visible_items = 3;
    if (menu_state.selected_index < menu_state.scroll_offset) {
        menu_state.scroll_offset = menu_state.selected_index;
    }
    if (menu_state.selected_index >= menu_state.scroll_offset + visible_items) {
        menu_state.scroll_offset = menu_state.selected_index - visible_items + 1;
    }
}

/**
 * @brief Render the menu
 */
void menu_render(void) {
    // Only redraw if something changed
    if (!menu_state.needs_redraw) {
        return;
    }
    
    // Full redraw needed (first time or after screen was cleared)
    if (menu_state.full_redraw) {
        lcd_fill_screen(COLOR_BLACK);
        draw_header();
        
        // Draw all visible menu items
        int visible_items = 3;
        for (int i = 0; i < visible_items && (i + menu_state.scroll_offset) < menu_state.game_count; i++) {
            int game_idx = i + menu_state.scroll_offset;
            int y_pos = MENU_OFFSET_Y + i * MENU_ITEM_HEIGHT;
            bool selected = (game_idx == menu_state.selected_index);
            draw_menu_item(game_idx, y_pos, selected);
        }
        
        draw_scrollbar();
        lcd_draw_string(10, SCREEN_HEIGHT - 25, "L/R:Select A:Play", 
                        COLOR_GRAY, COLOR_BLACK);
        menu_state.full_redraw = false;
    } else {
        // Only redraw the two changed items
        int visible_items = 3;
        
        // Redraw previously selected item (now unselected)
        if (menu_state.last_selected != menu_state.selected_index) {
            for (int i = 0; i < visible_items; i++) {
                int game_idx = i + menu_state.scroll_offset;
                if (game_idx == menu_state.last_selected) {
                    int y_pos = MENU_OFFSET_Y + i * MENU_ITEM_HEIGHT;
                    draw_menu_item(game_idx, y_pos, false);
                }
                if (game_idx == menu_state.selected_index) {
                    int y_pos = MENU_OFFSET_Y + i * MENU_ITEM_HEIGHT;
                    draw_menu_item(game_idx, y_pos, true);
                }
            }
        }
    }
    
    menu_state.needs_redraw = false;
    menu_state.last_selected = menu_state.selected_index;
}

/**
 * @brief Launch selected game via OTA
 */
void menu_launch_game(int index) {
    if (index < 0 || index >= menu_state.game_count) return;
    
    game_info_t *game = &menu_state.games[index];
    
    // Show loading screen
    lcd_fill_screen(COLOR_BLACK);
    lcd_draw_string(50, 140, "LOADING...", COLOR_WHITE, COLOR_BLACK);
    lcd_draw_string(30, 160, game->name, game->color, COLOR_BLACK);
    
    ESP_LOGI(TAG, "Would launch: %s", game->name);
    ESP_LOGI(TAG, "OTA URL: %s", game->ota_url);
    
    // TODO: Implement actual OTA update
    // For now, just show a message
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    lcd_draw_string(20, 200, "OTA not yet implemented", COLOR_YELLOW, COLOR_BLACK);
    lcd_draw_string(20, 220, "Flash game manually", COLOR_GRAY, COLOR_BLACK);
    
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Force full menu redraw when returning
    menu_state.full_redraw = true;
    menu_state.needs_redraw = true;
}

/**
 * @brief Main menu loop
 */
void menu_loop(void) {
    menu_handle_input();
    menu_update();
    menu_render();
    vTaskDelay(pdMS_TO_TICKS(16));  // ~60 FPS
}
