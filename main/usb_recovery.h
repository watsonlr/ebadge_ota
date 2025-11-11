/**
 * @file usb_recovery.h
 * @brief USB flash drive bootloader recovery mode
 */

#ifndef USB_RECOVERY_H
#define USB_RECOVERY_H

#include "esp_err.h"
#include <stdbool.h>

#define BOOT_BUTTON_GPIO GPIO_NUM_0
#define BOOTLOADER_OFFSET 0x1000
#define BOOTLOADER_MAX_SIZE (64 * 1024)

/**
 * @brief Check if recovery mode should be triggered
 * 
 * Checks if BOOT button is held during startup
 * 
 * @return true if recovery mode triggered, false otherwise
 */
bool usb_recovery_check_trigger(void);

/**
 * @brief Attempt to recover bootloader from USB drive
 * 
 * Mounts USB drive, looks for /bootloader.bin, verifies and flashes it
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t usb_recovery_reflash_bootloader(void);

#endif // USB_RECOVERY_H
