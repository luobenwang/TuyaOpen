##
# @file CMakeLists.txt
# @brief Allwinner T113 (glibc) board configuration
#/

# MODULE_PATH
set(MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# MODULE_NAME
set(MODULE_NAME "t113_glibc")

# LIB_SRCS - collect all board *.c sources
file(GLOB_RECURSE LIB_SRCS "${MODULE_PATH}/*.c")

# Exclude the bundled toolchain tree (no board C sources live there, but the
# GLOB above is recursive, so guard against accidental matches).
list(FILTER LIB_SRCS EXCLUDE REGEX "toolchain-sunxi-glibc")
list(FILTER LIB_SRCS EXCLUDE REGEX "hello_test")

# LIB_PUBLIC_INC
set(LIB_PUBLIC_INC ${MODULE_PATH})


########################################
# Target Configure
########################################
add_library(${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        ${LIB_SRCS}
    )

target_include_directories(${MODULE_NAME}
    PRIVATE
        ${LIB_PRIVATE_INC}
        # tkl_vad.h / tkl_kws.h for the audio AI placeholder implementations.
        ${CMAKE_CURRENT_LIST_DIR}/../../../platform/LINUX/tuyaos_adapter/include

    PUBLIC
        ${LIB_PUBLIC_INC}
    )

# Link ALSA library only if ALSA support is enabled. The bundled sunxi
# toolchain sysroot does not ship libasound, so ALSA is disabled by default
# for this board. Provide libasound in the toolchain sysroot before enabling.
if (CONFIG_ENABLE_AUDIO_ALSA STREQUAL "y")
    target_link_libraries(${MODULE_NAME} PUBLIC asound)
    message(STATUS "T113 board: Linking ALSA library")
endif()


########################################
# Layer Configure
########################################
list(APPEND COMPONENT_LIBS ${MODULE_NAME})
set(COMPONENT_LIBS "${COMPONENT_LIBS}" PARENT_SCOPE)
list(APPEND COMPONENT_PUBINC ${LIB_PUBLIC_INC})
set(COMPONENT_PUBINC "${COMPONENT_PUBINC}" PARENT_SCOPE)
