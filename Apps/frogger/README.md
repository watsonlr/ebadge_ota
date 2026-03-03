# Frogger Game for ESP32-S3 e-Badge

Classic Frogger arcade game implementation for the BYUI e-Badge V3.0 with ESP32-S3.

## Features

- **Classic Frogger Gameplay**: Guide frog across roads and rivers
- **Multiple Lanes**: 5 road lanes with cars/trucks, 6 river lanes with logs/turtles
- **Dynamic Traffic**: Vehicles and platforms move at different speeds
- **Goal System**: 5 goal spots to reach at the top
- **Lives System**: 3 lives to start
- **Time Pressure**: 60 seconds to complete each level
- **Progressive Difficulty**: Speed increases with each level
- **Score Tracking**: Points for progress and goals

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
| ↑ Up | Move frog forward |
| ↓ Down | Move frog backward |
| ← Left | Move frog left |
| → Right | Move frog right |
| A | Start next level (when complete) |
| B | Restart game |

## Game Mechanics

### Objective
Guide your frog from the bottom safe zone to one of the 5 goal spots at the top without:
- Getting hit by vehicles on the road
- Falling in the water
- Running out of time

### Lanes (Bottom to Top)
1. **Safe Start Zone** (Rows 0-1): Starting area - green grass
2. **Road** (Rows 2-6): Dodge cars and trucks
3. **Safe Median** (Row 7): Safe zone between road and river
4. **River** (Rows 8-13): Jump on logs and turtles
5. **Goal Zone** (Row 14): 5 goal spots to reach

### Scoring
- **Forward Movement**: +10 points per row
- **Reaching Goal**: +100 points
- **Level Complete**: Complete all 5 goals

### Hazards

#### On the Road
- **Cars** (Red/Blue): 2 grid units wide, various speeds
- **Trucks** (Brown): 3 grid units wide, slower but longer

#### In the River
- **Logs** (Brown): Short (2), Medium (3), or Long (4) units wide
- **Turtles** (Green): 2 units wide platforms

**Important**: You must jump onto logs or turtles to cross the river. Jump in the water = death!

### Strategy Tips
1. **Look ahead**: Check traffic patterns before crossing
2. **Timing is key**: Wait for gaps in traffic
3. **Stay on platforms**: In the river, always be on a log or turtle
4. **Watch the edges**: Logs can push you off the screen
5. **Use the median**: Safe zone to plan your river crossing
6. **Time management**: 60 seconds per level - don't wait too long!

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
   cd Apps/frogger
   ```

3. **Build the project**:
   ```bash
   ./build.sh
   ```

4. **Flash to device**:
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
cp build/frogger_game.bin ../../ota_files/apps/
```

Update your `manifest.json`:
```json
{
  "apps": [
    {
      "name": "Frogger Game",
      "version": "1.0.0",
      "url": "http://YOUR_SERVER_IP:8080/apps/frogger_game.bin"
    }
  ]
}
```

## Troubleshooting

### Display Issues
- Verify SPI wiring matches pin definitions
- Check display reset and DC pins
- Lower SPI clock speed if display glitches

### Button Not Responding
- Buttons are active-low with internal pull-ups
- Check GPIO assignments match hardware
- Increase `BUTTON_DEBOUNCE_MS` if too sensitive

### Game Too Fast/Slow
- Adjust lane speeds in `level_lanes` array
- Modify speed multiplier based on level
- Change FPS in main loop

### Collision Detection Issues
- Frog hitbox is 1 grid cell
- Platform detection checks frog center position
- Adjust collision tolerance if needed

## File Structure

```
Apps/frogger/
├── CMakeLists.txt          # ESP-IDF project config
├── sdkconfig.defaults      # Default SDK configuration
├── README.md               # This file
├── build.sh                # Build script
├── flash.sh                # Flash script
└── main/
    ├── CMakeLists.txt      # Main component config
    ├── frogger_main.c      # Entry point
    ├── frogger_game.c      # Game logic (lanes, collision, movement)
    ├── frogger_game.h      # Game header
    ├── lcd_driver.c        # LCD driver
    └── lcd_driver.h        # LCD header
```

## Configuration

Game parameters can be adjusted in `frogger_game.h`:

```c
#define GRID_SIZE     16    // Cell size in pixels
#define GRID_WIDTH    15    // Board width
#define GRID_HEIGHT   20    # Board height
```

Lane speeds and object types in `frogger_game.c`:
```c
static const lane_config_t level_lanes[GRID_HEIGHT] = {
    {LANE_ROAD, DIR_RIGHT, 0.8f, OBJ_CAR_RED, 5},  // speed, type, spacing
    // ... more lanes
};
```

## Gameplay Variations

### Easy Mode
- Increase time limit to 90 seconds
- Reduce vehicle speeds by 50%
- Add more logs/turtles in river

### Hard Mode
- Reduce time limit to 30 seconds
- Increase speeds by 50%
- Fewer platforms in river
- Add moving turtles that dive underwater

## Future Enhancements

- [ ] Diving turtles that temporarily disappear
- [ ] Alligators as additional hazards
- [ ] Lady frog bonus at the top
- [ ] Sound effects using buzzer
- [ ] High score persistence (NVS storage)
- [ ] Bonus items (flies, etc.)
- [ ] Animated sprites for frog directions
- [ ] Particle effects for splashes
- [ ] Multiple difficulty modes

## License

MIT License - See main project LICENSE file

## Credits

Based on the classic Frogger arcade game by Konami (1981).
Implemented for educational purposes on the BYUI e-Badge platform.

## Pro Tips

> "Patience is key - rushing gets you killed. Watch the patterns, wait for the perfect moment, then go for it!"

> "In the river, always plan your next jump before you make the current one. Looking one step ahead keeps you alive."

Hop safely and enjoy! 🐸✨
