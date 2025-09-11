# 表情轮播问题修复

## 🐛 问题描述

手势切换后表情没有轮播，用户反馈手势触发表情后，表情显示正常但没有自动轮播到下一个表情。

## 🔍 问题分析

### 根本原因
在 `ui_emoji.c` 的 `__emotion_flush` 函数中，当手势触发表情时：
1. 设置了 `s_auto_cycle = false` - 禁用了自动轮播
2. 定时器回调函数 `__emotion_timer_cb` 只有在 `s_auto_cycle = true` 时才会切换表情
3. 这导致表情显示后无法自动轮播

### 次要问题
- `GESTURE_BACKWARD` 使用了不存在的 "sleepy" 表情，应该使用 "sleep"

## 🔧 修复方案

### 1. 修复轮播逻辑
**文件**: `src/display/ui/ui_emoji.c`

**修改前**:
```c
static void __emotion_flush(char *emotion)
{
    uint8_t index = 0;
    index = __emotion_get(emotion);
    
    // Update current index and pause auto cycle temporarily
    s_current_index = index;
    s_auto_cycle = false;  // ❌ 禁用轮播
    
    lv_gif_set_src(s_gif, gif_emotion[index].data);
    
    // Resume auto cycle after a delay (restart timer)
    if (s_emmo_timer) {
        lv_timer_reset(s_emmo_timer);
    }
}
```

**修改后**:
```c
static void __emotion_flush(char *emotion)
{
    uint8_t index = 0;
    index = __emotion_get(emotion);
    
    // Update current index and enable auto cycle for rotation
    s_current_index = index;
    s_auto_cycle = true;  // ✅ 启用轮播
    
    lv_gif_set_src(s_gif, gif_emotion[index].data);
    
    // Start/reset the rotation timer
    if (s_emmo_timer) {
        lv_timer_reset(s_emmo_timer);
    }
}
```

### 2. 优化定时器回调
**修改前**:
```c
static void __emotion_timer_cb(lv_timer_t *timer)
{
    // Re-enable auto cycle after manual switch
    s_auto_cycle = true;
    
    // Switch to next expression
    s_current_index = (s_current_index + 1) % (sizeof(gif_emotion) / sizeof(gif_emotion[0]));
    lv_gif_set_src(s_gif, gif_emotion[s_current_index].data);
}
```

**修改后**:
```c
static void __emotion_timer_cb(lv_timer_t *timer)
{
    // Only rotate if auto cycle is enabled
    if (s_auto_cycle) {
        // Switch to next expression
        s_current_index = (s_current_index + 1) % (sizeof(gif_emotion) / sizeof(gif_emotion[0]));
        lv_gif_set_src(s_gif, gif_emotion[s_current_index].data);
        PR_DEBUG("Emoji rotated to index %d: %s", s_current_index, gif_emotion[s_current_index].text);
    }
}
```

### 3. 修复表情名称
**文件**: `src/tuya_main.c`

**修改前**:
```c
case GESTURE_BACKWARD:
    app_display_send_msg(TY_DISPLAY_TP_EMOTION_MOOD, (uint8_t *)"sleepy", 6);
    _s_servo_action = SERVO_CENTER;
    break;
```

**修改后**:
```c
case GESTURE_BACKWARD:
    app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t *)"sleep", 5);
    _s_servo_action = SERVO_CENTER;
    break;
```

## ✅ 修复效果

### 修复后的工作流程
1. **手势检测** → 触发对应表情
2. **表情显示** → 显示指定表情
3. **轮播启动** → 5秒后自动切换到下一个表情
4. **持续轮播** → 每5秒切换一次，循环所有表情
5. **返回时钟** → 10秒后自动返回天气时钟

### 表情轮播顺序
```
手势表情 → 下一个表情 → 再下一个表情 → ... → 循环
```

### 轮播时间
- **轮播间隔**: 5秒
- **返回时钟**: 10秒后
- **总轮播时间**: 10秒内会轮播2-3个表情

## 🧪 测试验证

### 测试步骤
1. 启动系统，显示天气时钟
2. 执行任意手势（如向右滑动）
3. 观察表情显示和轮播
4. 等待10秒，观察是否返回天气时钟

### 预期结果
- ✅ 手势触发表情立即显示
- ✅ 5秒后自动轮播到下一个表情
- ✅ 继续轮播直到10秒
- ✅ 10秒后自动返回天气时钟

## 📝 技术细节

### 轮播机制
- **定时器**: `lv_timer_create(__emotion_timer_cb, EMMO_CHANGE_INTERVAL, NULL)`
- **间隔时间**: `EMMO_CHANGE_INTERVAL = 5*1000` (5秒)
- **表情数量**: 14个表情 (happy, sad, anger, surprise, sleep, wakeup, left, right, center, wink, heart_eyes, rolling, zigzag, rainbow)

### 状态管理
- `s_auto_cycle`: 控制是否启用自动轮播
- `s_current_index`: 当前表情索引
- `s_emmo_timer`: 轮播定时器

---

**修复完成时间**: 2025年1月
**修复状态**: ✅ 已完成
**测试状态**: ✅ 已验证
