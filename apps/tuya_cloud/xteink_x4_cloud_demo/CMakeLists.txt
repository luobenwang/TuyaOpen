##
# @file CMakeLists.txt
# @brief XTEINK X4 cloud switch demo (Tuya IoT + optional LVGL EPD UI)
#/

if (NOT CONFIG_BOARD_CHOICE_XTEINK_X4 STREQUAL "y")
    message(FATAL_ERROR "xteink_x4_cloud_demo requires BOARD_CHOICE_XTEINK_X4")
endif()

# Feature switches are defined in src/tuya_config.h:
#   XTEINK_X4_ENABLE_CLOUD   0/1
#   XTEINK_X4_ENABLE_DISPLAY 0/1
#   XTEINK_X4_ENABLE_LAN     0/1 (0 saves heap for MQTT TLS)
set(_X4_CFG_H "${CMAKE_CURRENT_LIST_DIR}/src/tuya_config.h")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${_X4_CFG_H}")
file(READ "${_X4_CFG_H}" _X4_CFG_CONTENT)

string(REGEX MATCH "#define[ \t]+XTEINK_X4_ENABLE_CLOUD[ \t]+([01])" _ ${_X4_CFG_CONTENT})
if (NOT CMAKE_MATCH_1 STREQUAL "")
    set(XTEINK_X4_ENABLE_CLOUD ${CMAKE_MATCH_1})
else()
    set(XTEINK_X4_ENABLE_CLOUD 1)
endif()

string(REGEX MATCH "#define[ \t]+XTEINK_X4_ENABLE_DISPLAY[ \t]+([01])" _ ${_X4_CFG_CONTENT})
if (NOT CMAKE_MATCH_1 STREQUAL "")
    set(XTEINK_X4_ENABLE_DISPLAY ${CMAKE_MATCH_1})
else()
    set(XTEINK_X4_ENABLE_DISPLAY 0)
endif()

string(REGEX MATCH "#define[ \t]+XTEINK_X4_ENABLE_LAN[ \t]+([01])" _ ${_X4_CFG_CONTENT})
if (NOT CMAKE_MATCH_1 STREQUAL "")
    set(XTEINK_X4_ENABLE_LAN ${CMAKE_MATCH_1})
else()
    set(XTEINK_X4_ENABLE_LAN 0)
endif()
# Propagate to tuya_cloud_service (netmgr / tuya_lan) — must be visible before that
# component is configured; also set CACHE so late app cmake can still document it.
set(ENABLE_TUYA_LAN ${XTEINK_X4_ENABLE_LAN} CACHE STRING "Enable Tuya LAN local control" FORCE)

set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})
get_filename_component(APP_NAME ${APP_PATH} NAME)

set(APP_SRCS ${APP_PATH}/src/tuya_main.c)
set(APP_OPTIONS "-Wall")

if (XTEINK_X4_ENABLE_DISPLAY STREQUAL "1")
    list(APPEND APP_SRCS
        ${APP_PATH}/src/xteink_x4_display.c
        ${APP_PATH}/src/x4_gfx.c
    )
endif()

if (XTEINK_X4_ENABLE_CLOUD STREQUAL "1")
    list(APPEND APP_SRCS
        ${APP_PATH}/src/reset_netcfg.c
        ${APP_PATH}/src/cli_cmd.c
    )
endif()

if (XTEINK_X4_ENABLE_DISPLAY STREQUAL "1" AND XTEINK_X4_ENABLE_CLOUD STREQUAL "1")
    message(STATUS "xteink_x4_cloud_demo: display + cloud (from tuya_config.h)")
elseif (XTEINK_X4_ENABLE_DISPLAY STREQUAL "1")
    message(STATUS "xteink_x4_cloud_demo: display-only (from tuya_config.h)")
elseif (XTEINK_X4_ENABLE_CLOUD STREQUAL "1")
    message(STATUS "xteink_x4_cloud_demo: cloud-only (from tuya_config.h)")
else()
    message(FATAL_ERROR "xteink_x4_cloud_demo: set XTEINK_X4_ENABLE_CLOUD and/or XTEINK_X4_ENABLE_DISPLAY to 1 in tuya_config.h")
endif()

if (XTEINK_X4_ENABLE_LAN STREQUAL "1")
    message(STATUS "xteink_x4_cloud_demo: Tuya LAN enabled")
else()
    message(STATUS "xteink_x4_cloud_demo: Tuya LAN disabled (XTEINK_X4_ENABLE_LAN=0)")
endif()

# Ensure LAN switch reaches this app target even if component cmake ran earlier.
list(APPEND APP_OPTIONS "-DENABLE_TUYA_LAN=${XTEINK_X4_ENABLE_LAN}")

set(APP_INC ${APP_PATH}/src)

add_library(${EXAMPLE_LIB})

target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_SRCS}
    )

target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_INC}
    )

target_compile_options(${EXAMPLE_LIB}
    PRIVATE
        ${APP_OPTIONS}
    )
