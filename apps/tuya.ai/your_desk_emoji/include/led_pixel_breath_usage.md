# `led_pixel_breath` 接口使用说明

本文档说明 `include/led_pixel_breath.h` 中 LED Pixel 接口的使用方式，适用于当前 DuckyClaw 工程。

## 1. 生效条件

接口在以下宏开启时可用：

```c
#if defined(ENABLE_LEDS_PIXEL) && (ENABLE_LEDS_PIXEL)
```

如果该宏关闭，相关函数不会参与编译。

## 2. 调用时机

- 需先完成硬件注册：`board_register_hardware()`
- 需先初始化软件定时器：`tal_sw_timer_init()`
- 建议在系统初始化完成后（如 `user_main()` 后半段）再调用 LED 接口

> 说明：模块内部采用懒初始化（首次调用时自动 find/open 设备并创建定时器）。

## 3. 接口总览

头文件：`include/led_pixel_breath.h`

- `led_pixel_register_hardware(void)`  
  注册 LED Pixel 驱动（供板级 `board_register_hardware()` 调用）
- `led_pixel_breath_start_white(void)`  
  启动白色呼吸
- `led_pixel_breath_start_blue(const char *rgb_hex)`  
  启动自定义颜色呼吸（函数名历史保留，实际支持任意 RGB）
- `led_pixel_apply_rgb_mode(const char *rgb_hex, uint8_t brightness_pct, bool is_breath)`  
  按颜色 + 亮度 + 模式控制（推荐主入口）
- `led_pixel_breath_start_purple(void)`
- `led_pixel_breath_start_red(void)`
- `led_pixel_breath_start_green(void)`
- `led_pixel_breath_start_cyan(void)`
- `led_pixel_breath_start_yellow(void)`
- `led_pixel_breath_start_orange(void)`
- `led_pixel_breath_start_pink(void)`
- `led_pixel_breath_stop(void)`  
  停止呼吸并熄灭灯带

所有接口返回 `OPERATE_RET`，成功为 `OPRT_OK`。

## 4. 参数说明

### 4.1 `rgb_hex` 格式

- 支持 `RRGGBB` 或 `#RRGGBB`
- 例如：`"00A2FF"`、`"#FF8800"`
- 非法字符串会返回 `OPRT_INVALID_PARM`

### 4.2 `brightness_pct`

- 范围建议 `0..100`
- 大于 `100` 会在内部被钳制到 `100`
- 等于 `0` 时会执行 `led_pixel_breath_stop()`

### 4.3 `is_breath`

- `true`：呼吸模式
- `false`：常亮模式

## 5. 推荐用法

### 5.0 板级注册拆分（新增）

当前已将 `board_register_hardware()` 中的 `led_pixel` 注册逻辑拆分为独立接口：

```c
OPERATE_RET rt = led_pixel_register_hardware();
if (rt != OPRT_OK) {
    PR_ERR("led_pixel_register_hardware failed rt:%d", rt);
}
```

在本项目当前板级实现中，`board_register_hardware()` 已调用该接口，业务层通常无需重复调用。

### 5.1 开机默认白色呼吸

```c
OPERATE_RET rt = led_pixel_breath_start_white();
if (rt != OPRT_OK) {
    PR_ERR("led_pixel_breath_start_white failed rt:%d", rt);
}
```

### 5.2 根据业务设置颜色/亮度/模式（推荐）

```c
OPERATE_RET rt = led_pixel_apply_rgb_mode("#00A2FF", 70, true);
if (rt != OPRT_OK) {
    PR_ERR("led_pixel_apply_rgb_mode failed rt:%d", rt);
}
```

### 5.3 切到常亮

```c
OPERATE_RET rt = led_pixel_apply_rgb_mode("FF3300", 100, false);
if (rt != OPRT_OK) {
    PR_ERR("led steady mode failed rt:%d", rt);
}
```

### 5.4 熄灭灯带

```c
OPERATE_RET rt = led_pixel_breath_stop();
if (rt != OPRT_OK) {
    PR_ERR("led_pixel_breath_stop failed rt:%d", rt);
}
```

## 6. 移植配置（重要）

实现文件支持通过编译宏覆盖默认参数，以适配不同灯带/板卡：

- `LEDS_PIXEL_NAME`：像素设备名（默认 `"led_pixel"`）
- `LED_PIXEL_HW_ORDER_GRB`：通道顺序（`1=GRB`, `0=RGB`）
- `LED_PIXELS_TOTAL_NUM`：灯珠数量（默认 `24`）
- `LED_PIXEL_COLOR_RES`：颜色分辨率（默认 `100`）
- `LED_PIXEL_BREATH_STEP`：呼吸步进（默认 `1`）
- `LED_PIXEL_BREATH_MS`：呼吸周期毫秒（默认 `35`）

移植时优先确认：

1. 设备名是否匹配板级注册名  
2. 灯带是否为 GRB 排列
3. 灯珠数量与实际硬件一致

## 7. 常见问题

- 调用后无灯效：先确认 `ENABLE_LEDS_PIXEL` 是否开启，以及板级是否已注册 `led_pixel` 设备
- 颜色不对（红绿互换）：调整 `LED_PIXEL_HW_ORDER_GRB`
- 亮度感觉过高/过低：调整 `brightness_pct` 或 `LED_PIXEL_COLOR_RES`
- 呼吸快慢不合适：调整 `LED_PIXEL_BREATH_MS` 和 `LED_PIXEL_BREATH_STEP`
