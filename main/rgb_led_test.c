/**
 * @file rgb_led_test.c
 * @brief Simple RGB LED test program for ESP32-S3
 * 
 * Controls the RGB LED on BYUI e-Badge V3.0:
 * - GPIO 6: Red LED
 * - GPIO 5: Green LED
 * - GPIO 4: Blue LED
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// RGB LED GPIO pins
#define RGB_RED_PIN    GPIO_NUM_6
#define RGB_GREEN_PIN  GPIO_NUM_5
#define RGB_BLUE_PIN   GPIO_NUM_4

static const char *TAG = "rgb_led";

/**
 * @brief Initialize RGB LED pins
 */
void rgb_led_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RGB_RED_PIN) | (1ULL << RGB_GREEN_PIN) | (1ULL << RGB_BLUE_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "RGB LED initialized on pins R:%d G:%d B:%d", RGB_RED_PIN, RGB_GREEN_PIN, RGB_BLUE_PIN);
}

/**
 * @brief Set RGB LED color
 * @param red Red value (0=off, 1=on)
 * @param green Green value (0=off, 1=on)
 * @param blue Blue value (0=off, 1=on)
 */
void rgb_led_set(uint8_t red, uint8_t green, uint8_t blue)
{
    gpio_set_level(RGB_RED_PIN, red);
    gpio_set_level(RGB_GREEN_PIN, green);
    gpio_set_level(RGB_BLUE_PIN, blue);
}

/**
 * @brief Main application entry point
 */
void app_main(void)
{
    ESP_LOGI(TAG, "RGB LED Test Program Starting...");
    
    // Initialize the RGB LED
    rgb_led_init();
    
    // Turn all LEDs off initially
    rgb_led_set(0, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    while(1) {
        // Red
        ESP_LOGI(TAG, "Setting LED to RED");
        rgb_led_set(1, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Green
        ESP_LOGI(TAG, "Setting LED to GREEN");
        rgb_led_set(0, 1, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Blue
        ESP_LOGI(TAG, "Setting LED to BLUE");
        rgb_led_set(0, 0, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Yellow (Red + Green)
        ESP_LOGI(TAG, "Setting LED to YELLOW");
        rgb_led_set(1, 1, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Cyan (Green + Blue)
        ESP_LOGI(TAG, "Setting LED to CYAN");
        rgb_led_set(0, 1, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Magenta (Red + Blue)
        ESP_LOGI(TAG, "Setting LED to MAGENTA");
        rgb_led_set(1, 0, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // White (All on)
        ESP_LOGI(TAG, "Setting LED to WHITE");
        rgb_led_set(1, 1, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        // Off
        ESP_LOGI(TAG, "Setting LED to OFF");
        rgb_led_set(0, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
