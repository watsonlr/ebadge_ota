/**
 * @file launcher_main.c
 * @brief Main entry point for game launcher
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "menu.h"

static const char *TAG = "LAUNCHER";

void app_main(void)
{
    ESP_LOGI(TAG, "Game Launcher Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize menu system
    ESP_ERROR_CHECK(menu_init());
    
    ESP_LOGI(TAG, "Entering menu loop");
    
    // Main menu loop
    while (1) {
        menu_loop();
    }
}
