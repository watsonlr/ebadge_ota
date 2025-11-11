# ESP32-S3 OTA Loader System

A sophisticated OTA (Over-The-Air) bootloader system for ESP32-S3 that allows downloading and running multiple applications dynamically.

## Features

- **Permanent Factory Loader**: Never-overwritten supervisor firmware
- **Multiple App Slots**: Two OTA partitions for downloadable apps (ota_0, ota_1)
- **Web-based Manifest**: Fetch available apps from HTTP(S) server
- **USB Recovery Mode**: Reflash bootloader from USB flash drive (ESP32-S3 only)
- **Wi-Fi Management**: Automatic connection and reconnection
- **Safe OTA Updates**: Proper verification and rollback support

## System Architecture

### Partition Layout

```
┌─────────────────────────────────────────────┐
│ NVS (24KB)           - Non-volatile storage │
│ OTA Data (8KB)       - Boot partition info  │
│ Factory (1MB)        - This loader (permanent)
│ OTA_0 (1MB)          - Downloadable app slot 0
│ OTA_1 (1MB)          - Downloadable app slot 1
│ SPIFFS (1MB)         - File storage         │
└─────────────────────────────────────────────┘
```

### Boot Flow

```
Power On
   ↓
ROM Bootloader (in silicon)
   ↓
ESP-IDF Bootloader (0x1000) ← Can be recovered via USB
   ↓
Factory Loader (this app) OR Selected OTA App
   ↓
[Loader Menu] → Download/Install Apps
```

## Prerequisites

- **Hardware**: ESP32-S3 development board with USB-OTG
- **Software**: ESP-IDF v5.0 or later
- **Tools**: VS Code with ESP-IDF extension

## Quick Start

### 1. Configure the Project

```bash
idf.py menuconfig
```

Navigate to `OTA Loader Configuration` and set:
- Wi-Fi SSID
- Wi-Fi Password
- Manifest URL (where your app manifest JSON is hosted)

### 2. Build and Flash

```bash
# Build the project
idf.py build

# Erase flash (first time only)
idf.py -p COM3 erase-flash

# Flash the loader
idf.py -p COM3 flash monitor
```

### 3. Set Up App Server

Create a web server hosting your manifest and app binaries:

**Directory structure:**
```
/var/www/ota/
├── manifest.json
└── apps/
    ├── led_controller.bin
    └── weather_station.bin
```

**manifest.json example:**
```json
{
  "apps": [
    {
      "name": "LED Controller",
      "version": "1.0.0",
      "url": "http://192.168.1.100:8080/apps/led_controller.bin"
    },
    {
      "name": "Weather Station",
      "version": "2.1.3",
      "url": "http://192.168.1.100:8080/apps/weather_station.bin"
    }
  ]
}
```

### 4. Use the Loader

Once booted, the loader provides a menu:

```
=== Main Menu ===
1. Fetch and display available apps
2. Download and install an app
3. Show partition information
4. Reboot
0. Return to factory (this loader)
```

## Creating Downloadable Apps

Your downloadable apps should be standard ESP-IDF projects with:

1. **Same partition table** (or at least compatible OTA partitions)
2. **Standard `app_main()`** entry point
3. Built normally with `idf.py build`

To make an app available for download:

```bash
# Build your app
cd my_app
idf.py build

# Copy the binary to your OTA server
cp build/my_app.bin /var/www/ota/apps/

# Update manifest.json to include it
```

## USB Recovery Mode

If the bootloader becomes corrupted:

1. Create a file named `bootloader.bin` on a USB flash drive
2. Connect USB drive to ESP32-S3
3. Hold the BOOT button while resetting
4. The loader will detect recovery mode and attempt to reflash

**Note:** Full USB MSC host implementation is marked as a TODO in `usb_recovery.c`

## File Structure

```
ebadge_ota/
├── CMakeLists.txt              # Top-level build config
├── partitions.csv              # Custom partition table
├── README.md                   # This file
└── main/
    ├── CMakeLists.txt          # Component build config
    ├── Kconfig.projbuild       # Configuration options
    ├── ota_loader_main.c       # Main application
    ├── wifi_manager.c/h        # Wi-Fi connectivity
    ├── ota_manager.c/h         # OTA download/install
    └── usb_recovery.c/h        # USB bootloader recovery
```

## Configuration Options

All configurable via `idf.py menuconfig` → `OTA Loader Configuration`:

| Option | Description | Default |
|--------|-------------|---------|
| `WIFI_SSID` | Wi-Fi network name | myssid |
| `WIFI_PASSWORD` | Wi-Fi password | mypassword |
| `WIFI_MAXIMUM_RETRY` | Max connection attempts | 5 |
| `MANIFEST_URL` | URL of app manifest | http://192.168.1.100:8080/manifest.json |

## API Documentation

### OTA Manager

```c
// Fetch available apps from server
esp_err_t ota_manager_fetch_manifest(const char *manifest_url, app_manifest_t *manifest);

// Download and install an app
esp_err_t ota_manager_download_and_install(const char *app_url);

// Display partition information
void ota_manager_print_partition_info(void);
```

### Wi-Fi Manager

```c
// Initialize Wi-Fi
esp_err_t wifi_manager_init(void);

// Wait for connection with timeout
esp_err_t wifi_manager_wait_connected(uint32_t timeout_ms);

// Check connection status
bool wifi_manager_is_connected(void);
```

### USB Recovery

```c
// Check if BOOT button is pressed
bool usb_recovery_check_trigger(void);

// Attempt bootloader recovery from USB
esp_err_t usb_recovery_reflash_bootloader(void);
```

## Safety Features

1. **Factory Protection**: Loader lives in factory partition, never overwritten by OTA
2. **Dual OTA Slots**: Two app partitions allow rollback
3. **Verification**: Image verification before activation
4. **Recovery Mode**: USB-based bootloader recovery
5. **Always Recoverable**: ROM bootloader allows UART flashing

## Troubleshooting

### Wi-Fi won't connect
- Check SSID/password in menuconfig
- Verify 2.4GHz network (ESP32 doesn't support 5GHz)
- Check router firewall settings

### OTA download fails
- Verify manifest URL is accessible from ESP32's network
- Check server logs
- Try HTTP instead of HTTPS (certificate issues)
- Increase timeout in `ota_manager.c`

### Bootloader corrupted
Option 1 (USB Recovery - when implemented):
- Create USB stick with `bootloader.bin`
- Hold BOOT and reset

Option 2 (Manual):
```bash
idf.py -p COM3 bootloader-flash
```

### App won't boot after OTA
The loader automatically stays in factory partition. To return manually:
- Select option "0. Return to factory" from menu

## Advanced Usage

### HTTPS Support

To use HTTPS for manifest and app downloads:

1. Add server certificate to project
2. Update `ota_manager.c`:
```c
extern const uint8_t server_cert_pem_start[] asm("_binary_server_cert_pem_start");
extern const uint8_t server_cert_pem_end[]   asm("_binary_server_cert_pem_end");

esp_http_client_config_t config = {
    .url = url,
    .cert_pem = (char *)server_cert_pem_start,
};
```

### Secure Boot & Flash Encryption

Compatible with ESP32 security features, but requires:
- Signed bootloader images
- Encrypted app binaries
- Proper key management

## Future Enhancements

- [ ] Complete USB MSC host implementation
- [ ] SHA256 verification of downloaded apps
- [ ] Web-based UI for app selection
- [ ] BLE/MQTT remote triggering
- [ ] App version checking and auto-update
- [ ] Compressed app downloads
- [ ] Multi-language support

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test thoroughly on ESP32-S3 hardware
4. Submit pull request

## Support

- ESP-IDF Documentation: https://docs.espressif.com/projects/esp-idf/
- ESP32-S3 Datasheet: https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf
- GitHub Issues: [Create an issue](https://github.com/yourusername/ebadge_ota/issues)
