# Quick Setup Guide

## Step-by-Step Build Instructions

### 1. Prerequisites

Make sure you have:
- ESP-IDF v5.0 or later installed
- VS Code with ESP-IDF extension
- ESP32-S3 development board
- USB cable

### 2. Configure Wi-Fi

```bash
idf.py menuconfig
```

Navigate to: **OTA Loader Configuration**

Set:
- **WiFi SSID**: Your Wi-Fi network name
- **WiFi Password**: Your Wi-Fi password
- **Manifest URL**: `http://<your_ip>:8080/manifest.json`

Save and exit (press 'S', then 'Q')

### 3. Build the Project

```bash
idf.py build
```

This will:
- Compile all source files
- Generate bootloader
- Create partition table
- Link final binary

### 4. Flash to ESP32-S3

First time only (erases everything):
```bash
idf.py -p COM3 erase-flash
idf.py -p COM3 flash
```

After that, just:
```bash
idf.py -p COM3 flash
```

Replace `COM3` with your actual port (check Device Manager on Windows, or `/dev/ttyUSB0` on Linux)

### 5. Monitor Serial Output

```bash
idf.py -p COM3 monitor
```

Or combine flash + monitor:
```bash
idf.py -p COM3 flash monitor
```

Exit monitor with: `Ctrl + ]`

### 6. Set Up OTA Server (Development)

#### Option A: Simple Python Server (Recommended for Testing)

```bash
# Install Python 3 if needed
python3 simple_ota_server.py
```

This creates an `ota_files/` directory with example structure.

#### Option B: Use Your Own Web Server

Create directory structure:
```
/var/www/ota/
├── manifest.json
└── apps/
    ├── app1.bin
    └── app2.bin
```

Host on Apache/nginx/etc.

### 7. Create a Test App to Download

Create a new ESP-IDF project for your app:

```bash
cd ..
cp -r $IDF_PATH/examples/get-started/blink my_test_app
cd my_test_app
```

Edit the partition table to use the same OTA partitions:
```bash
cp ../ebadge_ota/partitions.csv .
```

Edit `CMakeLists.txt` to add partition table:
```cmake
set(PARTITION_CSV_PATH "${CMAKE_SOURCE_DIR}/partitions.csv")
```

Build it:
```bash
idf.py build
```

Copy the binary to your OTA server:
```bash
cp build/my_test_app.bin /path/to/ota_files/apps/
```

Update `manifest.json`:
```json
{
  "apps": [
    {
      "name": "Blink Test",
      "version": "1.0.0",
      "url": "http://192.168.1.100:8080/apps/my_test_app.bin"
    }
  ]
}
```

### 8. Test the OTA System

1. Boot the ESP32-S3 with the loader
2. From the menu, select **1** (Fetch apps)
3. Verify your app appears
4. Select **2** (Download and install)
5. Enter the app number
6. Wait for download and automatic reboot
7. Your app should now be running!

### 9. Return to Loader

To get back to the loader:
- Reset the device
- From any OTA app, you can also add code to jump back:
  ```c
  const esp_partition_t *factory = esp_partition_find_first(
      ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
  esp_ota_set_boot_partition(factory);
  esp_restart();
  ```

## Troubleshooting

### Build Errors

**Error: `esp_wifi.h: No such file`**
- Solution: Run `idf.py reconfigure`

**Error: `json: No such component`**
- Solution: Make sure you're using ESP-IDF v4.4+
- Run: `idf.py reconfigure`

### Flash Errors

**Error: `Failed to connect to ESP32`**
- Hold BOOT button while connecting
- Try different USB cable
- Check drivers (CP210x or CH340)

**Error: `A fatal error occurred: Packet content transfer stopped`**
- Lower baud rate: `idf.py -p COM3 -b 115200 flash`
- Try different USB port

### Runtime Errors

**Wi-Fi won't connect**
- Double-check SSID and password
- Ensure 2.4 GHz network (not 5 GHz)
- Check router is on and accessible

**Can't fetch manifest**
- Verify manifest URL is correct
- Check ESP32 and server are on same network
- Test URL in browser first
- Check firewall settings

**OTA download fails**
- Ensure .bin file exists on server
- Check file permissions (should be readable)
- Verify URL in manifest is correct
- Check server logs

## VS Code Integration

### Recommended Extensions
- ESP-IDF
- C/C++
- CMake Tools

### Build in VS Code
1. Press `F1`
2. Type: `ESP-IDF: Build`
3. Or use the build button in status bar

### Flash in VS Code
1. Press `F1`
2. Type: `ESP-IDF: Flash`
3. Or use the flash button in status bar

### Monitor in VS Code
1. Press `F1`
2. Type: `ESP-IDF: Monitor`
3. Or use the monitor button in status bar

## Next Steps

1. Create your own apps
2. Set up HTTPS with certificates
3. Implement USB recovery fully
4. Add version checking
5. Create a web UI for app selection

## Getting Help

- ESP-IDF Programming Guide: https://docs.espressif.com/projects/esp-idf/
- ESP32-S3 Technical Reference: https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
- ESP32 Forums: https://esp32.com/
