/**
 * @file watch_style_usage_example.c
 * @brief Simple usage example for watch styles
 * 
 * This file shows how to integrate the watch styles with the existing system
 * without complex dependencies.
 */

#include "vintage_watch_app.h"
#include <stdio.h>
#include <string.h>

/**********************
 *  STATIC VARIABLES
 **********************/
static int current_style = 0; // 0=Anime, 1=Fashion, 2=Tech, 3=Forest

/**********************
 *  STATIC FUNCTIONS
 **********************/

/**
 * @brief Apply anime style colors and settings
 */
static void apply_anime_style(void)
{
    printf("Applying Anime Style:\n");
    printf("- Background: Hot Pink (#FF69B4)\n");
    printf("- Face: Misty Rose (#FFE4E1)\n");
    printf("- Hands: Tomato (#FF6347)\n");
    printf("- Accent: Deep Pink (#FF1493)\n");
    printf("- Features: Sparkle decorations, bright colors\n");
}

/**
 * @brief Apply fashion style colors and settings
 */
static void apply_fashion_style(void)
{
    printf("Applying Fashion Style:\n");
    printf("- Background: Dark Gray (#2F2F2F)\n");
    printf("- Face: Beige (#F5F5DC)\n");
    printf("- Hands: Saddle Brown (#8B4513)\n");
    printf("- Accent: Goldenrod (#DAA520)\n");
    printf("- Features: Luxury hour markers, elegant design\n");
}

/**
 * @brief Apply tech style colors and settings
 */
static void apply_tech_style(void)
{
    printf("Applying Tech Style:\n");
    printf("- Background: Black (#000000)\n");
    printf("- Face: Dark Blue (#001122)\n");
    printf("- Hands: Green (#00FF00)\n");
    printf("- Accent: Cyan (#00FFFF)\n");
    printf("- Features: Digital-style markers, neon effects\n");
}

/**
 * @brief Apply forest style colors and settings
 */
static void apply_forest_style(void)
{
    printf("Applying Forest Style:\n");
    printf("- Background: Sea Green (#2E8B57)\n");
    printf("- Face: Honeydew (#F0FFF0)\n");
    printf("- Hands: Saddle Brown (#8B4513)\n");
    printf("- Accent: Forest Green (#228B22)\n");
    printf("- Features: Leaf decorations, earth tones\n");
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * @brief Initialize watch style system
 */
void watch_style_system_init(void)
{
    printf("Watch Style System Initialized\n");
    printf("Available styles: Anime, Fashion, Tech, Forest\n");
    current_style = 0; // Start with anime style
    apply_anime_style();
}

/**
 * @brief Switch to next watch style
 */
void switch_to_next_style(void)
{
    current_style = (current_style + 1) % 4;
    
    switch (current_style) {
        case 0:
            apply_anime_style();
            break;
        case 1:
            apply_fashion_style();
            break;
        case 2:
            apply_tech_style();
            break;
        case 3:
            apply_forest_style();
            break;
    }
}

/**
 * @brief Switch to specific watch style
 * @param style Style number (0-3)
 */
void switch_to_style(int style)
{
    if (style >= 0 && style <= 3) {
        current_style = style;
        
        switch (current_style) {
            case 0:
                apply_anime_style();
                break;
            case 1:
                apply_fashion_style();
                break;
            case 2:
                apply_tech_style();
                break;
            case 3:
                apply_forest_style();
                break;
        }
    }
}

/**
 * @brief Get current style name
 * @return Style name string
 */
const char* get_current_style_name(void)
{
    switch (current_style) {
        case 0: return "Anime Style";
        case 1: return "Fashion Style";
        case 2: return "Tech Style";
        case 3: return "Forest Style";
        default: return "Unknown Style";
    }
}

/**
 * @brief Handle style change commands from user input
 * @param text User input text
 * @return 1 if style was changed, 0 otherwise
 */
int handle_style_commands(const char *text)
{
    if (!text) return 0;
    
    /* Check for anime style commands */
    if (strstr(text, "动漫") != NULL || strstr(text, "anime") != NULL || 
        strstr(text, "卡通") != NULL || strstr(text, "可爱") != NULL) {
        switch_to_style(0);
        printf("Switched to Anime Style\n");
        return 1;
    }
    
    /* Check for fashion style commands */
    if (strstr(text, "时尚") != NULL || strstr(text, "fashion") != NULL || 
        strstr(text, "优雅") != NULL || strstr(text, "奢华") != NULL) {
        switch_to_style(1);
        printf("Switched to Fashion Style\n");
        return 1;
    }
    
    /* Check for tech style commands */
    if (strstr(text, "科技") != NULL || strstr(text, "tech") != NULL || 
        strstr(text, "未来") != NULL || strstr(text, "数字") != NULL) {
        switch_to_style(2);
        printf("Switched to Tech Style\n");
        return 1;
    }
    
    /* Check for forest style commands */
    if (strstr(text, "森林") != NULL || strstr(text, "forest") != NULL || 
        strstr(text, "自然") != NULL || strstr(text, "绿色") != NULL) {
        switch_to_style(3);
        printf("Switched to Forest Style\n");
        return 1;
    }
    
    /* Check for next style commands */
    if (strstr(text, "切换表盘") != NULL || strstr(text, "换表盘") != NULL || 
        strstr(text, "下一个表盘") != NULL) {
        switch_to_next_style();
        printf("Switched to next style: %s\n", get_current_style_name());
        return 1;
    }
    
    return 0;
}

/**
 * @brief Get style-specific response for historical events
 * @param year The historical year
 * @param event The historical event
 * @return Style-appropriate response
 */
const char* get_style_response(const char *year, const char *event)
{
    switch (current_style) {
        case 0: // Anime
            return "哇！这个历史事件好有趣呢！✨";
        case 1: // Fashion
            return "这是一个优雅的历史时刻，值得铭记。";
        case 2: // Tech
            return "历史数据已加载，事件分析完成。";
        case 3: // Forest
            return "这是大自然见证的历史时刻。";
        default:
            return "历史事件已记录。";
    }
}

/**
 * @brief Print current style information
 */
void print_style_info(void)
{
    printf("Current watch style: %s\n", get_current_style_name());
    printf("Available styles:\n");
    printf("1. Anime Style - Bright, colorful with anime-inspired design\n");
    printf("2. Fashion Style - Elegant, modern with luxury elements\n");
    printf("3. Tech Style - Futuristic, digital with neon accents\n");
    printf("4. Forest Style - Natural, organic with earth tones\n");
    printf("Use voice commands like '动漫', '时尚', '科技', '森林' to switch styles\n");
}
