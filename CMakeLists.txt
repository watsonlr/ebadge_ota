# Top-level CMakeLists.txt for ESP32-S3 OTA Loader
cmake_minimum_required(VERSION 3.16)

# Set partition table to our custom one
set(PARTITION_CSV_PATH "${CMAKE_SOURCE_DIR}/partitions.csv")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(ota_loader)
