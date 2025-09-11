# 天气时钟调试计划

## 当前状态
✅ 系统稳定，表情轮播正常工作  
❌ 天气时钟功能被禁用（安全模式）

## 调试策略：逐步启用

### 阶段1：基础UI初始化测试
目标：验证天气时钟UI初始化是否安全

**修改步骤：**
1. 在 `app_display.c` 中启用天气时钟UI初始化
2. 保持消息处理禁用
3. 保持主程序调用禁用

**修改代码：**
```c
// 在 app_display.c 中
// Initialize weather clock UI (temporarily disabled for safety)
#if 1  // 改为 1
TUYA_CALL_ERR_LOG(ui_weather_clock_init(&sg_display.ui_font));
PR_DEBUG("Weather clock UI initialization completed");
#else
PR_DEBUG("Weather clock UI initialization skipped (safety mode)");
#endif
```

**预期结果：**
- 系统正常启动
- 看到 "Weather clock UI initialization completed" 日志
- 表情轮播仍然正常工作

### 阶段2：消息处理测试
目标：验证天气时钟消息处理是否安全

**修改步骤：**
1. 保持UI初始化启用
2. 启用消息处理（但只显示日志，不执行实际功能）
3. 保持主程序调用禁用

**修改代码：**
```c
// 在 app_display.c 中
case TY_DISPLAY_TP_WEATHER_CLOCK_SHOW: {
    PR_DEBUG("Weather clock show requested - UI initialized: %s", 
             sg_weather_clock.ui.container ? "YES" : "NO");
    // ui_weather_clock_show(); // 仍然禁用
} break;
```

### 阶段3：显示功能测试
目标：验证天气时钟显示功能

**修改步骤：**
1. 保持UI初始化和消息处理启用
2. 启用显示功能
3. 保持主程序调用禁用

**修改代码：**
```c
case TY_DISPLAY_TP_WEATHER_CLOCK_SHOW: {
    PR_DEBUG("Showing weather clock");
    ui_weather_clock_show();
    PR_DEBUG("Weather clock show completed");
} break;
```

### 阶段4：主程序集成测试
目标：验证完整的天气时钟功能

**修改步骤：**
1. 启用所有功能
2. 在主程序中发送显示消息

**修改代码：**
```c
// 在 tuya_main.c 中取消注释
// Show weather clock on startup
PR_DEBUG("Sending weather clock show message...");
app_display_send_msg(TY_DISPLAY_TP_WEATHER_CLOCK_SHOW, NULL, 0);
PR_DEBUG("Weather clock show message sent");
```

## 调试检查点

### 每个阶段需要检查的日志：

**阶段1检查：**
- "Initializing weather clock UI..."
- "Display resolution: [宽度]x[高度]"
- "Using [分辨率] layout"
- "Initializing 160x80 weather clock layout..."
- "Font configuration initialized"
- "Screen style configured"
- "Main container created"
- "Time label created successfully"
- "Date weather label created successfully"
- "Update timer created successfully"
- "Weather clock initially hidden"
- "Weather clock UI initialized successfully for 160x80 display"

**阶段2检查：**
- "Weather clock show requested - UI initialized: YES"
- "Weather clock hide requested (disabled in safety mode)"
- "Weather update requested (disabled in safety mode): [数据]"

**阶段3检查：**
- "Attempting to show weather clock..."
- "Weather clock container exists, showing..."
- "Weather clock container made visible and moved to foreground"
- "Update timer resumed"
- "Initial time update completed"
- "Weather clock shown successfully"

**阶段4检查：**
- "Sending weather clock show message..."
- "Weather clock show message sent"
- 所有阶段3的日志

## 故障排除

### 如果阶段1失败：
- 检查字体配置是否正确
- 检查LVGL对象创建是否成功
- 检查内存分配是否正常

### 如果阶段2失败：
- 检查消息队列是否正常工作
- 检查消息类型是否正确

### 如果阶段3失败：
- 检查UI组件是否创建成功
- 检查定时器是否正常工作
- 检查时间获取是否正常

### 如果阶段4失败：
- 检查UI冲突问题
- 检查显示层级问题
- 检查手势切换逻辑

## 安全回退

如果任何阶段出现问题，可以立即回退到安全模式：

1. 将 `#if 1` 改回 `#if 0`
2. 注释掉相关功能调用
3. 系统会回到稳定状态

## 下一步行动

建议从阶段1开始，逐步测试每个功能。每次只启用一个功能，确认稳定后再进行下一步。

这样可以精确定位问题所在，避免系统崩溃。
