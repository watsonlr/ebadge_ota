# SPDX-FileCopyrightText: 2025
# SPDX-License-Identifier: MIT

idf_component_register(
    SRCS 
        "lcd_test.c"
    INCLUDE_DIRS "."
    REQUIRES 
        driver
)
