# Pac-Man Game for ESP32-S3 e-Badge

A classic Pac-Man game implementation for the BYUI e-Badge V3.0 with ESP32-S3.

## Features

- **Classic Gameplay**: Navigate the maze, eat dots, and avoid ghosts
- **6-Button Controls**: D-pad for movement, A for pause, B for restart
- **Ghost AI**: 4 ghosts with chase and scatter behavior
- **Power Pellets**: Eat them to turn ghosts blue and gain the ability to eat them
- **Score Tracking**: Points for dots (10), power pellets (50), and ghosts (200)
- **Lives System**: Start with 3 lives
- **Level Progression**: Complete the maze to advance to the next level

## Hardware Requirements

- **ESP32-S3**: BYUI e-Badge V3.0 or compatible board
- **Display**: ILI9341 240x320 LCD (already on badge)
- **Buttons**: 6 buttons mapped to GPIOs:
  - Up: GPIO17
  - Down: GPIO16
  - Left: GPIO14
  - Right: GPIO15
  - A (Pause): GPIO38
  - B (Restart): GPIO18

## Controls

| Button | Function |
|--------|----------|
| ↑ Up | Move Pac-Man up |
| ↓ Down | Move Pac-Man down |
| ← Left | Move Pac-Man left |
| → Right | Move Pac-Man right |
| A | Pause/Unpause game |
| B | Restart game |

## Building the Project

### Prerequisites

- ESP-IDF v5.0 or later
- ESP32-S3 development board

### Build Steps

1. **Set up ESP-IDF environment**:
   ```bash
   cd ~/esp/esp-idf
   . ./export.sh
   ```

2. **Navigate to the project directory**:
   ```bash
   cd Apps/pacman
   ```

3. **Set the target** (if not already set):
   ```bash
   idf.py set-target esp32s3
   ```

4. **Build the project**:
   ```bash
   idf.py build
   ```

5. **Flash to device**:
   ```bash
   # Replace /dev/ttyUSB0 with your port
   idf.py -p /dev/ttyUSB0 flash
   ```

6. **Monitor output** (optional):
   ```bash
   idf.py -p /dev/ttyUSB0 monitor
   ```

   Or combine flash and monitor:
   ```bash
   idf.py -p /dev/ttyUSB0 flash monitor
   ```

### Quick Build Script

For convenience, you can create a `build.sh` script:

```bash
#!/bin/bash
. ~/esp/esp-idf/export.sh
idf.py build
```

And a `flash.sh` script:

```bash
#!/bin/bash
. ~/esp/esp-idf/export.sh
export ESPPORT=/dev/ttyUSB0  # Update with your port
idf.py flash monitor
```

## Generating OTA Binary

To create a binary for OTA updates:

```bash
idf.py build
cp build/pacman_game.bin ../../ota_files/apps/
```

Update the `manifest.json`:
```json
{
  "apps": [
    {
      "name": "Pac-Man Game",
      "version": "1.0.0",
      "url": "http://YOUR_SERVER_IP:8080/apps/pacman_game.bin"
    }
  ]
}
```

## Game Mechanics

### Scoring
- **Dot**: 10 points
- **Power Pellet**: 50 points
- **Ghost** (when blue): 200 points

### Power Mode
When Pac-Man eats a power pellet:
- Ghosts turn blue for ~10 seconds
- Pac-Man can eat ghosts
- Ghosts flee from Pac-Man

### Ghost Behavior
- **Red Ghost**: Aggressive chaser
- **Pink Ghost**: Ambush behavior
- **Cyan Ghost**: Patrol pattern
- **Orange Ghost**: Random movement

### Lives
- Start with 3 lives
- Lose a life when touched by a ghost (except when in power mode)
- Game over when all lives are lost

## Troubleshooting

### Display Issues
- Check SPI wiring matches pin definitions in `lcd_driver.c`
- Verify display reset and DC pins are correct

### Button Not Responding
- Buttons are active-low with internal pull-ups
- Check gpio_num matches your hardware
- Adjust `BUTTON_DEBOUNCE_MS` if needed

### Performance Issues
- Reduce ghost count for better FPS
- Adjust move timers in `update_pacman()` and `update_ghosts()`
- Lower SPI clock speed in `lcd_init()`

## File Structure

```
Apps/pacman/
├── CMakeLists.txt          # ESP-IDF project config
├── sdkconfig.defaults      # Default SDK configuration
├── README.md               # This file
└── main/
    ├── CMakeLists.txt      # Main component config
    ├── pacman_main.c       # Entry point
    ├── pacman_game.c       # Game logic
    ├── pacman_game.h       # Game header
    ├── lcd_driver.c        # LCD driver
    └── lcd_driver.h        # LCD header
```

## Future Enhancements

- [ ] Sound effects using buzzer (GPIO42)
- [ ] High score persistence using NVS
- [ ] Different maze layouts per level
- [ ] Fruit/bonus items
- [ ] Animations for ghost eyes and Pac-Man mouth
- [ ] Multiplayer mode

## License

MIT License - See main project LICENSE file

## Credits

Based on the classic Pac-Man arcade game by Namco (1980).
Implemented for educational purposes on the BYUI e-Badge platform.
