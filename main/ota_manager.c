/**
 * @file ota_manager.c
 * @brief OTA download and flashing implementation
 */

#include "ota_manager.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "ota_manager";

#define OTA_RECV_TIMEOUT_MS 5000
#define HASH_LEN 32

esp_err_t ota_manager_fetch_manifest(const char *manifest_url, app_manifest_t *manifest)
{
    if (!manifest_url || !manifest) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(manifest, 0, sizeof(app_manifest_t));

    ESP_LOGI(TAG, "Fetching manifest from: %s", manifest_url);

    // Configure HTTP client
    esp_http_client_config_t config = {
        .url = manifest_url,
        .timeout_ms = OTA_RECV_TIMEOUT_MS,
        .buffer_size = 2048,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return err;
    }

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length <= 0) {
        ESP_LOGE(TAG, "Invalid content length: %d", content_length);
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_FAIL;
    }

    // Allocate buffer for manifest
    char *buffer = malloc(content_length + 1);
    if (!buffer) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return ESP_ERR_NO_MEM;
    }

    // Read manifest data
    int total_read = 0;
    while (total_read < content_length) {
        int data_read = esp_http_client_read(client, buffer + total_read, content_length - total_read);
        if (data_read <= 0) {
            ESP_LOGE(TAG, "Error reading data");
            free(buffer);
            esp_http_client_close(client);
            esp_http_client_cleanup(client);
            return ESP_FAIL;
        }
        total_read += data_read;
    }
    buffer[total_read] = '\0';

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    ESP_LOGI(TAG, "Manifest downloaded successfully (%d bytes)", total_read);

    // Parse JSON
    cJSON *root = cJSON_Parse(buffer);
    free(buffer);

    if (root == NULL) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        return ESP_FAIL;
    }

    cJSON *apps = cJSON_GetObjectItem(root, "apps");
    if (!cJSON_IsArray(apps)) {
        ESP_LOGE(TAG, "Invalid manifest format: 'apps' array not found");
        cJSON_Delete(root);
        return ESP_FAIL;
    }

    int array_size = cJSON_GetArraySize(apps);
    manifest->app_count = (array_size > MAX_APPS) ? MAX_APPS : array_size;

    for (int i = 0; i < manifest->app_count; i++) {
        cJSON *app = cJSON_GetArrayItem(apps, i);
        cJSON *name = cJSON_GetObjectItem(app, "name");
        cJSON *version = cJSON_GetObjectItem(app, "version");
        cJSON *url = cJSON_GetObjectItem(app, "url");

        if (name && cJSON_IsString(name)) {
            strncpy(manifest->apps[i].name, name->valuestring, MAX_APP_NAME_LEN - 1);
        }
        if (version && cJSON_IsString(version)) {
            strncpy(manifest->apps[i].version, version->valuestring, 15);
        }
        if (url && cJSON_IsString(url)) {
            strncpy(manifest->apps[i].url, url->valuestring, MAX_URL_LEN - 1);
        }
    }

    cJSON_Delete(root);
    ESP_LOGI(TAG, "Parsed %d apps from manifest", manifest->app_count);

    return ESP_OK;
}

esp_err_t ota_manager_download_and_install(const char *app_url)
{
    if (!app_url) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Starting OTA update from: %s", app_url);

    esp_http_client_config_t config = {
        .url = app_url,
        .timeout_ms = OTA_RECV_TIMEOUT_MS,
        .keep_alive_enable = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t https_ota_handle = NULL;
    esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed: %s", esp_err_to_name(err));
        return err;
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get app description: %s", esp_err_to_name(err));
        esp_https_ota_abort(https_ota_handle);
        return err;
    }

    ESP_LOGI(TAG, "New app version: %s", app_desc.version);
    ESP_LOGI(TAG, "New app project: %s", app_desc.project_name);

    // Download and write to partition
    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        ESP_LOGD(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        ESP_LOGE(TAG, "Complete data was not received.");
        err = ESP_FAIL;
    } else {
        err = esp_https_ota_finish(https_ota_handle);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "OTA update successful!");
            ESP_LOGI(TAG, "Total bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
            ESP_LOGI(TAG, "Rebooting in 3 seconds...");
            vTaskDelay(pdMS_TO_TICKS(3000));
            esp_restart();
        } else {
            ESP_LOGE(TAG, "ESP HTTPS OTA finish failed: %s", esp_err_to_name(err));
        }
    }

    esp_https_ota_abort(https_ota_handle);
    return err;
}

void ota_manager_display_apps(const app_manifest_t *manifest)
{
    if (!manifest || manifest->app_count == 0) {
        printf("\nNo apps available.\n");
        return;
    }

    printf("\n=== Available Apps ===\n");
    for (int i = 0; i < manifest->app_count; i++) {
        printf("%d: %s (v%s)\n", i, manifest->apps[i].name, manifest->apps[i].version);
    }
    printf("======================\n");
}

void ota_manager_print_partition_info(void)
{
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *boot = esp_ota_get_boot_partition();
    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);

    ESP_LOGI(TAG, "=== Partition Information ===");
    if (running) {
        ESP_LOGI(TAG, "Running partition: %s (type %d, subtype %d, offset 0x%lx, size 0x%lx)",
                 running->label, running->type, running->subtype, running->address, running->size);
    }
    if (boot) {
        ESP_LOGI(TAG, "Boot partition: %s", boot->label);
    }
    if (next) {
        ESP_LOGI(TAG, "Next update partition: %s", next->label);
    }
    ESP_LOGI(TAG, "============================");
}
