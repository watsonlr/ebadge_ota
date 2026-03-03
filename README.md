# ESP32-S3 E-Badge with USB Recovery System

A failsafe bootloader system for ESP32-S3 educational badges that allows students to recover from bricked firmware via USB flash drive. Designed for classroom environments where students may upload code that crashes or corrupts flash memory.

## Project Overview

This project implements a dual-partition architecture with a permanent **Recovery Application** that enables firmware updates through **two complementary methods**:

1. **USB Flash Drive Recovery** (Emergency/Offline)
   - For bricked devices that won't boot
   - No network or programmer required
   - Hold Boot button + insert USB drive → auto-flash

2. **Wi-Fi OTA Updates** (Normal Operation)
   - For functional devices
   - Download apps from HTTP server catalog
   - Network-based centralized distribution

Both methods make the device virtually unbrickable without specialized tools, perfect for educational environments where students experiment with code.

## Features

### Dual Recovery/Update System

This project provides **two complementary methods** for updating firmware:

**1. USB Flash Drive Recovery** (Emergency/Offline)
- Reflash bricked devices from USB flash drive with single button press
- No network or programmer required
- Perfect for classroom when device won't boot
- FAT32 flash drive with `update.bin` file

**2. Wi-Fi OTA Updates** (Convenience/Normal Operation)
- Download apps from HTTP server when device is functional
- Centralized distribution from instructor's server
- Manifest-based app catalog
- Requires working Wi-Fi connection

### Additional Features

- **Permanent Failsafe**: Recovery app in factory partition cannot be accidentally erased
- **Student-Proof**: No UART/programmer needed to recover from bad code
- **Wi-Fi Provisioning**: Built-in SoftAP portal for easy network configuration
- **Educational Focus**: Designed for classroom deployment where students can experiment safely

## 🎮 Included Games & Apps

This project includes a collection of classic arcade games ready to run on your eBadge! All games feature full button controls, smooth 60 FPS gameplay, and beautiful LCD graphics.

### Game Launcher
A visual menu system that displays all available games with icons and descriptions. Navigate with the D-pad and launch games with button A.

### Available Games
- **Pac-Man** 🟡 - Classic maze chase with 4 ghosts, power pellets, and AI
- **Tetris** 🔷 - Stack blocks and clear lines with 7 tetrominos
- **Frogger** 🐸 - Cross roads and rivers in this arcade classic

### Building & Flashing Games

**Quick Build All Games:**
```bash
# Windows (ESP-IDF PowerShell/CMD)
build_all.bat

# Linux/macOS (with ESP-IDF sourced)
./build_all.sh
```

**Flash Game Launcher:**
```bash
cd Apps/game_launcher
idf.py -p COM3 flash  # Windows
idf.py -p /dev/ttyUSB0 flash  # Linux
```

See [BUILD_GUIDE.md](BUILD_GUIDE.md) for detailed instructions and [Apps/README.md](Apps/README.md) for game documentation.

## System Architecture

### The Two-Partition Strategy

Since the ESP32-S3 ROM bootloader cannot natively "pull" files from a USB drive, this project uses a small, permanent **Recovery Application** that sits in its own partition.

```
┌──────────────────────────────────────────────────────────┐
│ Partition A (Recovery - Factory)                         │
│ - Small "read-only" factory app                          │
│ - Contains USB Host + MSC drivers                        │
│ - Looks for update.bin on USB drive                      │
│ - Triggered by holding Boot button during power-on       │
└──────────────────────────────────────────────────────────┘
                          ↓ Flashes
┌──────────────────────────────────────────────────────────┐
│ Partition B (Student Application - OTA_0)                │
│ - Where the badge application code runs                  │
│ - Can be reflashed via USB recovery                      │
│ - Includes Wi-Fi provisioning system                     │
└──────────────────────────────────────────────────────────┘
```

### Partition Layout

```
Name          Type    SubType   Offset    Size       Purpose
────────────────────────────────────────────────────────────────
nvs           data    nvs       0x9000    0x4000     Wi-Fi credentials, settings
otadata       data    ota       0xd000    0x2000     OTA boot partition info
recovery      app     factory   0x10000   1M         USB recovery app (permanent)
student_app   app     ota_0     0x110000  2M         Student/badge application
spiffs        data    spiffs    0x310000  1M         File storage (optional)
```

### Boot Flow

```
┌─────────────┐
│  Power On   │
└──────┬──────┘
       │
       ▼
┌─────────────────┐
│ ROM Bootloader  │  (Built into ESP32-S3 silicon)
└──────┬──────────┘
       │
       ▼
┌──────────────────┐
│ ESP-IDF          │  (At 0x1000, can be recovered)
│ Bootloader       │
└──────┬───────────┘
       │
       ├─── Boot Button Held? ────► Recovery App (Factory)
       │                                    │
       │                                    ▼
       │                            ┌──────────────────┐
       │                            │ 1. Init USB Host │
       │                            │ 2. Mount USB     │
       │                            │ 3. Find update.bin│
       │                            │ 4. Flash to OTA_0│
       │                            │ 5. Reboot        │
       │                            └──────────────────┘
       │
       └─── Normal Boot ──────────► Student App (OTA_0)
                                            │
                                            ▼
                                    ┌──────────────────┐
                                    │ Badge application│
                                    │ with provisioning│
                                    └──────────────────┘
```

## Hardware Requirements

- **ESP32-S3-Mini-1-N8** (or compatible S3 module)
- **USB-OTG support**: GPIO19 (D-), GPIO20 (D+) connected to USB-A port
- **VBUS power**: USB port must provide 5V to flash drives
- **Boot button**: Connected to GPIO0 (standard ESP32 boot button)
- **Additional peripherals** (see [HARDWARE.md](HARDWARE.md)):
  - MMA8452Q accelerometer (I2C)
  - WS2813B addressable LEDs
  - TFT display (SPI)
  - Buttons, buzzer, SD card slot, etc.

## Technical Implementation

### 1. USB Host Initialization

The recovery app acts as a USB Host using ESP-IDF's USB Host Stack:

```c
#include "usb/usb_host.h"
#include "msc_host.h"

// Initialize USB Host
usb_host_config_t host_config = {
    .skip_phy_setup = false,
    .intr_flags = ESP_INTR_FLAG_LEVEL1,
};
usb_host_install(&host_config);

// Initialize MSC (Mass Storage Class)
msc_host_install(&msc_config);
```

**Hardware**: GPIO19 (D-) and GPIO20 (D+) must be connected to the USB-A port.

### 2. File System Handling

Mount the USB drive using FATFS and locate the update binary:

```c
// Mount USB drive as FATFS
fatfs_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 5,
};
esp_vfs_fat_usb_mount("/usb", &mount_config);

// Look for update.bin
FILE *f = fopen("/usb/update.bin", "rb");
if (f) {
    // File found, proceed with flashing
}
```

**Note**: USB drive must be formatted as **FAT32**. ExFAT and NTFS are not supported.

### 3. Flash Writing (Failsafe OTA)

Use `esp_ota_ops` to safely write the binary to the student partition:

```c
#include "esp_ota_ops.h"

// Find the target partition
const esp_partition_t *update_partition = 
    esp_partition_find_first(ESP_PARTITION_TYPE_APP, 
                            ESP_PARTITION_SUBTYPE_APP_OTA_0, 
                            NULL);

// Begin OTA update
esp_ota_handle_t update_handle = 0;
esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);

// Read from USB and write to flash in chunks
uint8_t buffer[4096];
while ((read_bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
    esp_ota_write(update_handle, buffer, read_bytes);
}

// Finalize and set as boot partition
esp_ota_end(update_handle);
esp_ota_set_boot_partition(update_partition);

// Reboot into new firmware
esp_restart();
```
## Classroom Deployment Strategy

This system is designed to be foolproof for student use:

### Initial Setup (Instructor)

1. Flash the recovery app to the factory partition
2. Flash the initial badge application to OTA_0
3. Distribute boards to students
4. **(Optional but recommended)** Set up HTTP server for Wi-Fi OTA:
   - Host manifest.json with app catalog
   - Host .bin files for students to download
   - Configure devices with manifest URL
   - Enables convenient app distribution without USB drives

### Student Workflow

**Normal Development:**
- Student develops and uploads code via ESP-IDF/Arduino
- Code runs from the OTA_0 partition
- Wi-Fi provisioning portal available for network setup
- Can download new apps from instructor's OTA server (if configured)

**When Code Bricks the Board (Emergency Recovery):**
1. Student prepares a USB flash drive (FAT32 formatted)
2. Student copies `update.bin` (working firmware) to the root of the drive
3. Student plugs USB drive into the badge's USB-A port
4. Student holds the **Boot button** and presses **Reset**
5. Recovery app automatically:
   - Detects USB drive
   - Finds `update.bin`
   - Flashes it to OTA_0 partition
   - Reboots into recovered firmware

**Alternative: Wi-Fi OTA Update (When Device Works):**
1. Student ensures device is connected to Wi-Fi (via provisioning portal)
2. Student accesses OTA menu on device
3. Device fetches manifest from instructor's server
4. Student selects desired app from list
5. Device downloads and installs automatically
6. Device reboots into new app

**No instructor intervention needed** - students can recover/update their own boards!

## Choosing the Right Update Method

### Use USB Recovery When:
- ❌ Device won't boot (bricked firmware)
- ❌ Device stuck in infinite loop/crash
- ❌ No Wi-Fi access available
- ❌ Need to reflash factory recovery app itself
- ✅ Offline classroom environment
- ✅ Student needs immediate fix without instructor help

**Process**: Hold Boot button → Insert USB drive → Reset → Auto-flash → Reboot

### Use Wi-Fi OTA When:
- ✅ Device boots and runs normally
- ✅ Want to try a different app from server catalog
- ✅ Instructor pushes updates to all students at once
- ✅ Device has network connectivity
- ✅ Need to verify/download latest version

**Process**: Connect to Wi-Fi → Browse manifest → Select app → Download → Install

### Comparison Matrix

| Aspect | USB Recovery | Wi-Fi OTA |
|--------|-------------|-----------|
| **Device State** | Bricked/non-functional | Working/functional |
| **Network Required** | ❌ No (offline) | ✅ Yes |
| **Speed** | Fast (~30 seconds) | Depends on network |
| **Infrastructure** | USB drive only | HTTP server needed |
| **Use Case** | Emergency recovery | Normal app updates |
| **Student Can Do Solo** | ✅ Yes | ✅ Yes (if server setup) |
| **Implementation Status** | Skeleton (needs USB MSC) | Mostly complete |

## Badge Application Features

The student/badge application (OTA_0 partition) includes:

### Wi-Fi Provisioning System

**Provisioning-First Design:**
- Device always boots into provisioning mode on first run
- Creates SoftAP: **BYUI_NameBadge** (channel 6, open network)
- Access web portal at: **http://192.168.4.1**
- Scan for networks and select from dropdown
- Credentials saved to NVS (persistent)
- Auto-connects to saved network on subsequent boots

**Web Portal Features:**
- Network scanning with signal strength
- Dropdown SSID selection
- Password input (disabled for open networks)
- Automatic connection after configuration
- IP address logging to console

### Hardware Integration

See [HARDWARE.md](HARDWARE.md) for complete GPIO pinout and component datasheets.

**Peripherals:**
- **Accelerometer**: MMA8452Q on I2C (GPIO21 SCL, GPIO47 SDA)
- **Display**: TFT LCD on SPI2 (GPIO9-13, GPIO48)
- **LEDs**: RGB (GPIO4-6) + WS2813B addressable (GPIO7)
- **Buttons**: D-pad (GPIO14-17) + A/B (GPIO38, GPIO18)
- **Joystick**: Analog X/Y (GPIO1-2, ADC1)
- **Buzzer**: PWM output (GPIO42)
- **SD Card**: SPI2 (GPIO3 CS, shared with display)
- **Battery**: TP4056 charging circuit for single-cell Li-ion

## Development Setup

### Prerequisites

- **ESP-IDF v5.3.1** or later
- **WSL Ubuntu** (recommended) or native Linux
- **ESP32-S3 hardware** with USB-OTG support
- **USB flash drive** (FAT32 formatted)

### Installation (WSL)

See [SETUP_GUIDE.md](SETUP_GUIDE.md) for detailed WSL setup instructions.

```bash
# Clone ESP-IDF
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.3.1
./install.sh esp32s3

# Source the environment
. ~/esp/esp-idf/export.sh
```

### Quick Start

```bash
# Clone this repository
cd ~/projects
git clone <repository-url> ebadge_ota
cd ebadge_ota

# Set target to ESP32-S3
idf.py set-target esp32s3

# Build the project
./scripts/build.sh

# Flash to device (adjust COM port)
export ESPPORT=/dev/ttyS10  # COM10 in Windows = /dev/ttyS10 in WSL
./scripts/flash.sh

# Monitor serial output
./scripts/monitor.sh
```

**Or use the all-in-one script:**
```bash
./scripts/flash_monitor.sh
```

### Using Helper Scripts

The project includes WSL-friendly bash scripts:

| Script | Purpose |
|--------|---------|
| `build.sh` | Auto-detects ESP-IDF and builds project |
| `flash.sh` | Flashes to device (uses $ESPPORT) |
| `monitor.sh` | Opens serial monitor |
| `flash_monitor.sh` | Flash and monitor in one command |

All scripts automatically detect ESP-IDF location (WSL or Windows mount).

## Project Structure

```
ebadge_ota/
├── CMakeLists.txt              # Top-level build configuration
├── partitions.csv              # Custom partition table
├── sdkconfig.defaults          # Default config (target: esp32s3)
├── README.md                   # This file
├── SETUP_GUIDE.md              # Detailed WSL setup instructions
├── QUICK_REFERENCE.md          # Command cheat sheet
├── HARDWARE.md                 # GPIO pinout and component specs
├── .gitattributes              # Enforce LF line endings
├── scripts/                    # Build automation scripts
│   ├── build.sh
│   ├── flash.sh
│   ├── monitor.sh
│   └── flash_monitor.sh
└── main/
    ├── CMakeLists.txt          # Component build config
    ├── ota_loader_main.c       # Main application entry point
    ├── wifi_manager.c/h        # Wi-Fi STA connection manager
    ├── provisioning.c/h        # SoftAP provisioning portal
    └── (future)
        ├── usb_recovery.c/h    # USB recovery implementation
        └── drivers/            # Peripheral drivers
```

## Creating the Recovery App

The recovery application is the factory partition code that handles USB flashing. 

**Key Requirements:**
1. Implement USB Host + MSC driver initialization
2. Mount USB drive as FATFS
3. Look for `update.bin` in root directory
4. Use `esp_ota_ops` to flash to OTA_0 partition
5. Small size (<1MB to fit in factory partition)

**Trigger Detection:**
```c
// Check if Boot button (GPIO0) is held during startup
gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);
gpio_set_pull_mode(GPIO_NUM_0, GPIO_PULLUP_ONLY);

if (gpio_get_level(GPIO_NUM_0) == 0) {
    // Boot button held - enter recovery mode
    usb_recovery_mode();
} else {
    // Normal boot - jump to student app
    esp_ota_set_boot_partition(ota_0_partition);
    esp_restart();
}
```

## Badge Application Development

Build standard ESP-IDF projects that compile to the OTA_0 partition. You can deploy them via either method:

### Method 1: USB Recovery (For Testing/Offline)

```bash
# Your student/badge application project
cd my_badge_app

# Use the same partition table
cp ../ebadge_ota/partitions.csv .

# Build
idf.py build

# Copy binary to USB drive for recovery mode
cp build/my_badge_app.bin /path/to/usb_drive/update.bin

# Flash via USB recovery:
# 1. Insert USB drive into badge
# 2. Hold Boot button
# 3. Press Reset
# 4. Release Boot button
# 5. Device auto-flashes and reboots
```

### Method 2: Wi-Fi OTA (For Server Distribution)

```bash
# Build your app
cd my_badge_app
idf.py build

# Copy to web server
cp build/my_badge_app.bin /var/www/ota/apps/my_badge_app.bin

# Update manifest.json on server
# Add entry with app name, version, and URL

# Students download via device menu:
# 1. Device connects to Wi-Fi (provisioning portal)
# 2. Access OTA menu on device
# 3. Fetch manifest
# 4. Select app
# 5. Download and install
```

### Setting Up An OTA Server (Optional)

For Wi-Fi OTA distribution, create a simple HTTP server:

```bash
# Create directory structure
mkdir -p /var/www/ota/apps

# Create manifest.json
cat > /var/www/ota/manifest.json << EOF
{
  "apps": [
    {
      "name": "Badge Game v1",
      "version": "1.0.0",
      "url": "http://192.168.1.100:8080/apps/badge_game.bin",
      "size": 524288,
      "description": "Simple badge game for students"
    },
    {
      "name": "LED Controller",
      "version": "2.1.0",
      "url": "http://192.168.1.100:8080/apps/led_controller.bin",
      "size": 450000,
      "description": "Control RGB and addressable LEDs"
    }
  ]
}
EOF

# Start simple HTTP server (Python)
cd /var/www/ota
python3 -m http.server 8080

# Or use nginx, Apache, etc.
```

Students configure the manifest URL in their device settings (or flash with `idf.py menuconfig`).

## Troubleshooting

### Provisioning Portal Not Appearing

**Symptoms**: Can't see "BYUI_NameBadge" Wi-Fi network

**Solutions:**
- Ensure device is in provisioning mode (check serial monitor)
- APSTA mode required for simultaneous AP and scanning
- Check that SoftAP initialization succeeded in logs
- Try restarting device
- Scan for 2.4GHz networks (ESP32 doesn't support 5GHz)

### USB Recovery Not Working

**Symptoms**: Recovery mode doesn't flash update.bin

**Solutions:**
- ⚠️ **Note**: USB recovery requires completing the USB Host MSC implementation in `usb_recovery.c`
- Verify USB drive is **FAT32** formatted (not exFAT or NTFS)
- Ensure file is named exactly `update.bin` in root directory
- Check that Boot button (GPIO0) is held during reset
- Verify USB-OTG pins (GPIO19/20) are properly connected
- Ensure USB port provides 5V power (VBUS)
- Check serial monitor for USB enumeration messages

### Wi-Fi OTA Not Working

**Symptoms**: Can't download apps from manifest server

**Solutions:**
- Verify device is connected to Wi-Fi (check provisioning)
- Test manifest URL is accessible from device's network
  - Try: `curl http://your-server:8080/manifest.json`
- Check manifest.json format is valid JSON
- Ensure app .bin files exist at URLs specified in manifest
- Verify HTTP server is running and accessible
- Try HTTP instead of HTTPS (certificate issues)
- Check firewall isn't blocking ESP32's requests
- Increase timeout in `ota_manager.c` if on slow network

### Build Errors

**python\r: command not found**
```bash
# Fix CRLF line endings in scripts
dos2unix scripts/*.sh
# Or use sed
sed -i 's/\r$//' scripts/*.sh
```

**ESP-IDF not found**
```bash
# Source the environment
. ~/esp/esp-idf/export.sh
# Or use Windows installation fallback (scripts detect automatically)
```

**Target mismatch**
```bash
# Set correct target
idf.py set-target esp32s3
idf.py fullclean
idf.py build
```

### Flashing Issues

**Serial port not found**
```bash
# Check Windows COM port
# COM3 → /dev/ttyS3, COM10 → /dev/ttyS10
export ESPPORT=/dev/ttyS10

# Or use USB passthrough (better)
# See SETUP_GUIDE.md for usbipd setup
export ESPPORT=/dev/ttyUSB0
```

**Permission denied**
```bash
# Add user to dialout group (Linux)
sudo usermod -a -G dialout $USER
# Logout and login

# Or run with sudo (not recommended)
sudo ./scripts/flash.sh
```

### Runtime Issues

**Device won't connect to saved Wi-Fi**
- Check credentials in NVS (use `idf.py monitor`)
- Verify network is 2.4GHz (not 5GHz)
- Try erasing NVS: `idf.py erase-nvs`
- Re-provision through SoftAP portal

**Peripheral not working**
- Verify GPIO assignments match HARDWARE.md
- Check for pin conflicts (SPI2 shared between display/SD)
- Ensure proper initialization order (I2C before accelerometer, etc.)
- Check power supply (some peripherals need 5V)

## Key Limitations

### USB Recovery Constraints

1. **Power**: ESP32-S3 must provide 5V to USB flash drive via VBUS
   - If powered by student's laptop USB, ensure VBUS on OTG port is powered
   - May require external power supply or powered USB hub

2. **File System**: USB drive **must** be FAT32
   - ExFAT not supported by ESP-IDF FATFS
   - NTFS not supported
   - Format: `mkfs.vfat -F 32 /dev/sdX`

3. **Partition Size**: Update binary must fit in OTA_0 partition (2MB limit)
   - Keep student apps under 2MB
   - Use `idf.py size` to check binary size
   - Optimize with compiler flags if needed

4. **Boot Button**: Must be accessible on device
   - GPIO0 typically has pull-up resistor
   - Active LOW (press = LOW)
   - Standard ESP32 development boards have this button

## Safety and Reliability

### What Makes This "Unbrickable"

1. **Factory Partition Protection**
   - Recovery app lives in `factory` partition
   - Student OTA operations can only write to OTA_0/OTA_1
   - Factory partition cannot be accidentally erased via software

2. **ROM Bootloader Fallback**
   - Even if factory partition is corrupted, ROM bootloader still works
   - Can always recover via UART with `idf.py flash`

3. **No Networking Required**
   - USB recovery works offline
   - No server setup or Wi-Fi needed for recovery
   - Perfect for classroom with limited network access

4. **Simple Student Interface**
   - Single button press + USB drive
   - No command-line tools required
   - Can recover in seconds

### Failure Modes and Recovery

| Failure | Recovery Method | Time | Network Required |
|---------|----------------|------|------------------|
| Student app crashed | USB recovery **or** Wi-Fi OTA | ~30s | No (USB) / Yes (OTA) |
| Student app infinite loop | USB recovery | ~30s | No |
| Flash corrupted | USB recovery | ~30s | No |
| Want to try different app | Wi-Fi OTA (preferred) | ~1min | Yes |
| Factory partition corrupted | UART flash via idf.py | ~2min | No |
| Bootloader corrupted | UART flash bootloader | ~1min | No |
| Hardware failure | Replace board | N/A | N/A |

**Recovery Method Selection:**
- **USB Recovery**: Best for offline/emergency situations, bricked devices
- **Wi-Fi OTA**: Best for normal operation, trying new apps, bulk updates
- **UART Flashing**: Last resort when both USB and factory partition fail

## Implementation Status

### ✅ Completed Features
- Wi-Fi provisioning system (SoftAP portal with network scanning)
- NVS credential storage and auto-connect
- OTA manager with HTTP client and manifest parsing
- Partition table and boot flow architecture
- WSL development environment with helper scripts
- Complete hardware documentation with GPIO pinout

### ⏳ Partially Implemented
- **USB Recovery**: Skeleton/template code exists in `usb_recovery.c`
  - Boot button detection: ✅ Complete
  - USB Host + MSC driver: ⚠️ Needs implementation
  - FATFS mounting: ⚠️ Needs implementation
  - Flash writing: ⚠️ Needs implementation
- **Wi-Fi OTA**: Core functionality exists in `ota_manager.c`
  - HTTP manifest fetching: ✅ Complete
  - JSON parsing: ✅ Complete
  - Binary download: ✅ Complete
  - Menu interface: ⏳ Basic implementation

### 🔜 Planned Features
- [ ] Complete USB Host MSC implementation for recovery mode
- [ ] SHA256 verification of downloaded binaries
- [ ] Display status on LCD during recovery/updates
- [ ] LED progress indicators
- [ ] Automatic partition expansion for 8MB flash
- [ ] Peripheral driver library (accelerometer, display, LEDs)
- [ ] Badge application menu system
- [ ] Multi-file USB support (select from multiple .bin files)
- [ ] Bluetooth-based updates from phone app
- [ ] Compressed firmware downloads
- [ ] Incremental/delta updates

## Additional Documentation

- **[SETUP_GUIDE.md](SETUP_GUIDE.md)**: Detailed WSL environment setup
- **[QUICK_REFERENCE.md](QUICK_REFERENCE.md)**: Command cheat sheet
- **[HARDWARE.md](HARDWARE.md)**: Complete GPIO pinout and component datasheets

## References

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)
- [ESP32-S3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [USB Host API Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/usb_host.html)
- [OTA Updates Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/system/ota.html)

## License

MIT License - See LICENSE file for details

## Contributing

Contributions welcome! Areas of interest:
- USB recovery implementation
- Peripheral drivers (accelerometer, display, LEDs)
- Badge application features (games, animations, etc.)
- Documentation improvements
- Testing on different ESP32-S3 boards

## Support

For issues specific to this project:
- Check existing documentation (SETUP_GUIDE.md, HARDWARE.md)
- Review troubleshooting section above
- Search closed issues on GitHub
- Open a new issue with:
  - ESP-IDF version
  - Hardware variant (ESP32-S3-Mini-1-N8, etc.)
  - Serial monitor output
  - Steps to reproduce
