# Hardware Configuration

## Target Device
- **MCU**: ESP32-S3-Mini-1-N8 (Xtensa dual-core @ 240 MHz)
- **Flash**: 8 MB embedded
- **PSRAM**: None (N8 variant)
- **RAM**: 512 KB SRAM
- **Wi-Fi**: 2.4 GHz 802.11 b/g/n
- **Bluetooth**: BLE 5.0

**Note**: Project currently configured for ESP32-C3 in `sdkconfig.defaults`. Update target with `idf.py set-target esp32s3` for ESP32-S3 builds.

## Supported Development Boards
- ESP32-C3-DevKitM-1
- ESP32-C3-MINI-1 module-based boards
- Custom boards with ESP32-C3-WROOM-02 module

## Pin Assignments (ESP32-S3)

### BYUI e-Badge V3.0 Complete Pinout

| GPIO | Primary Function | Alternative Uses | Notes |
|------|-----------------|------------------|-------|
| IO0 | Boot/Strapping | - | Hold LOW for download mode |
| IO1 | Joystick X-axis | ADC1_CH0, TOUCH1 | Analog input |
| IO2 | Joystick Y-axis | ADC1_CH1, TOUCH2 | Analog input |
| IO3 | SD Card CS | ADC1_CH2, TOUCH3 | Strapping pin |
| IO4 | RGB Blue LED | ADC1_CH3, TOUCH4 | PWM output |
| IO5 | RGB Green LED | ADC1_CH4, TOUCH5 | PWM output |
| IO6 | RGB Red LED | ADC1_CH5, TOUCH6 | PWM output |
| IO7 | Addressable LEDs | ADC1_CH6, TOUCH7 | WS2813B data line |
| IO8 | Minibadge CLK | ADC1_CH7, SUBSPICS1 | Extension connector |
| IO9 | Display CS | FSPIHD, SUBSPIHD | SPI chip select |
| IO10 | SPI2 MISO | FSPICS0, FSPIIO4 | Display/SD SPI |
| IO11 | SPI2 MOSI | FSPID, FSPIIO5 | Display/SD SPI |
| IO12 | SPI2 CLK | FSPICLK, FSPIIO6 | Display/SD SPI |
| IO13 | Display DC | FSPIQ, FSPIIO7 | Data/Command select |
| IO14 | Button L | ADC2_CH3, TOUCH14 | Left button |
| IO15 | Button R | U0RTS, ADC2_CH4 | Right button |
| IO16 | Button D | U0CTS, ADC2_CH5 | Down button |
| IO17 | Button U | U1TXD, ADC2_CH6 | Up button |
| IO18 | Button B | U1RXD, ADC2_CH7 | B button |
| IO19 | USB D- | U1RTS, ADC2_CH8 | Native USB |
| IO20 | USB D+ | U1CTS, ADC2_CH9 | Native USB |
| IO21 | I2C SCL | - | Accelerometer clock |
| IO26 | **PSRAM** | SPICS1 | **Reserved for PSRAM (N4R2 modules)** |
| IO33-37 | **SPI Flash** | - | **Internal flash - do not use** |
| IO38 | Button A | FSPIWP, SUBSPIWP | A button |
| IO39 | JTAG MTCK | CLK_OUT3 | Debug (exposed) |
| IO40 | JTAG MTDO | CLK_OUT2 | Debug (exposed) |
| IO41 | JTAG MTDI / C LED | CLK_OUT1 | Debug / indicator |
| IO42 | JTAG MTMS / Buzzer | - | Debug / piezo |
| IO43 | UART TX | U0TXD, CLK_OUT1 | Console output |
| IO44 | UART RX | U0RXD, CLK_OUT2 | Console input |
| IO45 | Strapping | - | Boot config |
| IO46 | Strapping | - | Boot config |
| IO47 | I2C SDA | SUBSPICLK_P_DIFF | Accelerometer data |
| IO48 | Display RST | SUBSPICLK_N_DIFF | Display reset |

**Legend:**
- **Y (In Use)**: Pin actively used, not exposed
- **X (Exposed)**: Pin used but available on extension header/pads
- Strapping pins require specific states during boot

### Reserved/System Pins
| Pin | Function | Notes |
|-----|----------|-------|
| IO0 | Boot Mode / Strapping | Pulled HIGH for normal boot, LOW for download mode |
| IO3 | Strapping | Used for SD CS, strapping pin |
| IO45 | Strapping | Boot configuration |
| IO46 | Strapping | Boot configuration |
| IO26 | **PSRAM** | **Reserved on N4R2 modules - do not use** |
| IO33-37 | **SPI Flash** | **Internal flash interface - do not use** |

### Button Inputs
| Button | GPIO | Pull Configuration | Notes |
|--------|------|---------------------|-------|
| Up (U) | IO17 | Internal pull-up recommended | Direction pad |
| Down (D) | IO16 | Internal pull-up recommended | Direction pad |
| Left (L) | IO14 | Internal pull-up recommended | Direction pad |
| Right (R) | IO15 | Internal pull-up recommended | Direction pad |
| A | IO38 | Internal pull-up recommended | Action button |
| B | IO18 | Internal pull-up recommended | Action button |

### Display Interface (SPI)
| Signal | GPIO | Function |
|--------|------|----------|
| CS | IO9 | Chip select |
| DC | IO13 | Data/Command |
| RST | IO48 | Reset |
| MOSI | IO11 | SPI2 data out |
| MISO | IO10 | SPI2 data in |
| CLK | IO12 | SPI2 clock |

### SD Card Interface (SPI - Shared with Display)
| Signal | GPIO | Function |
|--------|------|----------|
| CS | IO3 | Chip select |
| MOSI | IO11 | SPI2 data out (shared) |
| MISO | IO10 | SPI2 data in (shared) |
| CLK | IO12 | SPI2 clock (shared) |

### UART (Programming/Debug)
| Pin | Function | Alternative |
|-----|----------|-------------|
| IO43 | UART0 TX | USB D- (if USB-serial chip present) |
| IO44 | UART0 RX | USB D+ (if USB-serial chip present) |
| IO19 | USB D- | Native USB (if supported) |
| IO20 | USB D+ | Native USB (if supported) |

### Available GPIO for Application
| Pin | Recommended Use | Notes |
|-----|-----------------|-------|
| GPIO0 | Button / Input | Has pull-up; strap pin |
| GPIO1 | LED / Output | General purpose |
| GPIO2 | LED / Output | Strap pin; has pull-down |
| GPIO3 | General I/O | ADC1_CH3 capable |
| GPIO4 | General I/O | ADC1_CH4 capable |
| GPIO5 | General I/O | ADC2_CH0 capable |
| GPIO10 | General I/O | |
| GPIO18 | General I/O | |
| GPIO19 | General I/O | |

### SPI Flash (Internal - Do Not Use)
| Pin | Function |
|-----|----------|
| GPIO12 | SPIHD |
| GPIO13 | SPIWP |
| GPIO14 | SPICS0 |
| GPIO15 | SPICLK |
| GPIO16 | SPID |
| GPIO17 | SPIQ |

## Peripheral Configuration

### Accelerometer (MMA8452Q)
- **Interface**: I2C
- **I2C Address**: 0x1C or 0x1D (configurable via SA0 pin)
- **GPIOs**: SDA=GPIO47, SCL=GPIO21

### Buzzer (MLT-5020)
- **Type**: Piezo
- **Control**: PWM or GPIO toggle
- **GPIO**: GPIO42

### Joystick (Adafruit 2765)
- **Interface**: Analog (ADC) for X/Y, GPIO for button
- **GPIOs**: 
  - X-axis: GPIO1 (ADC1_CH0)
  - Y-axis: GPIO2 (ADC1_CH1)
  - Button: (not mapped in pinout table)

### RGB LED & Addressable LEDs
- **RGB LED**: RS-3535MWAM (common anode/cathode - check datasheet)
  - Red: GPIO6
  - Green: GPIO5
  - Blue: GPIO4
- **Addressable LEDs**: WS2813B-2821 (NeoPixel-compatible)
  - Protocol: WS2812/WS2813 (single-wire)
  - Data GPIO: GPIO7 (RMT channel)

### SD Card (TF PUSH)
- **Interface**: SPI (shared with Display on SPI2)
- **GPIOs**:
  - CS: GPIO3
  - CLK: GPIO12 (shared with display)
  - MOSI: GPIO11 (shared with display)
  - MISO: GPIO10 (shared with display)

### Display (TFT)
- **Interface**: SPI2 (shared with SD Card)
- **GPIOs**:
  - CS: GPIO9
  - DC (Data/Command): GPIO13
  - RST (Reset): GPIO48
  - CLK: GPIO12 (shared)
  - MOSI: GPIO11 (shared)
  - MISO: GPIO10 (shared)
- **Note**: SPI2 bus is shared between display and SD card. Manage chip selects carefully.

### Charging (TP4056)
- **Battery**: Single-cell Li-ion/LiPo (4.2V max)
- **Charge Current**: Programmable via RPROG resistor
- **Status Outputs**: CHRG (charging), STDBY (complete)
- **Monitor GPIOs**: (Not mapped in pinout table - check schematic for status LEDs)

### Wi-Fi
- **Mode**: SoftAP (provisioning) + STA (client)
- **SSID (Provisioning)**: `BYUI_NameBadge`
- **Channel**: 6 (2.4 GHz)
- **Security**: Open (provisioning), WPA2-PSK (client)

### UART/Serial Console
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Parity**: None
- **Stop Bits**: 1
- **USB-Serial Bridge**: CP2102N-A02-GQFN28R
  - Auto DTR/RTS reset circuit recommended
  - Native USB UART pins: GPIO43 (TX), GPIO44 (RX)

### LED Indicators
| LED | GPIO | Purpose | Active |
|-----|------|---------|--------|
| Red LED | GPIO6 | Status/RGB Red | PWM |
| Green LED | GPIO5 | Status/RGB Green | PWM |
| Blue LED | GPIO4 | Status/RGB Blue | PWM |
| WS2813B | GPIO7 | Addressable LEDs | RMT |

### Buttons
| Button | GPIO | Function | Logic |
|--------|------|----------|-------|
| Up | GPIO17 | D-pad navigation | Active LOW (pull-up) |
| Down | GPIO16 | D-pad navigation | Active LOW (pull-up) |
| Left | GPIO14 | D-pad navigation | Active LOW (pull-up) |
| Right | GPIO15 | D-pad navigation | Active LOW (pull-up) |
| A Button | GPIO38 | Action | Active LOW (pull-up) |
| B Button | GPIO18 | Action | Active LOW (pull-up) |
| Boot | GPIO0 | Enter download mode | Active LOW (strapping) |
| Reset | EN/RST | System reset | Active LOW |

## Power Requirements
- **Supply Voltage**: 3.3V (3.0V - 3.6V operating range)
- **Typical Current**: 
  - Active Wi-Fi TX: ~120-140 mA
  - Active Wi-Fi RX: ~80-95 mA
  - Modem sleep: ~15-20 mA
  - Light sleep: ~0.8 mA
  - Deep sleep: ~5 µA
- **Recommended PSU**: 500 mA minimum for stable operation

## USB-to-Serial Interface
If using external USB-UART bridge:
- **Chipsets**: CP2102, CH340, FT232RL
- **DTR/RTS**: Auto-reset circuit (optional but recommended)
  - DTR → GPIO0 (via RC circuit)
  - RTS → EN (via RC circuit)

## External Components (Minimal Configuration)
- **Decoupling Capacitors**: 
  - 100 nF ceramic (close to VDD pins)
  - 10 µF electrolytic (power input)
- **EN Pull-up**: 10 kΩ to 3.3V
- **GPIO0 Pull-up**: 10 kΩ to 3.3V (if used as button, external pull-up optional)
- **Reset Button**: Momentary switch between EN and GND

## PCB Design Notes
- Keep Wi-Fi antenna area clear of ground plane
- Use recommended PCB antenna design from Espressif reference
- Route high-speed signals (SPI flash) with controlled impedance
- Add ESD protection on exposed I/O
- Use 4-layer PCB for better EMI performance (optional)

## Schematic Reference
<!-- Add link to schematic PDF or image here -->
```
Coming soon: Full schematic diagram
```

## Bill of Materials (BOM)
<!-- Add BOM spreadsheet or table here -->
| Part | Description | Qty | Notes |
|------|-------------|-----|-------|
| U1 | ESP32-C3-WROOM-02 | 1 | 4 MB flash |
| C1, C2 | 100nF 0603 | 2 | Decoupling |
| C3 | 10µF 1206 | 1 | Power supply |
| R1, R2 | 10kΩ 0603 | 2 | Pull-ups |
| SW1 | Tactile button | 1 | Boot |
| SW2 | Tactile button | 1 | Reset |

## Component Datasheets

| Part             | Part Number/Value   | Datasheet Link                                                                                                                          |
|------------------|---------------------|-----------------------------------------------------------------------------------------------------------------------------------------|
| Accelerometer    | MMA8452QR1_C11360   | [Link](https://lcsc.com/datasheet/lcsc_datasheet_2405281404_NXP-Semicon-MMA8452QR1_C11360.pdf)                                          |
| Buzzer           | MLT-5020            | [Link](https://lcsc.com/datasheet/lcsc_datasheet_2410121451_Jiangsu-Huaneng-Elec-MLT-5020_C94598.pdf)                                   |
| Charging Manager | TP4056              | [Link](https://www.lcsc.com/datasheet/lcsc_datasheet_1809261820_TOPPOWER-Nanjing-Extension-Microelectronics-TP4056-42-ESOP8_C16581.pdf) |
| MCU              | ESP32-S3-Mini-1-N8  | [Link](https://www.espressif.com/sites/default/files/documentation/esp32-s3-mini-1_mini-1u_datasheet_en.pdf)                            |
| Joystick         | 2765                | [Link](https://www.adafruit.com/product/2765)                                                                                           |
| RGB LED          | RS-3535MWAM         | [Link](https://lcsc.com/datasheet/lcsc_datasheet_2410121728_Foshan-NationStar-Optoelectronics-RS-3535MWAM_C842778.pdf)                  |
| SD Card          | TF PUSH             | [Link](https://lcsc.com/datasheet/lcsc_datasheet_2504101957_SHOU-HAN-TF-PUSH_C393941.pdf)                                               |
| USB-Serial Chip  | CP2102N-A02-GQFN28R | [Link](https://lcsc.com/datasheet/lcsc_datasheet_2304140030_SKYWORKS-SILICON-LABS-CP2102N-A02-GQFN28R_C964632.pdf)                      |
| Addressable LEDs | WS2813B-2121 (C22371528) | [Link](https://item.szlcsc.com/datasheet/WS2813B-2121/23859548.html)                                                                    |

## Programming/Flashing
### USB Connection (WSL)
- **Windows Port**: COM10 (example)
- **WSL Port**: `/dev/ttyS10` or `/dev/ttyUSB0`
- **Baud Rate (Flash)**: 921600 (default) or 460800/115200 (fallback)

### Flash Layout
See `partitions.csv` for detailed partition table.

| Partition | Type | Subtype | Offset | Size |
|-----------|------|---------|--------|------|
| nvs | data | nvs | 0x9000 | 24K |
| otadata | data | ota | 0xF000 | 8K |
| phy_init | data | phy | 0x11000 | 4K |
| factory | app | factory | 0x20000 | 1M |
| ota_0 | app | ota_0 | 0x120000 | 1M |
| ota_1 | app | ota_1 | 0x220000 | 1M |
| spiffs | data | spiffs | 0x320000 | 896K |

## Troubleshooting Hardware Issues

### Device Not Detected
- Check USB cable (must be data cable, not charge-only)
- Verify CP210x/CH340 drivers installed
- Try different USB port
- Hold BOOT button while connecting

### Flash Failures
- Lower baud rate: `idf.py -p /dev/ttyS10 -b 115200 flash`
- Check power supply stability (add bulk capacitor)
- Verify 3.3V rail voltage
- Check for shorts on flash pins

### Wi-Fi Not Working
- Verify antenna connection/quality
- Check RF layout and ground plane
- Ensure 2.4 GHz network (not 5 GHz)
- Move closer to access point

### Brownout/Reset Issues
- Increase power supply capacity (>500 mA)
- Add bulk capacitance (100-220 µF)
- Reduce TX power in menuconfig
- Check for voltage drop under load

## Safety & Compliance
- **FCC/CE**: Ensure proper RF shielding and filtering for commercial products
- **ESD**: Add TVS diodes on exposed I/O
- **Thermal**: Ensure adequate ventilation for continuous Wi-Fi operation

## Additional Resources
- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [ESP32-C3 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32-c3_technical_reference_manual_en.pdf)
- [ESP32-C3-DevKitM-1 Schematic](https://dl.espressif.com/dl/schematics/SCH_ESP32-C3-DEVKITM-1_V1_20200915A.pdf)
- [Hardware Design Guidelines](https://www.espressif.com/sites/default/files/documentation/esp32-c3_hardware_design_guidelines_en.pdf)

---

**Last Updated**: February 2026  
**Maintained By**: [Your Name/Team]
