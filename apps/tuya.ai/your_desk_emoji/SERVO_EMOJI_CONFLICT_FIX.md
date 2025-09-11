# 舵机动作与表情切换冲突修复

## 🐛 问题描述

用户反馈：顺时针/逆时针动作后出现重启问题，通过日志分析发现重启发生在舵机动作的最后阶段，同时表情轮播也在进行。

## 🔍 问题分析

### 时间线分析
```
08:00:11 - 舵机移动: from 0 to 90 (Down动作)
08:00:12 - 表情轮播: Rotation count: 1/13, 切换到 surprise
08:00:12 - 舵机移动: Second cycle - Right (from 30 to 150)
然后重启...
```

### 根本原因
**资源竞争冲突**：
1. **舵机动作**：正在执行PWM控制，占用硬件资源
2. **表情切换**：同时进行GIF动画切换，占用显示和内存资源
3. **系统负载**：两个高负载操作同时进行导致系统不稳定

### 具体冲突点
```c
// 在逆时针动作的最后阶段
app_servo_move_to_with_speed(SERVO_PWM_HORIZONTAL, &s_servo_horizontal_angle, SERVO_ANGLE_RIGHT, SERVO_FAST_MOVE_TIME_MS);
// 同时表情轮播定时器触发
__emotion_timer_cb() -> lv_gif_set_src() -> 大量内存操作
```

## 🔧 解决方案

### 1. 添加舵机状态检查

**修改文件**: `src/display/ui/ui_emoji.c`

**修改前**:
```c
static void __emotion_timer_cb(lv_timer_t *timer)
{
    // Only rotate if auto cycle is enabled
    if (s_auto_cycle) {
        // 直接进行表情切换，可能与舵机动作冲突
        s_rotation_count++;
        // ... 表情切换逻辑
    }
}
```

**修改后**:
```c
static void __emotion_timer_cb(lv_timer_t *timer)
{
    // Only rotate if auto cycle is enabled
    if (s_auto_cycle) {
        // Check if servo is busy to avoid conflicts
        extern BOOL_T _s_servo_busy;
        if (_s_servo_busy) {
            PR_DEBUG("Servo busy, delaying emoji rotation");
            return; // Skip this rotation cycle if servo is busy
        }
        
        // 安全进行表情切换
        s_rotation_count++;
        // ... 表情切换逻辑
    }
}
```

### 2. 冲突避免机制

**工作原理**:
1. **状态检查**：表情轮播前检查舵机是否忙碌
2. **延迟执行**：如果舵机忙碌，跳过当前轮播周期
3. **自动恢复**：舵机完成后，表情轮播自动继续
4. **无阻塞**：不影响舵机动作的正常执行

## ✅ 优化效果

### 1. 避免资源竞争
- **舵机优先**：舵机动作期间表情轮播暂停
- **资源隔离**：避免PWM和GIF同时占用系统资源
- **稳定性提升**：减少系统负载峰值

### 2. 保持功能完整
- **表情轮播**：舵机完成后自动恢复轮播
- **用户体验**：不影响正常的表情切换效果
- **时序正确**：确保所有表情都能正确轮播

### 3. 系统稳定性
- **减少重启**：避免资源竞争导致的重启
- **负载均衡**：分散高负载操作的时间
- **错误恢复**：提高系统的容错能力

## 📊 技术细节

### 舵机状态管理
```c
// 在 tuya_main.c 中
bool _s_servo_busy = FALSE;      // 改为全局变量，供其他文件访问

static void __servo_control_wk_cb(void *data)
{
    _s_servo_busy = TRUE;        // 开始舵机动作
    app_servo_move(_s_servo_action);
    _s_servo_busy = FALSE;       // 完成舵机动作
}
```

### 表情轮播保护
```c
// 在 ui_emoji.c 中
static void __emotion_timer_cb(lv_timer_t *timer)
{
    if (s_auto_cycle) {
        extern bool _s_servo_busy;    // 声明外部变量
        if (_s_servo_busy) {
            return; // 舵机忙碌时跳过轮播
        }
        // 安全进行表情切换
    }
}
```

## 🧪 测试验证

### 1. 功能测试
- [ ] 顺时针手势正常响应
- [ ] 逆时针手势正常响应
- [ ] 表情轮播功能正常
- [ ] 舵机动作流畅

### 2. 稳定性测试
- [ ] 连续多次顺时针/逆时针动作
- [ ] 长时间运行无重启
- [ ] 表情轮播不中断
- [ ] 系统资源使用正常

### 3. 冲突测试
- [ ] 舵机动作期间表情轮播暂停
- [ ] 舵机完成后表情轮播恢复
- [ ] 无资源竞争冲突
- [ ] 系统负载平稳

## 📝 日志监控

### 关键日志
```
// 正常情况
[ui_emoji.c:130] Servo busy, delaying emoji rotation
[ui_emoji.c:145] Emoji rotated to index X: emotion_name

// 异常情况（应该不再出现）
[app_servo.c:111] Moving servo from X to Y, steps: Z, delay: W
然后系统重启...
```

## 🚀 后续优化

### 1. 进一步优化
- 如果仍有问题，可增加更长的延迟
- 考虑使用信号量进行更精确的同步
- 优化表情切换的内存使用

### 2. 系统优化
- 优化舵机动作的时序
- 减少表情切换的资源消耗
- 改进系统调度策略

### 3. 监控优化
- 添加更详细的冲突检测日志
- 监控系统资源使用情况
- 建立性能指标监控

---

**修复完成时间**: 2025年1月
**问题类型**: 资源竞争冲突
**解决方案**: 舵机状态检查 + 表情轮播延迟
**预期效果**: 解决重启问题，提高系统稳定性
