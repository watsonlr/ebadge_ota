# Project Summary: ESP32-S3 OTA Bootloader System

## What We Built

A complete OTA (Over-The-Air) bootloader system for ESP32-S3 that acts as a permanent "app loader" allowing you to download and run multiple different programs on your ESP32 without needing to connect it to a computer each time.

## Key Concepts from the Chat

### 1. **Three-Layer Boot Architecture**
- **ROM Bootloader** (permanent in chip silicon)
- **ESP-IDF Bootloader** (`bootloader.bin` at flash offset 0x1000) - can be recovered
- **Application** (your loader or downloaded apps)

### 2. **Custom Partition Layout**
```
nvs (24KB)        - Stores Wi-Fi credentials, settings
otadata (8KB)     - Tracks which OTA partition to boot
loader (1MB)      - This permanent supervisor (factory partition)
app_0 (1MB)       - First downloadable app slot
app_1 (1MB)       - Second downloadable app slot
spiffs (1MB)      - File storage for certificates, configs
```

### 3. **How It Works**
1. ESP32 boots into the factory "loader" app
2. Loader connects to Wi-Fi
3. Fetches a JSON manifest from your web server listing available apps
4. You select an app to download
5. Loader downloads the .bin file via HTTP/HTTPS
6. Writes it to an OTA partition (ota_0 or ota_1)
7. Sets that partition as the boot partition
8. Reboots → ESP32 now runs your downloaded app
9. Can always return to the loader (factory partition)

## Files Created

### Core System Files
- **`partitions.csv`** - Defines flash memory layout
- **`CMakeLists.txt`** - Top-level build configuration
- **`sdkconfig.defaults`** - Default ESP32-S3 configuration

### Main Application (`main/`)
- **`ota_loader_main.c`** - Main loader app with menu system
- **`wifi_manager.c/h`** - Wi-Fi connection management
- **`ota_manager.c/h`** - Manifest fetching and OTA downloads
- **`usb_recovery.c/h`** - USB bootloader recovery (skeleton)
- **`Kconfig.projbuild`** - Configuration menu definitions
- **`CMakeLists.txt`** - Component build configuration

### Documentation
- **`README.md`** - Complete project documentation
- **`SETUP_GUIDE.md`** - Step-by-step build and usage instructions
- **`LICENSE`** - MIT license

### Development Tools
- **`simple_ota_server.py`** - Python development server for hosting apps
- **`manifest.json.example`** - Example manifest structure
- **`.gitignore`** - Git ignore patterns

## Main Features Implemented

### ✅ Completed
1. **Permanent Factory Loader** - Never gets overwritten by OTA
2. **Wi-Fi Management** - Automatic connection with retry logic
3. **Manifest System** - Fetch list of available apps from server
4. **OTA Downloads** - Full HTTPS OTA implementation
5. **Partition Management** - Safe switching between apps
6. **Serial Menu Interface** - Easy to use text-based UI
7. **Multi-app Support** - Two OTA slots for different apps

### 🚧 Partially Implemented (Skeleton/TODO)
1. **USB Recovery Mode** - Structure in place, needs full USB MSC host implementation
   - Detects BOOT button press
   - Would mount USB drive and reflash bootloader
   - Currently shows placeholder message

### 🔮 Future Enhancements (Mentioned in Chat)
- SHA256 verification of downloaded apps
- Signed image verification
- Compressed downloads
- Web UI instead of serial menu
- BLE/MQTT remote triggering
- Auto-update checking
- Rollback on failed boot

## Usage Flow

### Initial Setup
```bash
# Configure Wi-Fi and manifest URL
idf.py menuconfig

# Build and flash
idf.py build
idf.py -p COM3 erase-flash
idf.py -p COM3 flash monitor
```

### Using the Loader
```
=== Main Menu ===
1. Fetch and display available apps     ← See what's available
2. Download and install an app          ← Install an app
3. Show partition information           ← Debug info
4. Reboot                               ← Restart
0. Return to factory (this loader)      ← Go back to loader
```

### Creating Apps to Download
1. Build any ESP-IDF app with the same partition table
2. Copy `build/myapp.bin` to your web server
3. Update `manifest.json` with app info
4. Download via the loader menu

## Important Concepts Learned

### 1. **Factory Partition = Safety Net**
The factory partition (`loader`) is never touched by OTA updates, so you always have a way to recover.

### 2. **OTA Partitions = Switchable Apps**
Two OTA partitions (`ota_0`, `ota_1`) let you download different apps and switch between them.

### 3. **Boot Partition Selection**
```c
esp_ota_set_boot_partition(partition);  // Set which app runs next
esp_restart();                           // Reboot into it
```

### 4. **Bootloader Recovery**
- ESP32's ROM bootloader (in silicon) can ALWAYS enter UART download mode
- Press BOOT button → can reflash via USB no matter what
- Additional safety: USB flash drive recovery (when fully implemented)

### 5. **Manifest-Based Distribution**
Instead of hardcoding app URLs, use a JSON manifest:
```json
{
  "apps": [
    {
      "name": "My App",
      "version": "1.0.0",
      "url": "http://server/apps/myapp.bin"
    }
  ]
}
```

## Security Considerations

### Current Implementation
- HTTP downloads (easy for development)
- No signature verification
- No encryption

### Production Recommendations
1. Use HTTPS with certificate pinning
2. Sign binaries with RSA keys
3. Enable ESP32 Secure Boot
4. Enable Flash Encryption
5. Verify SHA256 hashes before flashing
6. Implement rollback protection

## Testing Your System

### Step 1: Start OTA Server
```bash
python3 simple_ota_server.py
# Note the IP address shown
```

### Step 2: Configure ESP32
```bash
idf.py menuconfig
# Set MANIFEST_URL to http://<server_ip>:8080/manifest.json
```

### Step 3: Create Test App
```bash
# Use any ESP-IDF example, e.g., blink
cp build/blink.bin ota_files/apps/
# Update ota_files/manifest.json
```

### Step 4: Download via Loader
- Select menu option 2
- Choose app number
- Watch it download and reboot

## Troubleshooting Quick Reference

| Problem | Solution |
|---------|----------|
| Wi-Fi won't connect | Check SSID/password in menuconfig |
| Can't fetch manifest | Verify URL and network connectivity |
| OTA download fails | Check .bin file exists on server |
| Bootloader corrupted | `idf.py bootloader-flash` or USB recovery |
| Wrong partition boots | Use menu option 0 to return to factory |

## Next Steps for Your Project

1. **Test the basic system** - Build, flash, and verify menu works
2. **Set up OTA server** - Use the Python script or your own server
3. **Create a test app** - Use blink example or your own code
4. **Test OTA download** - Verify you can download and run it
5. **Enhance USB recovery** - Fully implement USB MSC host (if needed)
6. **Add security** - Implement HTTPS, signing, verification
7. **Custom apps** - Build the actual applications you want to deploy

## Resources Used

- **ESP-IDF OTA API**: `esp_ota_ops.h`, `esp_https_ota.h`
- **HTTP Client**: `esp_http_client.h`
- **Wi-Fi**: `esp_wifi.h`, `esp_event.h`
- **JSON Parsing**: `cJSON.h`
- **Flash Operations**: `esp_flash.h`
- **Partition API**: `esp_partition.h`

## Why This Architecture?

Based on the chat discussion, this design gives you:

✅ **Flexibility** - Download different apps without USB cable  
✅ **Safety** - Factory loader can never be OTA-overwritten  
✅ **Recovery** - Multiple ways to recover (UART, USB, factory reset)  
✅ **Development Speed** - Quick iteration on apps without reflashing  
✅ **Production Ready** - Can be extended with security features  
✅ **Multi-App Support** - Run different firmware on same device  

This is essentially your own "App Store" for ESP32! 🎉
