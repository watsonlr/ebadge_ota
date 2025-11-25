/**
 * @file wifi_manager.c
 * @brief Wi-Fi connection management implementation
 */

#include "wifi_manager.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

#define WIFI_SSID      CONFIG_WIFI_SSID
#define WIFI_PASSWORD  CONFIG_WIFI_PASSWORD
#define WIFI_MAXIMUM_RETRY  CONFIG_WIFI_MAXIMUM_RETRY
#define NVS_NAMESPACE "wifi"
#define NVS_KEY_SSID  "ssid"
#define NVS_KEY_PASS  "pass"

static const char *TAG = "wifi_manager";
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        // Association succeeded; log SSID immediately (before DHCP/IP assignment)
        wifi_event_sta_connected_t *connected = (wifi_event_sta_connected_t *)event_data;
        // Ensure SSID is null-terminated for logging (ssid_len <= 32 per spec)
        char ssid_buf[33];
        size_t copy_len = connected->ssid_len < sizeof(ssid_buf)-1 ? connected->ssid_len : sizeof(ssid_buf)-1;
        memcpy(ssid_buf, connected->ssid, copy_len);
        ssid_buf[copy_len] = '\0';
        ESP_LOGI(TAG, "Associated with SSID '%s' (channel %d, authmode %d)", ssid_buf, connected->channel, connected->authmode);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry connecting to AP (%d/%d)", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "Connect to AP failed");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        // Try to get the SSID we joined and include it in the log
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            ESP_LOGI(TAG, "Joined SSID '%s' and got IP: " IPSTR, (char*)ap_info.ssid, IP2STR(&event->ip_info.ip));
        } else {
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        }
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_manager_init(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = { 0 }; // zero-init
    
    // Try to load credentials from NVS first (saved by provisioning)
    nvs_handle_t nvs_h;
    size_t ssid_len = sizeof(wifi_config.sta.ssid);
    size_t pass_len = sizeof(wifi_config.sta.password);
    bool nvs_creds_found = false;
    
    if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_h) == ESP_OK) {
        if (nvs_get_str(nvs_h, NVS_KEY_SSID, (char*)wifi_config.sta.ssid, &ssid_len) == ESP_OK &&
            nvs_get_str(nvs_h, NVS_KEY_PASS, (char*)wifi_config.sta.password, &pass_len) == ESP_OK) {
            nvs_creds_found = true;
            ESP_LOGI(TAG, "Using provisioned credentials from NVS");
        }
        nvs_close(nvs_h);
    }
    
    // Fall back to CONFIG defaults if no NVS credentials
    if (!nvs_creds_found) {
        strlcpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
        strlcpy((char*)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
        ESP_LOGI(TAG, "Using default credentials from menuconfig");
    }

    // Handle open network (no password) differently so authmode threshold doesn't block connection
    if (strlen((char*)wifi_config.sta.password) == 0) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        ESP_LOGI(TAG, "Configuring open Wi-Fi network: %s", wifi_config.sta.ssid);
    } else {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_LOGI(TAG, "Configuring secured Wi-Fi network: %s", wifi_config.sta.ssid);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi initialization finished. Connecting to SSID:%s", wifi_config.sta.ssid);

    return ESP_OK;
}

esp_err_t wifi_manager_wait_connected(uint32_t timeout_ms)
{
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            pdMS_TO_TICKS(timeout_ms));

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Connected to AP successfully");
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE(TAG, "Failed to connect to AP");
        return ESP_FAIL;
    } else {
        ESP_LOGE(TAG, "Connection timeout");
        return ESP_ERR_TIMEOUT;
    }
}

bool wifi_manager_is_connected(void)
{
    EventBits_t bits = xEventGroupGetBits(s_wifi_event_group);
    return (bits & WIFI_CONNECTED_BIT) != 0;
}

void wifi_manager_deinit(void)
{
    esp_wifi_stop();
    esp_wifi_deinit();
    if (s_wifi_event_group) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }
    s_retry_num = 0;
}
