/**
 * @file wifi_manager.h
 * @brief Wi-Fi connection management for OTA loader
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "esp_err.h"
#include <stdbool.h>

/**
 * @brief Initialize and connect to Wi-Fi
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_manager_init(void);

/**
 * @brief Wait for Wi-Fi connection
 * 
 * @param timeout_ms Timeout in milliseconds
 * @return ESP_OK if connected, ESP_ERR_TIMEOUT otherwise
 */
esp_err_t wifi_manager_wait_connected(uint32_t timeout_ms);

/**
 * @brief Check if Wi-Fi is connected
 * 
 * @return true if connected, false otherwise
 */
bool wifi_manager_is_connected(void);

/**
 * @brief Deinitialize Wi-Fi manager
 * 
 * Stops Wi-Fi and cleans up resources
 */
void wifi_manager_deinit(void);

#endif // WIFI_MANAGER_H
