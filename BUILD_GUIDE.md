# Building and Deploying eBadge Apps

This guide explains how to build all games and deploy them for OTA updates.

## Prerequisites

- ESP-IDF v5.0 or later installed
- ESP-IDF environment activated
- Python 3.x installed

## Quick Build - All Apps

### Windows (PowerShell/CMD with ESP-IDF)

```batch
build_all.bat
```

### Linux/macOS/WSL

```bash
# First, source ESP-IDF environment
source $HOME/esp/esp-idf/export.sh

# Then build all apps
chmod +x build_all.sh
./build_all.sh
```

## Manual Build - Individual Apps

### Build Pac-Man
```bash
cd Apps/pacman
idf.py build
```

### Build Tetris
```bash
cd Apps/tetris
idf.py build
```

### Build Frogger
```bash
cd Apps/frogger
idf.py build
```

### Build Game Launcher
```bash
cd Apps/game_launcher
idf.py build
```

## Flash Game Launcher to Device

The game launcher should be flashed to your eBadge as the main application:

```bash
cd Apps/game_launcher
idf.py -p COMX flash monitor  # Windows
# or
idf.py -p /dev/ttyUSB0 flash monitor  # Linux
```

Where `COMX` is your device's COM port (check Device Manager on Windows).

## OTA Deployment

After building, all game binaries are copied to `ota_files/apps/`:
- `pacman.bin` - Pac-Man game
- `tetris.bin` - Tetris game
- `frogger.bin` - Frogger game
- `launcher.bin` - Game launcher

### Start OTA Server

```bash
python simple_ota_server.py
```

This starts an HTTP server on port 8080 serving the game binaries.

### Update Manifest

Edit `ota_files/manifest.json` and update:
1. `server` URL to match your server IP
2. `size` fields with actual binary sizes (optional)

### Connect eBadge to Server

The game launcher includes OTA update functionality (framework ready, full implementation pending). The `manifest.json` file lists all available games.

## Binary Sizes (Approximate)

- Pac-Man: ~150-200 KB
- Tetris: ~100-150 KB
- Frogger: ~100-150 KB
- Launcher: ~80-120 KB

## Build Output Locations

After building, binaries are located at:
```
Apps/pacman/build/pacman_game.bin
Apps/tetris/build/tetris_game.bin
Apps/frogger/build/frogger_game.bin
Apps/game_launcher/build/game_launcher.bin
```

## Troubleshooting

### ESP-IDF not found

**Windows**: Run from "ESP-IDF PowerShell" or "ESP-IDF CMD" shortcut

**Linux/macOS**:
```bash
source $HOME/esp/esp-idf/export.sh
```

Or if ESP-IDF is elsewhere:
```bash
source /path/to/esp-idf/export.sh
```

### Build errors

1. Clean build directory:
   ```bash
   idf.py fullclean
   idf.py build
   ```

2. Check ESP-IDF version:
   ```bash
   idf.py --version
   ```
   Should be v5.0 or later

3. Verify target:
   ```bash
   idf.py set-target esp32s3
   ```

### Flash errors

1. Check device connection:
   ```bash
   # Windows
   mode
   
   # Linux
   ls /dev/tty*
   ```

2. Try different baud rate:
   ```bash
   idf.py -p COM3 -b 115200 flash
   ```

3. Hold BOOT button while flashing if auto-reset fails

## Next Steps

1. **Flash Game Launcher** - This becomes your main menu
2. **Start OTA Server** - For wireless game loading
3. **Play Games** - Use the launcher to select games
4. **Add More Games** - Follow the app template in `Apps/README.md`

## Directory Structure

```
ebadge_ota/
├── build_all.bat          # Windows build script
├── build_all.sh           # Linux/Mac build script
├── BUILD_GUIDE.md         # This file
├── simple_ota_server.py   # OTA HTTP server
├── ota_files/
│   ├── manifest.json      # Game catalog
│   └── apps/
│       ├── launcher.bin   # Game launcher binary
│       ├── pacman.bin     # Pac-Man binary
│       ├── tetris.bin     # Tetris binary
│       └── frogger.bin    # Frogger binary
└── Apps/
    ├── game_launcher/     # Menu system
    ├── pacman/            # Pac-Man game
    ├── tetris/            # Tetris game
    └── frogger/           # Frogger game
```

## Development Workflow

1. **Develop** - Edit game code in `Apps/your_game/`
2. **Build** - Run `build_all.sh` or build individually
3. **Test** - Flash to device and test
4. **Deploy** - Copy to `ota_files/apps/` and start OTA server
5. **Update** - Device can download new version via OTA

Happy Gaming! 🎮
