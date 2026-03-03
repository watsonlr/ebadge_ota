/**
 * @file tetris_main.c
 * @brief Entry point for Tetris game
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tetris_game.h"

static const char *TAG = "main";

void app_main(void) {
    ESP_LOGI(TAG, "Starting Tetris Game!");
    
    // Initialize NVS (for high scores later)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize game
    ESP_ERROR_CHECK(tetris_init());
    
    ESP_LOGI(TAG, "Game initialized, starting main loop");
    
    // Main game loop
    while (1) {
        tetris_game_loop();
        vTaskDelay(pdMS_TO_TICKS(16));  // ~60 FPS
    }
}
