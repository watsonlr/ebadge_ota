# eBadge Apps

This directory contains standalone applications that can be loaded via the OTA loader or flashed directly to the ESP32-S3.

## Available Apps

### � Game Launcher
**Location**: `Apps/game_launcher/`

A menu system that displays all available games and lets you select which one to play.

**Controls**:
- Up/Down: Navigate menu
- Button A: Launch selected game
- Button B: Options (future)

**Build & Flash**:
```bash
cd Apps/game_launcher
./build.sh
./flash.sh
```

**Features**:
- Visual menu with game icons
- Scrollable game list
- Game descriptions
- Theme colors per game
- OTA-ready framework
- Supports up to 10 games

**Included Games**:
- Pac-Man
- Tetris
- Frogger

See [game_launcher/README.md](game_launcher/README.md) for detailed information.

---

### �🎮 Pac-Man Game
**Location**: `Apps/pacman/`

Classic Pac-Man game with ghost AI, power pellets, and score tracking.

**Controls**:
- D-pad: Move Pac-Man
- Button A: Pause/Resume
- Button B: Restart game

**Build & Flash**:
```bash
cd Apps/pacman
./build.sh
./flash.sh
```

**Features**:
- 15x19 tile maze
- 4 ghosts with AI (chase/scatter/frightened modes)
- Power pellets for eating ghosts
- Lives system (3 lives)
- Score tracking
- Level progression

See [pacman/README.md](pacman/README.md) for detailed information.

---

### 🧱 Tetris Game
**Location**: `Apps/tetris/`

Classic Tetris with all 7 tetrominos, rotation, and line clearing.

**Controls**:
- Left/Right: Move piece
- Up: Rotate clockwise
- Down: Soft drop
- Button A: Hard drop
- Button B: Pause/Restart

**Build & Flash**:
```bash
cd Apps/tetris
./build.sh
./flash.sh
```

**Features**:
- 7 standard tetrominos (I, O, T, S, Z, J, L)
- Full rotation system
- Line clearing (1-4 lines at once)
- Progressive difficulty (speed increases)
- Next piece preview
- Scoring system with combos
- Level progression every 10 lines

See [tetris/README.md](tetris/README.md) for detailed information.

---

### 🐸 Frogger Game
**Location**: `Apps/frogger/`

Classic Frogger arcade game - guide the frog across roads and rivers!

**Controls**:
- D-pad: Move frog (up/down/left/right)
- Button A: Start next level
- Button B: Restart game

**Build & Flash**:
```bash
cd Apps/frogger
./build.sh
./flash.sh
```

**Features**:
- Multiple lanes (roads with traffic, rivers with platforms)
- Dynamic objects (cars, trucks, logs, turtles)
- 5 goal spots to reach
- Time limit (60 seconds per level)
- Lives system (3 lives)
- Progressive difficulty
- Score tracking

See [frogger/README.md](frogger/README.md) for detailed information.

## Creating New Apps

Each app should be a standalone ESP-IDF project:

```
Apps/
└── your_app/
    ├── CMakeLists.txt
    ├── sdkconfig.defaults
    ├── README.md
    ├── build.sh
    ├── flash.sh
    └── main/
        ├── CMakeLists.txt
        ├── your_app_main.c
        └── ... (other source files)
```

### Build Scripts Template

**build.sh**:
```bash
#!/bin/bash
source ~/esp/esp-idf/export.sh
idf.py build
```

**flash.sh**:
```bash
#!/bin/bash
source ~/esp/esp-idf/export.sh
idf.py -p ${ESPPORT:-/dev/ttyS10} flash monitor
```

## Hardware Reference

**ESP32-S3 e-Badge Hardware:**
- **Display**: ILI9341 240x320 LCD (SPI)
- **Buttons**: 6 buttons (Up, Down, Left, Right, A, B)
- **LEDs**: RGB LED + addressable LEDs
- **Storage**: SD card, 8MB flash
- **Sensors**: Accelerometer, joystick
- **Audio**: Piezo buzzer

See main project [HARDWARE.md](../HARDWARE.md) for complete pinout.

## Adding Apps to OTA Manifest

To make your app available via OTA updates:

1. **Build the app**:
   ```bash
   cd Apps/your_app
   idf.py build
   ```

2. **Copy binary to OTA directory**:
   ```bash
   cp build/your_app.bin ../../ota_files/apps/
   ```

3. **Update manifest.json**:
   ```json
   {
     "apps": [
       {
         "name": "Your App Name",
         "version": "1.0.0",
         "url": "http://YOUR_SERVER:8080/apps/your_app.bin"
       }
     ]
   }
   ```

4. **Start OTA server**:
   ```bash
   cd ../..
   python3 simple_ota_server.py
   ```

## App Ideas

Here are some ideas for future apps:

- 🎯 **Snake Game**: Classic snake with joystick control
- 🎨 **Drawing App**: Use joystick to draw on screen
- 📊 **System Monitor**: Display WiFi signal, battery, etc.
- 🎵 **Music Player**: Play tones through buzzer
- 🧩 **Puzzle Games**: Tetris, 2048, etc.
- 📡 **Network Tools**: WiFi scanner, ping utility
- 🎲 **Dice Roller**: For gaming, uses accelerometer
- ⏱️ **Timer/Stopwatch**: Precise timing utility
- 🌡️ **Sensor Dashboard**: Show accelerometer data
- 💾 **File Browser**: Browse SD card files

## License

All apps inherit the MIT License from the main project unless otherwise specified.

Enjoy creating apps for your e-Badge! 🚀
