# Game Launcher

A menu system for the ESP32-S3 e-Badge that allows you to browse and select games to play.

## Features

- **Visual Menu System**: Clean, colorful interface showing available games
- **Game Icons**: Unique icons for each game with theme colors
- **Smooth Navigation**: D-pad controls with debouncing
- **Scrollable List**: Supports many games with auto-scroll
- **Game Information**: Shows game name and description
- **OTA Ready**: Framework for loading games via OTA updates

## Hardware

- **Display**: ILI9341 240x320 LCD
- **Controls**: 6-button input (D-pad + A/B buttons)
- **MCU**: ESP32-S3-Mini-1-N8

## Controls

| Button | Action |
|--------|--------|
| UP     | Move selection up |
| DOWN   | Move selection down |
| A      | Launch selected game |
| B      | Options (future use) |

## Available Games

1. **PAC-MAN** - Classic arcade maze chase game
2. **TETRIS** - Stack blocks and clear lines
3. **FROGGER** - Cross roads and rivers

## Building

```bash
cd Apps/game_launcher
./build.sh
```

Or manually:
```bash
idf.py build
```

## Flashing

```bash
./flash.sh
```

Or manually:
```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

## Configuration

### Adding New Games

Edit `main/menu.c` and add entries to the `game_database` array:

```c
{
    .name = "YOUR GAME",
    .description = "Short description",
    .ota_url = "http://server:8080/yourgame.bin",
    .color = COLOR_YOUR_COLOR,
    .available = true
}
```

### Customizing Colors

Available colors defined in `menu.h`:
- `COLOR_RED`, `COLOR_GREEN`, `COLOR_BLUE`
- `COLOR_CYAN`, `COLOR_MAGENTA`, `COLOR_YELLOW`
- `COLOR_ORANGE`, `COLOR_PURPLE`
- `COLOR_WHITE`, `COLOR_BLACK`, `COLOR_GRAY`

### OTA Configuration

Update the IP address and port in `game_database` array in `menu.c` to match your OTA server.

## File Structure

```
game_launcher/
├── CMakeLists.txt              # Project build config
├── sdkconfig.defaults          # Default ESP-IDF settings
├── build.sh                    # Build script
├── flash.sh                    # Flash script
├── README.md                   # This file
└── main/
    ├── CMakeLists.txt          # Component build config
    ├── launcher_main.c         # Entry point
    ├── menu.c                  # Menu implementation
    ├── menu.h                  # Menu header
    ├── lcd_driver.c            # Display driver
    └── lcd_driver.h            # Display driver header
```

## Technical Details

### Display

- **Resolution**: 240x320 pixels
- **Color Format**: RGB565 (16-bit, little-endian)
  - Byte order: Low byte first, high byte second
  - Send as: `{color & 0xFF, color >> 8}`
- **Driver**: ILI9341 via SPI
- **Refresh Rate**: ~60 FPS

### Menu System

- **Visible Items**: 3 games at a time
- **Total Capacity**: Up to 10 games (expandable)
- **Scrollbar**: Automatic when > 3 games
- **Item Height**: 60 pixels per menu item

### Button Debouncing

- **Debounce Time**: 50ms
- **Polling Rate**: 60Hz (16ms)
- **Active Level**: LOW (pull-up enabled)

## Future Enhancements

- [ ] Implement OTA game loading
- [ ] Save game preferences to NVS
- [ ] Add game statistics/high scores
- [ ] Settings menu (brightness, sound, etc.)
- [ ] Game screenshots/previews
- [ ] WiFi configuration interface
- [ ] Achievements/unlockables

## OTA Implementation Notes

The launcher includes placeholder code for OTA updates. To fully implement:

1. **Connect to WiFi** - Add WiFi initialization in `launcher_main.c`
2. **HTTP Client** - Use `esp_http_client` to download game binaries
3. **OTA API** - Use `esp_ota_begin()`, `esp_ota_write()`, `esp_ota_end()`
4. **Partition Switching** - Use `esp_ota_set_boot_partition()` to switch apps
5. **Reboot** - Call `esp_restart()` to load new game

Example OTA flow:
```c
const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
esp_ota_handle_t update_handle;
esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
// Download and write game binary...
esp_ota_end(update_handle);
esp_ota_set_boot_partition(update_partition);
esp_restart();
```

## License

MIT License - Feel free to modify and expand!

## Credits

Created for the ESP32-S3 e-Badge V3.0 hardware platform.
