# 情绪图片测试指南

## 概述
现在 `__ui_set_emotion_image` 函数支持以下情绪图片：

1. **happy** / **HAPPY** - 开心表情
2. **sad** / **SAD** - 悲伤表情  
3. **think** / **THINKING** - 思考表情
4. **angry** / **ANGRY** - 愤怒表情
5. **sleep** - 睡觉表情
6. **love** - 爱心表情
7. **shock** / **SURPRISE** - 惊讶表情

## 测试方法

### 方法1: 通过消息系统测试
```c
// 发送情绪消息到显示系统
app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t*)"sad", strlen("sad"));
app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t*)"angry", strlen("angry"));
app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t*)"think", strlen("think"));
app_display_send_msg(TY_DISPLAY_TP_EMOTION, (uint8_t*)"think", strlen("think"));
```

### 方法2: 直接调用函数测试
```c
// 直接调用情绪设置函数
ui_set_emotion("sad");
ui_set_emotion("angry");
ui_set_emotion("think");
ui_set_emotion("think");
ui_set_emotion("love");
ui_set_emotion("think");
```

### 方法3: 使用预定义常量测试
```c
// 使用头文件中定义的常量
ui_set_emotion(EMOJI_SAD);
ui_set_emotion(EMOJI_ANGRY);
ui_set_emotion(EMOJI_THINKING);
```

## 编译说明
所有情绪图片文件都已包含在 CMakeLists.txt 中：
- emotion_happy_image.c
- emotion_sad_image.c
- emotion_think_image.c
- emotion_angry_image.c
- emotion_sleep_image.c
- emotion_love_image.c
- emotion_shock_image.c

## 注意事项
1. 所有图片都是 RGB565 格式
2. 图片大小应该一致（通常是 128x128 像素）
3. 如果指定的情绪不存在，会默认显示 happy 表情
4. 函数支持大小写不敏感的情绪名称
