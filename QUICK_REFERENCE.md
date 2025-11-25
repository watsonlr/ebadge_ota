# Quick Reference Card

## 🚀 Quick Start Commands

```bash
# WSL bash: set up ESP-IDF environment (once per shell)
. "$HOME/esp/esp-idf/export.sh"

# Initial setup for ESP32-C3
idf.py set-target esp32c3
idf.py menuconfig                    # Configure Wi-Fi & manifest URL
idf.py build                         # Build the loader
idf.py -p /dev/ttyS10 erase-flash    # Erase (first time only); COM10 -> /dev/ttyS10
idf.py -p /dev/ttyS10 -b 115200 flash monitor  # Flash and monitor

# Start OTA server (WSL bash)
python3 simple_ota_server.py         # Serves on port 8080
```

## �️ Windows vs WSL Notes

| Scenario | Recommended Shell | Serial Port Arg | Notes |
|----------|-------------------|-----------------|-------|
| Normal build/flash/monitor | ESP-IDF Windows PowerShell/Command Prompt | `-p COM10` (example) | Easiest; avoids line-ending and device mapping issues |
| Using WSL (Linux bash) | WSL bash with a Linux-installed ESP-IDF | `-p /dev/ttyS10` (COM10→`/dev/ttyS10`) | Ensure scripts have LF endings; run `. $HOME/esp/esp-idf/export.sh` |
| Reusing Windows IDF inside WSL | Not recommended | `/dev/ttyS*` | Convert line endings with `dos2unix tools/idf.py` if `/usr/bin/env: 'python\r'` error appears |

Quick tips:
1. Don’t mix PowerShell syntax (`$env:VAR`, leading `&`) in bash; use `export VAR=value`.
2. Error `/usr/bin/env: 'python\r': No such file or directory` means the script has Windows (CRLF) line endings—convert with `dos2unix` or run via `python3 tools/idf.py`.
3. Exit `idf.py monitor` with `Ctrl+]` (or `Ctrl+T` then `Ctrl+X`).
4. Always run `idf.py set-target esp32c3` once after switching chips.
5. For just monitoring (already flashed): `idf.py -p COM10 -b 115200 monitor` (Windows) or `idf.py -p /dev/ttyS10 -b 115200 monitor` (WSL).


## �📂 File Structure

```
ebadge_ota/
├── partitions.csv              ← Custom partition table
├── CMakeLists.txt              ← Project build config
├── sdkconfig.defaults          ← Default ESP32-C3 settings
├── simple_ota_server.py        ← Development OTA server
├── manifest.json.example       ← Example manifest
├── README.md                   ← Full documentation
├── SETUP_GUIDE.md              ← Step-by-step instructions
├── PROJECT_SUMMARY.md          ← Chat digest & overview
├── ARCHITECTURE.md             ← Visual diagrams
└── main/
    ├── ota_loader_main.c       ← Main application
    ├── wifi_manager.c/h        ← Wi-Fi connectivity
    ├── ota_manager.c/h         ← OTA download/install
    ├── usb_recovery.c/h        ← USB bootloader recovery
    ├── CMakeLists.txt          ← Component config
    └── Kconfig.projbuild       ← Menu config options
```

## 🎯 Main Menu Options

```
=== Main Menu ===
1. Fetch and display available apps
2. Download and install an app
3. Show partition information
4. Reboot
0. Return to factory (this loader)
```

## 🔧 Configuration (menuconfig)

**Location:** `OTA Loader Configuration`

| Setting | Description | Default |
|---------|-------------|---------|
| WiFi SSID | Your Wi-Fi network | myssid |
| WiFi Password | Wi-Fi password | mypassword |
| Maximum Retry | Connection attempts | 5 |
| Manifest URL | JSON manifest location | http://192.168.1.100:8080/manifest.json |

## 📋 Manifest Format

```json
{
  "apps": [
    {
      "name": "App Name",
      "version": "1.0.0",
      "url": "http://server:8080/apps/app.bin"
    }
  ]
}
```

## 🔄 Partition Layout

| Offset | Size | Name | Type | Purpose |
|--------|------|------|------|---------|
| 0x1000 | ~40KB | bootloader | boot | ESP-IDF bootloader |
| 0x9000 | 24KB | nvs | data | Settings storage |
| 0xF000 | 8KB | otadata | data | OTA boot info |
| 0x10000 | 1MB | loader | factory | **Permanent loader** |
| 0x110000 | 1MB | app_0 | ota_0 | Downloadable slot 1 |
| 0x210000 | 1MB | app_1 | ota_1 | Downloadable slot 2 |
| 0x310000 | 1MB | spiffs | data | File storage |

## 🛠️ Creating Apps for OTA

1. Use same partition table:
   ```bash
   cp /path/to/ebadge_ota/partitions.csv .
   ```

2. Add to CMakeLists.txt:
   ```cmake
   set(PARTITION_CSV_PATH "${CMAKE_SOURCE_DIR}/partitions.csv")
   ```

3. Build and deploy:
   ```bash
   idf.py build
   cp build/myapp.bin /path/to/ota_files/apps/
   # Update manifest.json
   ```

## ⚠️ Troubleshooting

| Problem | Solution |
|---------|----------|
| Won't flash | Hold BOOT button, try `-b 115200` |
| No Wi-Fi | Check SSID/password, use 2.4GHz |
| Can't fetch manifest | Verify URL, ping server |
| OTA fails | Check .bin exists, verify URL |
| Bootloader corrupt | `idf.py bootloader-flash` |

## 🔐 Recovery Options

### Option 1: Factory Reset
- Menu option `0` → Returns to loader

### Option 2: Manual Bootloader Flash
```bash
idf.py -p /dev/ttyS10 bootloader-flash
```

### Option 3: Complete Reflash
```bash
idf.py -p /dev/ttyS10 erase-flash
idf.py -p /dev/ttyS10 flash
```

### Option 4: USB Recovery (when implemented)
1. Put `bootloader.bin` on USB stick
2. Hold BOOT button during reset
3. Loader reflashes bootloader

## 📡 API Quick Reference

### Wi-Fi Manager
```c
esp_err_t wifi_manager_init(void);
esp_err_t wifi_manager_wait_connected(uint32_t timeout_ms);
bool wifi_manager_is_connected(void);
```

### OTA Manager
```c
esp_err_t ota_manager_fetch_manifest(const char *url, app_manifest_t *manifest);
esp_err_t ota_manager_download_and_install(const char *app_url);
void ota_manager_display_apps(const app_manifest_t *manifest);
void ota_manager_print_partition_info(void);
```

### USB Recovery
```c
bool usb_recovery_check_trigger(void);
esp_err_t usb_recovery_reflash_bootloader(void);
```

## 🎓 Key Concepts

**Factory Partition**
- Permanent loader, never overwritten by OTA
- Always available for recovery

**OTA Partitions** 
- Two slots: ota_0 and ota_1
- Download apps here via HTTP/HTTPS
- Switchable at runtime

**Boot Selection**
```c
esp_ota_set_boot_partition(partition);  // Choose next boot
esp_restart();                           // Reboot into it
```

**Return to Loader**
```c
const esp_partition_t *factory = esp_partition_find_first(
    ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
esp_ota_set_boot_partition(factory);
esp_restart();
```

## 📊 Typical Workflow

1. **Initial Setup** (once)
   - Build loader → Flash via USB → Configure Wi-Fi

2. **Start OTA Server**
   - Run `simple_ota_server.py`
   - Note IP address

3. **Create App**
   - Build ESP-IDF project
   - Copy .bin to server
   - Update manifest

4. **Deploy**
   - Use loader menu option 2
   - Select app → Download → Reboot

5. **Iterate**
   - Modify app → Rebuild → Copy .bin
   - Download again (no USB needed!)

## 🔗 Important Links

- [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [ESP-IDF OTA Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/ota.html)

## 💡 Pro Tips

✅ Always keep a backup of `build/bootloader/bootloader.bin`  
✅ Test manifest URL in browser before using  
✅ Use HTTPS in production with certificate pinning  
✅ Implement SHA256 verification for security  
✅ Keep loader app small and simple  
✅ Use version checking to prevent downgrades  
✅ Monitor serial output during OTA updates  

## 📦 What's Included

✅ Complete OTA loader system  
✅ Wi-Fi management  
✅ Manifest-based app distribution  
✅ Dual OTA partitions  
✅ Factory partition protection  
✅ USB recovery mode (skeleton)  
✅ Development OTA server  
✅ Example manifest  
✅ Complete documentation  

## 🚧 TODO / Future Enhancements

- [ ] Complete USB MSC host implementation
- [ ] SHA256 verification of downloads
- [ ] HTTPS with certificate support
- [ ] Web UI for app selection
- [ ] Automatic version checking
- [ ] Rollback on boot failure
- [ ] Compressed downloads
- [ ] Progress bar for downloads
- [ ] BLE/MQTT remote triggering
