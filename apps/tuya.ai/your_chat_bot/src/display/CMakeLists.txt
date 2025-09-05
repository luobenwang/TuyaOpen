##
# @file CMakeLists.txt
# @brief 
#/
if (CONFIG_ENABLE_CHAT_DISPLAY STREQUAL "y")

set(APP_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

set(APP_MODULE_SRCS)
list(APPEND APP_MODULE_SRCS
        ${APP_MODULE_PATH}/app_display.c
        ${APP_MODULE_PATH}/tuya_lvgl.c
    )

aux_source_directory(${APP_MODULE_PATH}/font FONT_SRCS)
aux_source_directory(${APP_MODULE_PATH}/font/emoji EMOJI_SRCS)
aux_source_directory(${APP_MODULE_PATH}/ui UI_SRCS)
aux_source_directory(${APP_MODULE_PATH}/image/eyes128 IMAG_EYES_SRCS)
# Include all emotion images for better user experience
set(EMOTION_IMAGES_SRCS
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_happy_image.c
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_angry_image.c
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_sad_image.c
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_think_image.c
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_sleep_image.c
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_love_image.c
    ${APP_MODULE_PATH}/ui/emotion_images/emotion_shock_image.c
)

set(IMAGE_SRCS ${APP_MODULE_PATH}/image/TuyaOpen_img_320_480.c)
set(IMAGE_SRCS ${APP_MODULE_PATH}/image/_dino_alpha_48x48.c)

list(APPEND APP_MODULE_SRCS
    ${FONT_SRCS}
    ${EMOJI_SRCS}
    ${IMAGE_SRCS}
    ${UI_SRCS}
    ${IMAG_EYES_SRCS}
    ${EMOTION_IMAGES_SRCS}
    ${APP_MODULE_PATH}/ui/ui_2048.c
    ${APP_MODULE_PATH}/ui/ui_rgb_control.c
    ${APP_MODULE_PATH}/ui/ui_dino.c
)

# Add RGB LED driver (always enabled for now)
list(APPEND APP_MODULE_SRCS
    ${APP_MODULE_PATH}/rgb_led/rgb_led_spi.c
)

set(APP_MODULE_INC 
    ${APP_MODULE_PATH}
    ${APP_MODULE_PATH}/font
    ${APP_MODULE_PATH}/ui
)

# Add RGB LED include path (always enabled for now)
list(APPEND APP_MODULE_INC ${APP_MODULE_PATH}/rgb_led)

########################################
# Target Configure
########################################
target_sources(${EXAMPLE_LIB}
    PRIVATE
        ${APP_MODULE_SRCS}
    )

target_include_directories(${EXAMPLE_LIB}
    PRIVATE
        ${APP_MODULE_INC}
    )

endif()