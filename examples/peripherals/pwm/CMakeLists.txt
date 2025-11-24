##
# @file CMakeLists.txt
# @brief 
#/

if (CONFIG_ENABLE_PWM STREQUAL "y")

# APP_PATH
set(APP_PATH ${CMAKE_CURRENT_LIST_DIR})

# APP_NAME
get_filename_component(APP_NAME ${APP_PATH} NAME)

# APP_SRCS
aux_source_directory(${APP_PATH}/src APP_SRCS)

# Add c_port source files
list(APPEND APP_SRCS
    ${APP_PATH}/src/c_port/otto_ninja/otto_ninja_app_servo.c
)

########################################
# Target Configure
########################################
add_library(${EXAMPLE_LIB})

target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_SRCS}
    )

# Add include directories for c_port
target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_PATH}/src/c_port/otto_ninja
)
else()
message(FATAL_ERROR "pwm cannot work when CONFIG_ENABLE_PWM is not set")
endif()