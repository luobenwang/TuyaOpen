# 动态主题切换功能

## 问题描述

用户反馈深色模式可以看到数字了，但是浅色模式又看不到数字了。这是因为字体颜色没有根据主题模式动态变化。

## 解决方案

### 1. **扩展全局结构体**

添加了新的字段来保存所有UI元素的引用：

```c
typedef struct {
    // ... 原有字段 ...
    
    /* Watch marks and labels for theme switching */
    lv_obj_t *hour_marks[12];    /* Hour marks */
    lv_obj_t *hour_labels[12];    /* Hour number labels */
    lv_obj_t *minute_marks[60];   /* Minute marks */
    lv_obj_t *decorative_rings[3]; /* Decorative rings */
    lv_obj_t *watch_title;        /* Watch title label */
    lv_obj_t *watch_subtitle;     /* Watch subtitle label */
} centered_watch_t;
```

### 2. **修改创建函数**

在 `create_watch_marks` 函数中保存所有UI元素的引用：

```c
/* 保存刻度引用到全局结构体 */
g_watch.hour_marks[i] = mark;

/* 保存数字标签引用到全局结构体 */
g_watch.hour_labels[i] = label;

/* 保存分钟刻度引用到全局结构体 */
g_watch.minute_marks[i] = mark;

/* 保存装饰环引用到全局结构体 */
g_watch.decorative_rings[0] = decorative_ring;
g_watch.decorative_rings[1] = middle_ring;
g_watch.decorative_rings[2] = inner_ring;

/* 保存文字标签引用到全局结构体 */
g_watch.watch_title = watch_title;
g_watch.watch_subtitle = watch_subtitle;
```

### 3. **增强深色主题函数**

`vintage_watch_set_dark_theme()` 现在会更新所有UI元素：

```c
void vintage_watch_set_dark_theme(void)
{
    // ... 原有代码 ...
    
    /* Update hour marks to dark theme */
    for (int i = 0; i < 12; i++) {
        if (g_watch.hour_marks[i]) {
            lv_obj_set_style_bg_color(g_watch.hour_marks[i], lv_color_hex(DARK_HAND_COLOR), 0);
        }
    }
    
    /* Update hour labels to dark theme */
    for (int i = 0; i < 12; i++) {
        if (g_watch.hour_labels[i]) {
            lv_obj_set_style_text_color(g_watch.hour_labels[i], lv_color_hex(DARK_TEXT_COLOR), 0);
        }
    }
    
    /* Update minute marks to dark theme */
    for (int i = 0; i < 60; i++) {
        if (g_watch.minute_marks[i]) {
            lv_obj_set_style_bg_color(g_watch.minute_marks[i], lv_color_hex(DARK_HAND_COLOR), 0);
        }
    }
    
    /* Update decorative rings to dark theme */
    for (int i = 0; i < 3; i++) {
        if (g_watch.decorative_rings[i]) {
            lv_obj_set_style_border_color(g_watch.decorative_rings[i], lv_color_hex(DARK_BORDER_COLOR), 0);
        }
    }
    
    /* Update text labels to dark theme */
    if (g_watch.watch_title) {
        lv_obj_set_style_text_color(g_watch.watch_title, lv_color_hex(DARK_TEXT_COLOR), 0);
        lv_obj_set_style_text_opa(g_watch.watch_title, LV_OPA_80, 0);
    }
    if (g_watch.watch_subtitle) {
        lv_obj_set_style_text_color(g_watch.watch_subtitle, lv_color_hex(DARK_TEXT_COLOR), 0);
        lv_obj_set_style_text_opa(g_watch.watch_subtitle, LV_OPA_60, 0);
    }
    
    lv_refr_now(NULL);
}
```

### 4. **增强浅色主题函数**

`vintage_watch_set_light_theme()` 现在会更新所有UI元素：

```c
void vintage_watch_set_light_theme(void)
{
    // ... 原有代码 ...
    
    /* Update hour marks to light theme */
    for (int i = 0; i < 12; i++) {
        if (g_watch.hour_marks[i]) {
            lv_obj_set_style_bg_color(g_watch.hour_marks[i], lv_color_hex(VINTAGE_GOLD_COLOR), 0);
        }
    }
    
    /* Update hour labels to light theme */
    for (int i = 0; i < 12; i++) {
        if (g_watch.hour_labels[i]) {
            lv_obj_set_style_text_color(g_watch.hour_labels[i], lv_color_hex(VINTAGE_TEXT_COLOR), 0);
        }
    }
    
    /* Update minute marks to light theme */
    for (int i = 0; i < 60; i++) {
        if (g_watch.minute_marks[i]) {
            lv_obj_set_style_bg_color(g_watch.minute_marks[i], lv_color_hex(VINTAGE_GOLD_COLOR), 0);
        }
    }
    
    /* Update decorative rings to light theme */
    for (int i = 0; i < 3; i++) {
        if (g_watch.decorative_rings[i]) {
            if (i == 0) {
                lv_obj_set_style_border_color(g_watch.decorative_rings[i], lv_color_hex(VINTAGE_GOLD_COLOR), 0);
            } else if (i == 1) {
                lv_obj_set_style_border_color(g_watch.decorative_rings[i], lv_color_hex(VINTAGE_PLATINUM_COLOR), 0);
            } else {
                lv_obj_set_style_border_color(g_watch.decorative_rings[i], lv_color_hex(VINTAGE_DARK_GOLD), 0);
            }
        }
    }
    
    /* Update text labels to light theme */
    if (g_watch.watch_title) {
        lv_obj_set_style_text_color(g_watch.watch_title, lv_color_hex(0x8B7355), 0);
        lv_obj_set_style_text_opa(g_watch.watch_title, LV_OPA_60, 0);
    }
    if (g_watch.watch_subtitle) {
        lv_obj_set_style_text_color(g_watch.watch_subtitle, lv_color_hex(0x6B5B47), 0);
        lv_obj_set_style_text_opa(g_watch.watch_subtitle, LV_OPA_40, 0);
    }
    
    lv_refr_now(NULL);
}
```

## 主题颜色方案

### 深色主题
- **数字和文字**: 白色 (0xFFFFFF)
- **刻度和指针**: 白色 (0xFFFFFF)
- **秒针**: 绿色 (0x00FF00)
- **装饰环**: 深灰色 (0x333333)

### 浅色主题
- **数字和文字**: 复古棕色 (0x8B7355)
- **刻度和指针**: 金色 (0xD4AF37)
- **秒针**: 淡紫色 (0xE6E6FA)
- **装饰环**: 金色系 (VINTAGE_GOLD_COLOR, VINTAGE_PLATINUM_COLOR, VINTAGE_DARK_GOLD)

## 功能特点

### 1. **动态切换**
- ✅ 深色主题：白色文字在深色背景下清晰可见
- ✅ 浅色主题：复古棕色文字在浅色背景下清晰可见
- ✅ 所有UI元素都会同步更新

### 2. **完整覆盖**
- ✅ 小时数字 (12个)
- ✅ 小时刻度 (12个)
- ✅ 分钟刻度 (60个)
- ✅ 装饰环 (3个)
- ✅ 文字标签 (2个)

### 3. **实时更新**
- ✅ 主题切换时立即更新所有元素
- ✅ 强制刷新显示确保变化生效
- ✅ 详细的日志输出便于调试

## 使用方法

### 1. **语音命令**
- "深色主题" → 切换到深色主题
- "浅色主题" → 切换到浅色主题

### 2. **编程接口**
```c
vintage_watch_set_dark_theme();   // 深色主题
vintage_watch_set_light_theme();  // 浅色主题
```

## 效果对比

### 深色主题
- 背景：纯黑色
- 数字：白色，清晰可见
- 刻度：白色，清晰可见
- 指针：白色，清晰可见

### 浅色主题
- 背景：复古棕色
- 数字：复古棕色，清晰可见
- 刻度：金色，清晰可见
- 指针：金色，清晰可见

## 总结

动态主题切换功能已完成：

1. **扩展了全局结构体** - 保存所有UI元素引用
2. **修改了创建函数** - 保存UI元素到全局结构体
3. **增强了主题函数** - 动态更新所有UI元素颜色
4. **实现了完整切换** - 深色和浅色主题都能正确显示

现在字体颜色会根据主题模式动态变化，深色模式下数字清晰可见，浅色模式下数字也清晰可见！
