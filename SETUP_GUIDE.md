# Setup Guide (Unified Environment)

## Choose ONE Development Environment (Recommended: WSL Ubuntu)

To eliminate PowerShell vs. WSL conflicts, this project standardizes on a single environment. Use WSL (Ubuntu) for all commands below. Do NOT mix PowerShell and WSL for build/flash/monitor steps.

This project intentionally omits native Windows/PowerShell command examples to avoid accidental mixing. All commands assume a WSL (Ubuntu) bash shell. If you previously used PowerShell, close those terminals to prevent stray carriage returns in scripts.

### Why WSL?
WSL gives a Linux‑like workflow, consistent line endings (LF), proper shebang handling, and easier scripting. A `.gitattributes` file enforces LF endings to prevent the `python\r` monitor error.

## 1. Prerequisites (WSL Flow)

Inside WSL Ubuntu terminal:
- Git
- Python 3.11+
- `pip`
- `cmake`, `ninja-build`, `flex`, `bison`, `gperf`, `ccache`
- USB access (configure /dev/ttyS* or passthrough for native USB devices)

Install dependencies:
```bash
sudo apt update
sudo apt install -y git wget flex bison gperf python3 python3-pip cmake ninja-build ccache libffi-dev libssl-dev dfu-util
```

## 2. Install ESP-IDF in WSL
```bash
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git -b v5.3.1
cd esp-idf
./install.sh esp32c3
```

Add to shell (optional):
```bash
echo 'source $HOME/esp/esp-idf/export.sh' >> ~/.bashrc
source ~/.bashrc
```

Quick manual activation helper (add to `~/.bashrc` for auto-detection):
```bash
if [ -d "$HOME/esp/esp-idf" ] && [ -f "$HOME/esp/esp-idf/export.sh" ]; then
  source "$HOME/esp/esp-idf/export.sh" >/dev/null 2>&1
fi
```

Manual activation for a fresh shell:
```bash
source ~/esp/esp-idf/export.sh
```

Confirm:
```bash
idf.py --version
```

## 3. Clone Project (Inside WSL)
```bash
cd ~/projects
git clone https://github.com/watsonlr/ebadge_ota.git
cd ebadge_ota
```

Ensure line endings are LF:
```bash
git config core.autocrlf false
git reset --hard
```

## 4. Serial Port Access (WSL Only)
Preferred: attach the USB device directly to WSL so it appears as `/dev/ttyUSB0` or `/dev/ttyACM0` (better stability than the `/dev/ttyS*` proxy).

If it currently appears as a Windows COM port (e.g., COM10), you can still access via `/dev/ttyS10`, but for optimal reliability:
1. Install usbipd-win on Windows (if not already).
2. In elevated PowerShell (once, not for build): `usbipd list` then `usbipd bind -b <BUSID>` and `usbipd attach --wsl --busid <BUSID>`.
3. Back in WSL: `ls /dev/ttyUSB* /dev/ttyACM*` and pick the new device.

Set environment convenience variable:

List ports:
```bash
ls /dev/ttyS* /dev/ttyUSB* 2>/dev/null
```

Set an environment variable for convenience:
```bash
export ESPPORT=/dev/ttyUSB0  # or /dev/ttyACM0 /dev/ttyS10 as fallback
```

## 5. First Build (Provisioning‑Only Loader)
```bash
idf.py set-target esp32c3
idf.py build
```

Outputs: bootloader, partition table, factory app.

## 6. Flash
```bash
idf.py -p "$ESPPORT" erase-flash
idf.py -p "$ESPPORT" flash
```

Subsequent flashes:
```bash
idf.py -p "$ESPPORT" flash
```

## 7. Monitor
```bash
idf.py -p "$ESPPORT" monitor
```
Exit with `Ctrl+]`.

Helper script (auto-uses `$ESPPORT` if set):
```bash
./scripts/monitor.sh    # defaults to /dev/ttyS10
./scripts/monitor.sh /dev/ttyUSB0
```

If you see `/usr/bin/env: 'python\r': No such file or directory`, run:
```bash
grep -Il $'\r' $(git ls-files '*.py' '*.sh') | xargs -r dos2unix
```
and ensure you are inside WSL (not PowerShell).

## 8. Provisioning Workflow
On boot the device starts a SoftAP (provisioning-first design):
- SSID: `BYUI_NameBadge`
- Open network

Steps:
1. Connect laptop/phone to `BYUI_NameBadge`
2. Browse to `http://192.168.4.1/`
3. Click "Scan Networks" → dropdown fills
4. Select SSID, enter password (if secured)
5. Click Save
6. Device switches to station mode and connects
7. Console logs:
   - Association message
   - IP acquisition: `Joined SSID '<ssid>' and got IP: x.x.x.x`

## 9. OTA Test App Build (Optional)

After provisioning success and loader menu appears:
```bash
cd ..
cp -r $IDF_PATH/examples/get-started/blink my_test_app
cd my_test_app
cp ../ebadge_ota/partitions.csv .
idf.py set-target esp32c3 build
```

Copy artifact to server:
```bash
cp build/*.bin /path/to/ota_files/apps/
```

Update `manifest.json` accordingly.

## 10. Return to Loader
Factory partition remains intact; reset to return. From an app you can force loader:
```c
const esp_partition_t *factory = esp_partition_find_first(
  ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
esp_ota_set_boot_partition(factory);
esp_restart();
```

## Troubleshooting (Unified)

### Serial Monitor Fails
- Ensure correct port: `echo $ESPPORT`
- Line endings: run `dos2unix` on scripts if needed
- Try lower baud: `idf.py -p "$ESPPORT" -b 115200 monitor`

### SoftAP Not Visible
- Check log for `SoftAP started successfully`
- Ensure no prior STA init before provisioning
- Reflash after clean build: `idf.py fullclean build flash`

### Scan Returns Empty
- Move closer to router
- Ensure running in APSTA mode

### Cannot Connect After Provisioning
- Wrong password → re-enter via portal (power cycle)
- Hidden SSID? Enable broadcast during provisioning

### `python\r` Error
Cause: File created/edited in Windows editor with CRLF line endings, then executed in WSL.
Fix:
```bash
git config core.autocrlf false
grep -Il $'\r' $(git ls-files '*.py' '*.sh') | xargs -r dos2unix
```
Prevent: Only edit inside VS Code WSL remote; keep `.gitattributes` (already present).

## VS Code (WSL)
Open folder inside WSL using Remote – WSL extension. Use integrated terminal (bash) only.

Commands palette:
- Build: `ESP-IDF: Build`
- Flash: `ESP-IDF: Flash`
- Monitor: `ESP-IDF: Monitor`

## Next Steps
1. Add HTTPS support
2. Implement USB recovery fully for C3
3. Add version comparison before OTA
4. Extend provisioning to validate password
5. Add captive portal redirect

## Getting Help
- ESP-IDF Programming Guide: https://docs.espressif.com/projects/esp-idf/
- ESP32-C3 Technical Reference: https://www.espressif.com/
- Forums: https://esp32.com/

