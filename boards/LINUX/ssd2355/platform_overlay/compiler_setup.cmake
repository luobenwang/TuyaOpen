# have COMPILE_PREX
if (NOT DEFINED CONFIG_COMPILE_PREX)
    set(CONFIG_COMPILE_PREX "aarch64-none-linux-gnu-")
endif()

# Repo root, resolved relative to this file so it works whether this file is
# included from the top-level build (boards/<...>) or from the platform build
# (platform/LINUX). compiler_setup.cmake lives at platform/LINUX/, so ../.. is
# the repository root.
get_filename_component(OPEN_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)

# Is cross compile?
if ("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    if (EXISTS "/sys/firmware/devicetree/base/model")
        file(READ "/sys/firmware/devicetree/base/model" DEVICE_MODEL)
        string(TOLOWER "${DEVICE_MODEL}" DEVICE_MODEL_LOWER)
        # DshanPi_A1
        if (DEVICE_MODEL_LOWER MATCHES "dshanpi a1")
            set(IS_CROSS_COMPILE FALSE)
        else()
            set(IS_CROSS_COMPILE TRUE)
        endif()
        # Raspberry_Pi
        if (DEVICE_MODEL_LOWER MATCHES "raspberry pi")
            set(IS_CROSS_COMPILE FALSE)
        else()
            set(IS_CROSS_COMPILE TRUE)
        endif()
    else()
        if (PLATFORM_BOARD STREQUAL "Raspberry_Pi")
            set(IS_CROSS_COMPILE TRUE)
        elseif (PLATFORM_BOARD STREQUAL "DshanPi_A1")
            set(IS_CROSS_COMPILE TRUE)
        else()
            set(IS_CROSS_COMPILE FALSE)
        endif()
    endif()
else()
    set(IS_CROSS_COMPILE TRUE)
endif()

# Toolchain path
get_filename_component(TOOLCHAIN_PATH "${CMAKE_CURRENT_LIST_DIR}/../tools/" ABSOLUTE)

if (PLATFORM_BOARD STREQUAL "t113_glibc")
    # Allwinner T113 (sunxi) bundled OpenWrt glibc toolchain (armv7-a, hardfp).
    set(T113_WRAPPER_DIR "${OPEN_ROOT_DIR}/boards/LINUX/t113_glibc/toolchain_wrapper")
    set(T113_TC_BIN "${OPEN_ROOT_DIR}/boards/LINUX/t113_glibc/toolchain-sunxi-glibc/toolchain/bin")
    if (NOT EXISTS "${T113_WRAPPER_DIR}/t113-gcc")
        message(FATAL_ERROR "T113 toolchain wrapper not found: ${T113_WRAPPER_DIR}/t113-gcc")
    endif()
    set(CONFIG_COMPILE_PREX "arm-openwrt-linux-gnueabi-")
    set(CMAKE_C_COMPILER   "${T113_WRAPPER_DIR}/t113-gcc")
    set(CMAKE_CXX_COMPILER "${T113_WRAPPER_DIR}/t113-g++")
    set(CMAKE_ASM_COMPILER "${T113_WRAPPER_DIR}/t113-gcc")
    set(CMAKE_AR      "${T113_TC_BIN}/arm-openwrt-linux-gnueabi-ar")
    set(CMAKE_RANLIB  "${T113_TC_BIN}/arm-openwrt-linux-gnueabi-ranlib")
    set(CMAKE_STRIP   "${T113_TC_BIN}/arm-openwrt-linux-gnueabi-strip")
elseif (PLATFORM_BOARD STREQUAL "ssd2355")
    # SigmaStar SSD2355 bundled OpenWrt glibc toolchain (armv7-a, hardfp).
    set(SSD2355_WRAPPER_DIR "${OPEN_ROOT_DIR}/boards/LINUX/ssd2355/toolchain_wrapper")
    set(SSD2355_TC_BIN "${OPEN_ROOT_DIR}/boards/LINUX/ssd2355/bin")
    if (NOT EXISTS "${SSD2355_WRAPPER_DIR}/ssd2355-gcc")
        message(FATAL_ERROR "SSD2355 toolchain wrapper not found: ${SSD2355_WRAPPER_DIR}/ssd2355-gcc")
    endif()
    set(CONFIG_COMPILE_PREX "arm-openwrt-linux-gnueabi-")
    set(CMAKE_C_COMPILER   "${SSD2355_WRAPPER_DIR}/ssd2355-gcc")
    set(CMAKE_CXX_COMPILER "${SSD2355_WRAPPER_DIR}/ssd2355-g++")
    set(CMAKE_ASM_COMPILER "${SSD2355_WRAPPER_DIR}/ssd2355-gcc")
    set(CMAKE_AR      "${SSD2355_TC_BIN}/arm-openwrt-linux-gnueabi-ar")
    set(CMAKE_RANLIB  "${SSD2355_TC_BIN}/arm-openwrt-linux-gnueabi-ranlib")
    set(CMAKE_STRIP   "${SSD2355_TC_BIN}/arm-openwrt-linux-gnueabi-strip")
elseif (IS_CROSS_COMPILE)
    set(TOOLCHAIN_PREFIX "")
    if (PLATFORM_BOARD STREQUAL "Raspberry_Pi")
        get_filename_component(TOOLCHAIN_PREFIX "${TOOLCHAIN_PATH}/aarch64-none-linux-gnu-14.2-2024.10/bin" ABSOLUTE)
        # TOOLCHAIN_PREFIX Check
        if(NOT EXISTS "${TOOLCHAIN_PREFIX}")
            message(FATAL_ERROR "Toolchain directory does not exist: ${TOOLCHAIN_PREFIX}")
        endif()

        get_filename_component(TOOLCHAIN_PREFIX "${TOOLCHAIN_PREFIX}/${CONFIG_COMPILE_PREX}" ABSOLUTE)
    elseif (PLATFORM_BOARD STREQUAL "DshanPi_A1")
        get_filename_component(TOOLCHAIN_PREFIX "${TOOLCHAIN_PATH}/aarch64-none-linux-gnu-13.3-2024.04/bin" ABSOLUTE)
        # TOOLCHAIN_PREFIX Check
        if(NOT EXISTS "${TOOLCHAIN_PREFIX}")
            message(FATAL_ERROR "Toolchain directory does not exist: ${TOOLCHAIN_PREFIX}")
        endif()

        get_filename_component(TOOLCHAIN_PREFIX "${TOOLCHAIN_PREFIX}/${CONFIG_COMPILE_PREX}" ABSOLUTE)
    else()
        message(FATAL_ERROR "Unsupported PLATFORM_BOARD for cross compilation: ${PLATFORM_BOARD}")
    endif()

    # On Windows, add .exe suffix to executables
    if(WIN32)
        set(EXE_SUFFIX ".exe")
    else()
        set(EXE_SUFFIX "")
    endif()

    set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc${EXE_SUFFIX}")
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++${EXE_SUFFIX}")
    set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc${EXE_SUFFIX}")
    set(CMAKE_AR "${TOOLCHAIN_PREFIX}ar${EXE_SUFFIX}")
    set(CMAKE_RANLIB "${TOOLCHAIN_PREFIX}ranlib${EXE_SUFFIX}")
    set(CMAKE_STRIP "${TOOLCHAIN_PREFIX}strip${EXE_SUFFIX}")
else()
    # Native compile settings
    set(CMAKE_C_COMPILER "gcc")
    set(CMAKE_CXX_COMPILER "g++")
    set(CMAKE_ASM_COMPILER "gcc")
    set(CMAKE_AR "ar")
    set(CMAKE_RANLIB "ranlib")
    set(CMAKE_STRIP "strip")
endif()
