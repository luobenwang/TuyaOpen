##
# @file toolchain_file.cmake
# @brief 
#/

include("${CMAKE_CURRENT_LIST_DIR}/compiler_setup.cmake")

if (PLATFORM_BOARD STREQUAL "Raspberry_Pi" AND IS_CROSS_COMPILE)
    # Raspberry_Pi build cmake settings
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
elseif (PLATFORM_BOARD STREQUAL "DshanPi_A1" AND IS_CROSS_COMPILE)
    # DshanPi_A1 build cmake settings
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR aarch64)
elseif (PLATFORM_BOARD STREQUAL "t113_glibc")
    # Allwinner T113 build cmake settings (32-bit armv7-a, hardfp)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
elseif (PLATFORM_BOARD STREQUAL "ssd2355")
    # SigmaStar SSD2355 build cmake settings (32-bit armv7-a, hardfp)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
else()
    # Other platform build cmake settings
    set(CMAKE_SYSTEM_NAME Linux)
endif()

set(CMAKE_C_FLAGS " -g -O1")
