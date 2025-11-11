# SPDX-FileCopyrightText: 2025
# SPDX-License-Identifier: MIT

idf_component_register(
    SRCS 
        "ota_loader_main.c"
        "wifi_manager.c"
        "ota_manager.c"
        "usb_recovery.c"
        "provisioning.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash
        esp_wifi
        esp_http_server
        esp_http_client
        esp_https_ota
        json
        usb
        fatfs
        app_update
)
