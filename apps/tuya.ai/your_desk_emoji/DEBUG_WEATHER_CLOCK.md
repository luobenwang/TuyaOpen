# 天气时钟调试指南

## 问题描述
开机界面卡顿在白色，没有切换到天气时钟显示。

## 已添加的调试信息

### 1. 主程序调试 (tuya_main.c)
- 显示系统初始化状态
- 天气时钟消息发送状态
- 测试天气更新

### 2. 显示系统调试 (app_display.c)
- UI初始化各个步骤
- 消息接收和处理状态
- 天气时钟相关消息处理

### 3. 天气时钟UI调试 (ui_weather_clock.c)
- UI初始化过程
- 显示/隐藏操作
- 定时器更新状态
- 时间获取和格式化

## 可能的问题原因

### 1. UI冲突问题
- 表情轮播UI和天气时钟UI都在同一屏幕上创建
- 可能存在层级冲突

### 2. 初始化时序问题
- 天气时钟初始化后默认隐藏
- 显示消息可能在UI完全初始化前发送

### 3. 字体或显示问题
- 字体可能未正确加载
- 显示分辨率可能不匹配

## 调试步骤

### 1. 查看启动日志
运行程序后查看以下关键日志：
```
Starting UI initialization...
Font initialization completed
Main UI initialization completed
Weather clock UI initialization completed
UI initialization completed successfully
Waiting for display message...
Received display message type: [数字]
Showing weather clock
Weather clock show completed
```

### 2. 检查UI初始化
确认以下日志出现：
```
Initializing weather clock UI...
Display resolution: [宽度]x[高度]
Using [分辨率] layout
Initializing 160x80 weather clock layout...
Font configuration initialized
Screen style configured
Main container created
Update timer created successfully
Weather clock initially hidden
Weather clock UI initialized successfully for 160x80 display
```

### 3. 检查显示操作
确认以下日志出现：
```
Attempting to show weather clock...
Weather clock container exists, showing...
Weather clock container made visible and moved to foreground
Update timer resumed
Initial time update completed
Weather clock shown successfully
```

### 4. 检查时间更新
确认以下日志出现：
```
Updating time: [时间], date: [日期]
Updated date/weather: [日期]  ☀️ 22°C
```

## 故障排除

### 如果看到"Weather clock not initialized"
- 检查UI初始化是否成功
- 确认字体配置是否正确

### 如果看到"Update timer is NULL"
- 检查LVGL定时器创建是否成功
- 确认内存分配是否正常

### 如果看到"Time label is NULL"
- 检查UI组件创建是否成功
- 确认LVGL对象创建是否正常

### 如果没有任何天气时钟相关日志
- 检查ENABLE_CHAT_DISPLAY是否启用
- 确认显示系统是否正确初始化

## 测试建议

### 1. 手动测试
在代码中添加以下测试代码：
```c
// 在main函数中添加
PR_DEBUG("Testing weather clock manually...");
ui_weather_clock_show();
tal_system_sleep(5000);
ui_weather_clock_hide();
```

### 2. 简化测试
临时注释掉表情轮播UI初始化，只保留天气时钟：
```c
// 在app_display.c中注释掉
// TUYA_CALL_ERR_LOG(ui_init(&sg_display.ui_font));
```

### 3. 强制显示测试
在天气时钟初始化后立即显示：
```c
// 在ui_weather_clock_init函数末尾添加
ui_weather_clock_show();
```

## 下一步调试

1. 运行程序并收集完整日志
2. 根据日志确定问题所在
3. 如果UI冲突，考虑修改UI管理方式
4. 如果初始化问题，调整初始化时序
5. 如果显示问题，检查LVGL配置

## 联系信息
如果问题仍然存在，请提供完整的启动日志以便进一步分析。
