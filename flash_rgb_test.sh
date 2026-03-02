#!/bin/bash
# Flash RGB LED test program

# Source ESP-IDF environment
source /Users/lynnrwatson/esp/v5.5/esp-idf/export.sh

# Flash to device
idf.py -p /dev/tty.usbserial-1310 flash monitor
