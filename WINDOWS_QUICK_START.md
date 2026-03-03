# Quick Start - Building eBadge Games (Windows)

## Step 1: Open ESP-IDF PowerShell

1. Press Windows key
2. Search for "ESP-IDF PowerShell"
3. Click to open (or run as Administrator if needed)

You should see something like:
```
ESP-IDF v5.3.1
```

## Step 2: Navigate to Project

```powershell
cd C:\Users\lynn\Documents\Repositories\ebadge_ota
```

## Step 3: Build All Games

Simply run:
```batch
build_all.bat
```

This will:
- Build Pac-Man game
- Build Tetris game
- Build Frogger game
- Build Game Launcher
- Copy all binaries to `ota_files\apps\`

**Build time:** ~2-5 minutes per game (first build is slower)

## Step 4: Flash Game Launcher to Device

1. **Connect your eBadge** via USB cable

2. **Find COM port:**
   - Open Device Manager (Win+X → Device Manager)
   - Look under "Ports (COM & LPT)"
   - Find "USB-SERIAL CH340" or similar (e.g., COM3)

3. **Flash the launcher:**
   ```batch
   cd Apps\game_launcher
   idf.py -p COM3 flash
   ```
   Replace `COM3` with your actual port

4. **Monitor output (optional):**
   ```batch
   idf.py -p COM3 monitor
   ```
   Press Ctrl+] to exit monitor

## Step 5: Use the Game Launcher

After flashing completes:
1. Press RESET button on eBadge
2. You'll see the Game Launcher menu
3. Use UP/DOWN buttons to select a game
4. Press A button to "launch" (currently shows loading screen)

**Note:** Full OTA game loading not yet implemented. To actually play games, flash them individually:

```batch
cd Apps\pacman
idf.py -p COM3 flash

cd ..\tetris
idf.py -p COM3 flash

cd ..\frogger
idf.py -p COM3 flash
```

## Troubleshooting

### "idf.py: command not found"
- Make sure you're in "ESP-IDF PowerShell" not regular PowerShell
- Or run: `C:\Espressif\frameworks\esp-idf-v5.3.1\export.ps1`

### "Serial port COM3 could not be opened"
- Close any other terminal/serial programs
- Check Device Manager for correct COM port
- Try unplugging and replugging USB cable
- Hold BOOT button while flashing if auto-reset fails

### Build errors
- Clean and retry:
  ```batch
  idf.py fullclean
  idf.py build
  ```
- Check ESP-IDF version: `idf.py --version` (should be 5.0+)
- Make sure target is set: `idf.py set-target esp32s3`

### Device not detected
- Install CH340 USB driver if needed
- Check USB cable (must be data cable, not charge-only)
- Try different USB port

## Next Steps

- **Play the games!** Each has unique controls (see game READMEs)
- **Start OTA server:** `python simple_ota_server.py`
- **Create your own game:** Follow template in `Apps\README.md`
- **Modify existing games:** Edit code in `Apps\<game>\main\`

## File Locations

After building, find binaries here:
```
ota_files\apps\
├── launcher.bin    (flash this first!)
├── pacman.bin
├── tetris.bin
└── frogger.bin
```

Source code locations:
```
Apps\
├── game_launcher\  (menu system)
├── pacman\         (Pac-Man game)
├── tetris\         (Tetris game)
└── frogger\        (Frogger game)
```

## Controls Reference

**Game Launcher:**
- UP/DOWN: Navigate
- A: Select game

**Pac-Man:**
- D-pad: Move
- A: Pause
- B: Restart

**Tetris:**
- LEFT/RIGHT: Move piece
- UP: Rotate
- DOWN: Soft drop
- A: Hard drop
- B: Pause

**Frogger:**
- D-pad: Move frog
- A: Next level
- B: Restart

---

**Need more help?** See [BUILD_GUIDE.md](BUILD_GUIDE.md) for detailed instructions.

🎮 Happy Gaming! 🎮
