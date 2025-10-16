/**
 * @file watch_style_integration.c
 * @brief Integration of watch styles with chat bot functionality
 * 
 * This file shows how to integrate the different watch face styles
 * with the existing chat bot system.
 */

#include "watch_style_demo.h"
#include "ui_wechat.h"
#include "watch_style_integration.h"
#include "vintage_watch_app.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**********************
 *  STATIC VARIABLES
 **********************/
static bool integration_initialized = false;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize watch style integration
 */
void watch_style_integration_init(void)
{
    if (!integration_initialized) {
        watch_style_demo_init();
        integration_initialized = true;
        printf("Watch style integration initialized\n");
    }
}

/**
 * @brief Handle style change commands from user input
 * @param text User input text
 */
void handle_style_change_commands(const char *text)
{
    if (!text) return;
    
    /* Check for anime style commands */
    if (strstr(text, "动漫") != NULL || strstr(text, "anime") != NULL || 
        strstr(text, "卡通") != NULL || strstr(text, "可爱") != NULL) {
        switch_to_anime_style();
        ui_set_user_msg("已切换到动漫风格表盘");
        return;
    }
    
    /* Check for fashion style commands */
    if (strstr(text, "时尚") != NULL || strstr(text, "fashion") != NULL || 
        strstr(text, "优雅") != NULL || strstr(text, "奢华") != NULL) {
        switch_to_fashion_style();
        ui_set_user_msg("已切换到时尚风格表盘");
        return;
    }
    
    /* Check for tech style commands */
    if (strstr(text, "科技") != NULL || strstr(text, "tech") != NULL || 
        strstr(text, "未来") != NULL || strstr(text, "数字") != NULL) {
        switch_to_tech_style();
        ui_set_user_msg("已切换到科技风格表盘");
        return;
    }
    
    /* Check for forest style commands */
    if (strstr(text, "森林") != NULL || strstr(text, "forest") != NULL || 
        strstr(text, "自然") != NULL || strstr(text, "绿色") != NULL) {
        switch_to_forest_style();
        ui_set_user_msg("已切换到森林风格表盘");
        return;
    }
    
    /* Check for style cycle commands */
    if (strstr(text, "切换表盘") != NULL || strstr(text, "换表盘") != NULL || 
        strstr(text, "下一个表盘") != NULL) {
        cycle_watch_styles();
        ui_set_user_msg("已切换到下一个表盘样式");
        return;
    }
    
    /* Check for style info commands */
    if (strstr(text, "表盘信息") != NULL || strstr(text, "当前表盘") != NULL || 
        strstr(text, "表盘样式") != NULL) {
        print_style_info();
        ui_set_user_msg("当前表盘样式信息已显示");
        return;
    }
}

/**
 * @brief Get style-specific response for historical events
 * @param year The historical year
 * @param event The historical event
 * @return Style-appropriate response
 */
const char* get_style_specific_response(const char *year, const char *event)
{
    custom_watch_style_t current_style = custom_watch_get_style();
    
    switch (current_style) {
        case ANIME_STYLE:
            return "哇！这个历史事件好有趣呢！✨";
        case FASHION_STYLE:
            return "这是一个优雅的历史时刻，值得铭记。";
        case TECH_STYLE:
            return "历史数据已加载，事件分析完成。";
        case FOREST_STYLE:
            return "这是大自然见证的历史时刻。";
        default:
            return "历史事件已记录。";
    }
}

/**
 * @brief Enhanced user message handler with style integration
 * @param text User input text
 */
void enhanced_ui_set_user_msg(const char *text)
{
    /* First handle style change commands */
    handle_style_change_commands(text);
    
    /* Then handle historical year detection */
    if (text) {
        if (strstr(text, "一九四九") != NULL || strstr(text, "1949") != NULL) {
            printf("Detected 1949 in user message, showing on watch\n");
            vintage_watch_show_text("1949 中华人民共和国成立");
            ui_set_user_msg(get_style_specific_response("1949", "中华人民共和国成立"));
        }
        else if (strstr(text, "一九五零") != NULL || strstr(text, "1950") != NULL) {
            printf("Detected 1950 in user message, showing on watch\n");
            vintage_watch_show_text("1950 抗美援朝战争开始");
            ui_set_user_msg(get_style_specific_response("1950", "抗美援朝战争开始"));
        }
        else if (strstr(text, "一九七八") != NULL || strstr(text, "1978") != NULL) {
            printf("Detected 1978 in user message, showing on watch\n");
            vintage_watch_show_text("1978 改革开放开始");
            ui_set_user_msg(get_style_specific_response("1978", "改革开放开始"));
        }
        else if (strstr(text, "一九九七") != NULL || strstr(text, "1997") != NULL) {
            printf("Detected 1997 in user message, showing on watch\n");
            vintage_watch_show_text("1997 香港回归");
            ui_set_user_msg(get_style_specific_response("1997", "香港回归"));
        }
        else if (strstr(text, "二零零八") != NULL || strstr(text, "2008") != NULL) {
            printf("Detected 2008 in user message, showing on watch\n");
            vintage_watch_show_text("2008 北京奥运会");
            ui_set_user_msg(get_style_specific_response("2008", "北京奥运会"));
        }
        else if (strstr(text, "中国") != NULL) {
            printf("Detected '中国' in user message, showing on watch\n");
            vintage_watch_show_text("1949 新中国");
            ui_set_user_msg(get_style_specific_response("1949", "新中国"));
        }
    }
}
