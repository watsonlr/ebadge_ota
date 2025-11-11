/**
 * @file ota_manager.h
 * @brief OTA download and flashing management
 */

#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include "esp_err.h"
#include <stdint.h>

#define MAX_APPS 10
#define MAX_APP_NAME_LEN 64
#define MAX_URL_LEN 256

/**
 * @brief Structure representing an available app in the manifest
 */
typedef struct {
    char name[MAX_APP_NAME_LEN];
    char version[16];
    char url[MAX_URL_LEN];
} app_info_t;

/**
 * @brief Structure representing the app manifest
 */
typedef struct {
    app_info_t apps[MAX_APPS];
    int app_count;
} app_manifest_t;

/**
 * @brief Fetch the app manifest from the server
 * 
 * @param manifest_url URL of the manifest JSON file
 * @param manifest Pointer to store the parsed manifest
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t ota_manager_fetch_manifest(const char *manifest_url, app_manifest_t *manifest);

/**
 * @brief Download and install an app from URL
 * 
 * @param app_url URL of the app binary
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t ota_manager_download_and_install(const char *app_url);

/**
 * @brief Display available apps from manifest
 * 
 * @param manifest Pointer to the manifest
 */
void ota_manager_display_apps(const app_manifest_t *manifest);

/**
 * @brief Get the currently running partition info
 */
void ota_manager_print_partition_info(void);

#endif // OTA_MANAGER_H
