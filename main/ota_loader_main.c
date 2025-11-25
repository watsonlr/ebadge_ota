/**
 * @file ota_loader_main.c
 * @brief Main OTA loader application
 * 
 * This is the factory/loader firmware that lives permanently on the ESP32-C3.
 * It provides:
 * - USB recovery mode for bootloader reflashing
 * - Wi-Fi connectivity
 * - App manifest fetching from server
 * - OTA download and installation of apps
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"

#include "wifi_manager.h"
#include "ota_manager.h"
#include "usb_recovery.h"
#include "provisioning.h"

static const char *TAG = "ota_loader";

// Default manifest URL - configure via menuconfig
#define MANIFEST_URL CONFIG_MANIFEST_URL

void print_banner(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║    ESP32-C3 OTA Loader & App Manager      ║\n");
    printf("║                                            ║\n");
    printf("║  Factory firmware - cannot be overwritten ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
}

void show_menu(void)
{
    printf("\n=== Main Menu ===\n");
    printf("1. Fetch and display available apps\n");
    printf("2. Download and install an app\n");
    printf("3. Show partition information\n");
    printf("4. Reboot\n");
    printf("0. Return to factory (this loader)\n");
    printf("==================\n");
    printf("Enter choice: ");
}

int get_user_choice(void)
{
    char input[10];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        return atoi(input);
    }
    return -1;
}

void handle_fetch_apps(void)
{
    app_manifest_t manifest;
    
    printf("\nFetching app manifest from: %s\n", MANIFEST_URL);
    
    esp_err_t err = ota_manager_fetch_manifest(MANIFEST_URL, &manifest);
    if (err != ESP_OK) {
        printf("Failed to fetch manifest: %s\n", esp_err_to_name(err));
        return;
    }
    
    ota_manager_display_apps(&manifest);
}

void handle_install_app(void)
{
    app_manifest_t manifest;
    
    printf("\nFetching app manifest...\n");
    
    esp_err_t err = ota_manager_fetch_manifest(MANIFEST_URL, &manifest);
    if (err != ESP_OK) {
        printf("Failed to fetch manifest: %s\n", esp_err_to_name(err));
        return;
    }
    
    ota_manager_display_apps(&manifest);
    
    if (manifest.app_count == 0) {
        printf("No apps available.\n");
        return;
    }
    
    printf("\nEnter app number to install (0-%d): ", manifest.app_count - 1);
    int choice = get_user_choice();
    
    if (choice < 0 || choice >= manifest.app_count) {
        printf("Invalid selection.\n");
        return;
    }
    
    printf("\nInstalling: %s (v%s)\n", 
           manifest.apps[choice].name, 
           manifest.apps[choice].version);
    printf("URL: %s\n", manifest.apps[choice].url);
    
    err = ota_manager_download_and_install(manifest.apps[choice].url);
    if (err != ESP_OK) {
        printf("Installation failed: %s\n", esp_err_to_name(err));
    }
    // Note: If successful, device will reboot into the new app
}

void handle_return_to_factory(void)
{
    printf("\nReturning to factory partition (this loader)...\n");
    
    const esp_partition_t *factory = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, 
        ESP_PARTITION_SUBTYPE_APP_FACTORY, 
        NULL
    );
    
    if (factory == NULL) {
        printf("Factory partition not found!\n");
        return;
    }
    
    esp_err_t err = esp_ota_set_boot_partition(factory);
    if (err != ESP_OK) {
        printf("Failed to set boot partition: %s\n", esp_err_to_name(err));
        return;
    }
    
    printf("Boot partition set to factory. Rebooting...\n");
    vTaskDelay(pdMS_TO_TICKS(2000));
    esp_restart();
}

void app_main(void)
{
    esp_err_t ret;
    
    print_banner();
    
    // Check for USB recovery mode (BOOT button pressed)
    if (usb_recovery_check_trigger()) {
        printf("\n*** RECOVERY MODE TRIGGERED ***\n");
        printf("Attempting to recover bootloader from USB drive...\n\n");
        
        ret = usb_recovery_reflash_bootloader();
        if (ret != ESP_OK) {
            printf("\nRecovery failed: %s\n", esp_err_to_name(ret));
            printf("Continuing with normal boot...\n\n");
            vTaskDelay(pdMS_TO_TICKS(3000));
        }
        // If recovery succeeds, device will reboot
    }
    
    // Initialize NVS (required for Wi-Fi)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Display current partition info
    ota_manager_print_partition_info();
    
    // Start directly in provisioning mode
    printf("\n=== Starting Provisioning Mode ===\n");
    
    // Initialize network stack
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Start SoftAP provisioning portal
    ret = provisioning_start_softap("BYUI_NameBadge", "", 6);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start provisioning portal: %s", esp_err_to_name(ret));
        printf("\n⚠️  ERROR: Could not start provisioning portal!\n");
        printf("Check serial output for details.\n");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
    
    printf("\n╔════════════════════════════════════════════╗\n");
    printf("║   Wi-Fi Provisioning Mode Active          ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n1. Connect your phone/laptop to Wi-Fi: BYUI_NameBadge (open)\n");
    printf("2. Open browser to: http://192.168.4.1/\n");
    printf("3. Click 'Scan Networks' to see available networks\n");
    printf("4. Enter SSID and password, click Save\n");
    printf("5. Reboot the device\n\n");
    printf("Provisioning portal is running...\n");
    printf("Press Ctrl+C to exit and reboot manually.\n\n");
    
    // Wait indefinitely for user to provision via web portal
    while (!provisioning_was_configured()) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    // Credentials saved - now connect to the configured network
    printf("\n✓ Credentials saved!\n");
    printf("Stopping provisioning portal and connecting to configured network...\n\n");
    
    // Stop provisioning
    provisioning_stop();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Initialize wifi_manager and connect
    ret = wifi_manager_init();
    if (ret == ESP_OK) {
        printf("Connecting to saved Wi-Fi network...\n");
        ret = wifi_manager_wait_connected(30000); // 30s timeout
        if (ret == ESP_OK) {
            printf("\n✓ Wi-Fi connected successfully!\n\n");
        } else {
            ESP_LOGE(TAG, "Failed to connect to saved network");
            printf("\n⚠️  Could not connect to the saved network.\n");
            printf("Please check credentials and reboot to try again.\n");
        }
    }
    
    // Continue to main menu
    printf("\n=== Main Menu ===\n\n");
    
    while (1) {
        show_menu();
        int choice = get_user_choice();
        
        switch (choice) {
            case 1:
                handle_fetch_apps();
                break;
                
            case 2:
                handle_install_app();
                break;
                
            case 3:
                ota_manager_print_partition_info();
                break;
                
            case 4:
                printf("\nRebooting...\n");
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
                break;
                
            case 0:
                handle_return_to_factory();
                break;
                
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
