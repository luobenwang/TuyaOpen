/**
 * @file keyword_trigger_test.c
 * @brief Test file for keyword-triggered watch style switching
 * 
 * This file demonstrates how the keyword triggering works
 * for watch style switching.
 */

#include "watch_style_usage_example.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Test function to simulate ui_set_user_msg behavior
 * @param text User input text
 */
void test_ui_set_user_msg(const char *text)
{
    printf("=== Testing ui_set_user_msg with: '%s' ===\n", text);
    
    /* Initialize watch style system if not already done */
    static bool watch_style_initialized = false;
    if (!watch_style_initialized) {
        watch_style_system_init();
        watch_style_initialized = true;
        printf("Watch style system initialized\n");
    }
    
    /* Check for watch style keywords first */
    if (text) {
        /* Check for anime style keywords */
        if (strstr(text, "动漫") != NULL || strstr(text, "anime") != NULL || 
            strstr(text, "卡通") != NULL || strstr(text, "可爱") != NULL) {
            printf("Detected anime keywords, switching to anime style\n");
            switch_to_style(0); // Switch to anime style
            printf("Watch style changed to: %s\n", get_current_style_name());
            return;
        }
        
        /* Check for fashion style keywords */
        if (strstr(text, "时尚") != NULL || strstr(text, "fashion") != NULL || 
            strstr(text, "优雅") != NULL || strstr(text, "奢华") != NULL) {
            printf("Detected fashion keywords, switching to fashion style\n");
            switch_to_style(1); // Switch to fashion style
            printf("Watch style changed to: %s\n", get_current_style_name());
            return;
        }
        
        /* Check for tech style keywords */
        if (strstr(text, "科技") != NULL || strstr(text, "tech") != NULL || 
            strstr(text, "未来") != NULL || strstr(text, "数字") != NULL) {
            printf("Detected tech keywords, switching to tech style\n");
            switch_to_style(2); // Switch to tech style
            printf("Watch style changed to: %s\n", get_current_style_name());
            return;
        }
        
        /* Check for forest style keywords */
        if (strstr(text, "森林") != NULL || strstr(text, "forest") != NULL || 
            strstr(text, "自然") != NULL || strstr(text, "绿色") != NULL) {
            printf("Detected forest keywords, switching to forest style\n");
            switch_to_style(3); // Switch to forest style
            printf("Watch style changed to: %s\n", get_current_style_name());
            return;
        }
        
        /* Check for general style change keywords */
        if (strstr(text, "切换表盘") != NULL || strstr(text, "换表盘") != NULL || 
            strstr(text, "下一个表盘") != NULL) {
            printf("Detected style change keywords, switching to next style\n");
            switch_to_next_style();
            printf("Watch style changed to: %s\n", get_current_style_name());
            return;
        }
    }
    
    printf("No style keywords detected, continuing with other processing...\n");
}

/**
 * @brief Main test function
 */
int main(void)
{
    printf("=== Keyword Trigger Test for Watch Style Switching ===\n\n");
    
    /* Test anime keywords */
    test_ui_set_user_msg("动漫");
    printf("\n");
    
    test_ui_set_user_msg("anime");
    printf("\n");
    
    test_ui_set_user_msg("可爱");
    printf("\n");
    
    /* Test fashion keywords */
    test_ui_set_user_msg("时尚");
    printf("\n");
    
    test_ui_set_user_msg("fashion");
    printf("\n");
    
    test_ui_set_user_msg("优雅");
    printf("\n");
    
    /* Test tech keywords */
    test_ui_set_user_msg("科技");
    printf("\n");
    
    test_ui_set_user_msg("tech");
    printf("\n");
    
    test_ui_set_user_msg("未来");
    printf("\n");
    
    /* Test forest keywords */
    test_ui_set_user_msg("森林");
    printf("\n");
    
    test_ui_set_user_msg("forest");
    printf("\n");
    
    test_ui_set_user_msg("自然");
    printf("\n");
    
    /* Test general switch keywords */
    test_ui_set_user_msg("切换表盘");
    printf("\n");
    
    test_ui_set_user_msg("换表盘");
    printf("\n");
    
    /* Test non-keyword input */
    test_ui_set_user_msg("一九四九年");
    printf("\n");
    
    test_ui_set_user_msg("你好");
    printf("\n");
    
    printf("=== Test Complete ===\n");
    return 0;
}
