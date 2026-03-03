#!/bin/bash
# Flash and monitor game launcher

echo "Flashing Game Launcher..."
idf.py -p /dev/ttyUSB0 flash monitor
