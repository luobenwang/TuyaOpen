##
# @file CMakeLists.txt
# @brief XTEINK X4 cloud switch demo (Tuya IoT + optional LVGL EPD UI)
#/

if (NOT CONFIG_BOARD_CHOICE_XTEINK_X4 STREQUAL "y")
    message(FATAL_ERROR "xteink_x4_cloud_demo requires BOARD_CHOICE_XTEINK_X4")
endif()

# Build mode (pick one):
#   LVGL only:  ENABLE_LVGL=ON,  ENABLE_CLOUD=OFF
#   Cloud only: ENABLE_LVGL=OFF, ENABLE_CLOUD=ON
#   Both:       ENABLE_LVGL=ON,  ENABLE_CLOUD=ON
set(XTEINK_X4_ENABLE_LVGL OFF)
set(XTEINK_X4_ENABLE_CLOUD ON)

set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})
get_filename_component(APP_NAME ${APP_PATH} NAME)

set(APP_SRCS ${APP_PATH}/src/tuya_main.c)
set(APP_OPTIONS "-Wall")

if (XTEINK_X4_ENABLE_LVGL)
    list(APPEND APP_SRCS ${APP_PATH}/src/xteink_x4_display.c)
    list(APPEND APP_OPTIONS "-DLV_LVGL_H_INCLUDE_SIMPLE")
endif()

if (XTEINK_X4_ENABLE_CLOUD)
    list(APPEND APP_SRCS
        ${APP_PATH}/src/reset_netcfg.c
        ${APP_PATH}/src/cli_cmd.c
    )
endif()

if (XTEINK_X4_ENABLE_LVGL AND XTEINK_X4_ENABLE_CLOUD)
    message(STATUS "xteink_x4_cloud_demo: LVGL + cloud")
elseif (XTEINK_X4_ENABLE_LVGL)
    message(STATUS "xteink_x4_cloud_demo: LVGL-only mode (cloud disabled)")
elseif (XTEINK_X4_ENABLE_CLOUD)
    message(STATUS "xteink_x4_cloud_demo: cloud-only mode (LVGL disabled)")
else()
    message(FATAL_ERROR "xteink_x4_cloud_demo: enable LVGL and/or cloud")
endif()

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

if (XTEINK_X4_ENABLE_LVGL)
    set(_DISABLE_LVGL 0)
else()
    set(_DISABLE_LVGL 1)
endif()

if (XTEINK_X4_ENABLE_CLOUD)
    set(_DISABLE_CLOUD 0)
else()
    set(_DISABLE_CLOUD 1)
endif()

target_compile_definitions(${EXAMPLE_LIB}
    PRIVATE
        XTEINK_X4_DISABLE_LVGL=${_DISABLE_LVGL}
        XTEINK_X4_DISABLE_CLOUD=${_DISABLE_CLOUD}
    )
