# 7segment-demo - comparison screens for seven-segment and fourteen-segment
# digit sizes/styles on the PicoCalc LCD (F1 switches between them).
#
# Reuses the LCD, keyboard and south-bridge drivers from the
# picocalc-text-starter project by Blair Leduc (see drivers/), plus the
# Raspberry Pi Pico SDK. Build with the Raspberry Pi Pico VS Code extension,
# or from a terminal that has PICO_SDK_PATH set (see README.md).

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
# set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(7segment-demo C CXX ASM)

pico_sdk_init()

add_executable(7segment-demo
        main.c
        demo.c
        demo.h
        demo14.c
        demo14.h
        sevenseg.c
        sevenseg.h
        seg14.c
        seg14.h
        constants.h
        drivers/lcd.c
        drivers/lcd.h
        drivers/font-8x10.c
        drivers/font.h
        drivers/keyboard.c
        drivers/keyboard.h
        drivers/southbridge.c
        drivers/southbridge.h
        )

pico_set_program_name(7segment-demo "7segment-demo")
pico_set_program_version(7segment-demo "0.1")

# No stdio needed for this demo.
pico_enable_stdio_uart(7segment-demo 0)
pico_enable_stdio_usb(7segment-demo 0)

target_compile_options(7segment-demo PRIVATE -Wall -Werror)

target_include_directories(7segment-demo PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(7segment-demo
        pico_stdlib
        hardware_spi
        hardware_watchdog   # watchdog_reboot(): leaving for the UF2 Loader
        hardware_gpio
        hardware_i2c
        )

# The chip is part of every output file name (7segment-demo-RP2040.uf2 /
# 7segment-demo-RP2350.uf2, and the matching .elf/.bin/.hex), so a downloaded file
# always says which board it was built for.
if(PICO_PLATFORM MATCHES "^rp2040")
    set(CHIP_NAME RP2040)
elseif(PICO_PLATFORM MATCHES "^rp2350")
    set(CHIP_NAME RP2350)
else()
    set(CHIP_NAME ${PICO_PLATFORM})
endif()
set_target_properties(7segment-demo PROPERTIES OUTPUT_NAME 7segment-demo-${CHIP_NAME})

pico_add_extra_outputs(7segment-demo)

# Copy the finished .uf2 (same name) to the top-level project folder, so the
# RP2040 and RP2350 builds can sit side by side.
add_custom_command(TARGET 7segment-demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE_DIR:7segment-demo>/$<TARGET_FILE_BASE_NAME:7segment-demo>.uf2
            ${CMAKE_CURRENT_LIST_DIR}/
)

