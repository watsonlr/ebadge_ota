# Tetris Game for ESP32-S3 e-Badge

Classic Tetris game implementation for the BYUI e-Badge V3.0 with ESP32-S3.

## Features

- **Classic Tetris Gameplay**: 7 standard tetrominos (I, O, T, S, Z, J, L)
- **Full Rotation System**: Rotate pieces 90 degrees
- **Line Clearing**: Clear 1-4 lines at once for points
- **Progressive Difficulty**: Speed increases with level
- **Score System**: Points for lines cleared and drops
- **Level Progression**: Level up every 10 lines
- **Next Piece Preview**: See what's coming next
- **Soft Drop & Hard Drop**: Control piece descent speed

## Hardware Requirements

- **ESP32-S3**: BYUI e-Badge V3.0 or compatible board
- **Display**: ILI9341 240x320 LCD (already on badge)
- **Buttons**: 6 buttons mapped to GPIOs:
  - Up: GPIO17
  - Down: GPIO16
  - Left: GPIO14
  - Right: GPIO15
  - A: GPIO38
  - B: GPIO18

## Controls

| Button | Function |
|--------|----------|
| ↑ Up | Rotate piece clockwise |
| ↓ Down | Soft drop (move down faster, +1 point) |
| ← Left | Move piece left |
| → Right | Move piece right |
| A | Hard drop (instant drop, +2 points) |
| B | Pause/Restart |

## Game Mechanics

### Scoring
- **Single Line**: 100 points × level
- **Double Lines**: 300 points × level
- **Triple Lines**: 500 points × level
- **Tetris (4 lines)**: 800 points × level
- **Soft Drop**: +1 point per block
- **Hard Drop**: +2 points per block

### Levels
- Start at Level 1
- Level up every 10 lines cleared
- Drop speed increases with each level
- Minimum drop speed at higher levels

### Tetrominos

| Piece | Color | Shape |
|-------|-------|-------|
| I | Cyan | ████ |
| O | Yellow | ██<br>██ |
| T | Purple | ▀█▀ |
| S | Green | ▀██<br>██▀ |
| Z | Red | ██▀<br>▀██ |
| J | Blue | █<br>███ |
| L | Orange | ▀▀█<br>███ |

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
   cd Apps/tetris
   ```

3. **Set the target** (if not already set):
   ```bash
   idf.py set-target esp32s3
   ```

4. **Build the project**:
   ```bash
   ./build.sh
   ```

5. **Flash to device**:
   ```bash
   # Update ESPPORT if needed
   export ESPPORT=/dev/ttyUSB0
   ./flash.sh
   ```

### Quick Commands

```bash
# Build only
./build.sh

# Flash and monitor
./flash.sh

# Or manually
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Generating OTA Binary

To create a binary for OTA updates:

```bash
./build.sh
cp build/tetris_game.bin ../../ota_files/apps/
```

Update your `manifest.json`:
```json
{
  "apps": [
    {
      "name": "Tetris Game",
      "version": "1.0.0",
      "url": "http://YOUR_SERVER_IP:8080/apps/tetris_game.bin"
    }
  ]
}
```

## Tips & Strategy

### Basic Strategy
1. **Plan ahead**: Use the next piece preview
2. **Keep it flat**: Avoid creating tall columns
3. **Line up Tetrises**: Save the I-piece for 4-line clears
4. **Use hard drop wisely**: Quick placement but no take-backs

### Advanced Techniques
- **T-Spins**: Rotate T-pieces into tight spots
- **Combos**: Clear multiple lines in succession
- **Well Strategy**: Leave a column open for I-pieces
- **Flat Top**: Keep the playfield level

## Troubleshooting

### Display Issues
- Verify SPI wiring matches pin definitions
- Check display reset and DC pins
- Lower SPI clock speed if display glitches

### Button Not Responding
- Buttons are active-low with internal pull-ups
- Check GPIO assignments match hardware
- Adjust `BUTTON_DEBOUNCE_MS` if needed

### Game Too Fast/Slow
- Adjust `get_drop_interval()` function
- Modify level progression in `clear_lines()`
- Change FPS in main loop (currently 60 FPS)

### Performance Issues
- Reduce BLOCK_SIZE for faster rendering
- Optimize LCD driver SPI speed
- Simplify piece rendering

## File Structure

```
Apps/tetris/
├── CMakeLists.txt          # ESP-IDF project config
├── sdkconfig.defaults      # Default SDK configuration
├── README.md               # This file
├── build.sh                # Build script
├── flash.sh                # Flash script
└── main/
    ├── CMakeLists.txt      # Main component config
    ├── tetris_main.c       # Entry point
    ├── tetris_game.c       # Game logic (pieces, rotation, lines)
    ├── tetris_game.h       # Game header
    ├── lcd_driver.c        # LCD driver
    └── lcd_driver.h        # LCD header
```

## Configuration

Game parameters can be adjusted in `tetris_game.h`:

```c
#define BOARD_WIDTH   10    // Board width (standard Tetris)
#define BOARD_HEIGHT  20    // Board height (standard Tetris)
#define BLOCK_SIZE    12    // Block size in pixels
```

Speed progression in `tetris_game.c`:
```c
static int get_drop_interval(void) {
    int base = 60;              // Base speed (lower = faster)
    int reduction = (game.level - 1) * 5;  // Speed increase per level
    int interval = base - reduction;
    return (interval < 10) ? 10 : interval;
}
```

## Future Enhancements

- [ ] Ghost piece (shows where piece will land)
- [ ] Hold piece feature
- [ ] Sound effects using buzzer
- [ ] High score persistence (NVS storage)
- [ ] Different game modes (marathon, sprint, ultra)
- [ ] Combo counter and display
- [ ] Particle effects for line clears
- [ ] Touch controls using joystick

## License

MIT License - See main project LICENSE file

## Credits

Based on the classic Tetris game by Alexey Pajitnov (1984).
Implemented for educational purposes on the BYUI e-Badge platform.

## Gameplay Tips from the Pros

> "The key to high scores is patience and planning. Don't just place pieces quickly - think about the next 2-3 pieces and how they'll fit together."

Play responsibly and enjoy! 🎮✨
