# Seeed XIAO ESP32S3 Sense + SSD1306 + MAX98357A 适配指南

本文档面向 XIAO 开发者，说明如何在 TuyaOpen 上将 `your_chat_bot` 完整适配到 **Seeed XIAO ESP32S3 Sense** 扩展板，外接 **0.96" SSD1306 OLED** 与 **MAX98357A I2S 功放**，实现 AI 语音对话、时钟主屏与滚动字幕。

---

## 1. 目标能力

| 能力 | 说明 |
|------|------|
| AI 语音对话 | 唤醒词「你好小智」，云端 ASR + TTS |
| 麦克风 | Sense 板载 PDM 数字麦（GPIO41/42） |
| 喇叭 | 外接 MAX98357A（D3/D4/D5） |
| 显示 | 0.96" SSD1306 128×64 I2C OLED（横屏） |
| 主屏 UI | 时间居中、日期/品牌、WiFi、左上 AI 状态 |
| 对话 UI | 全屏遮罩 + 左上状态 + 正文循环滚动 |

---

## 2. 硬件清单

| 器件 | 规格/型号 | 数量 |
|------|-----------|------|
| 主控 | Seeed XIAO ESP32S3 **Sense** 扩展板 | 1 |
| 显示屏 | 0.96" SSD1306 OLED，I2C，128×64，地址 0x3C | 1 |
| 功放 | MAX98357A I2S 模块 | 1 |
| 喇叭 | 4Ω 或 8Ω 小喇叭（建议 0.5~3 W） | 1 |
| 杜邦线 | 若干 | — |

> **注意**：必须使用 **Sense** 版（带板载麦克风）。非 Sense 版需自行接 I2S 麦克风并关闭 PDM 配置（见 §6.3）。

---

## 3. 引脚与接线

### 3.1 XIAO ESP32S3 丝印与 GPIO 对照

| 丝印 | GPIO | 本方案用途 |
|------|------|------------|
| D0 | GPIO1 | OLED SDA |
| D1 | GPIO2 | OLED SCL |
| D3 | GPIO4 | MAX98357A LRC（WS） |
| D4 | GPIO5 | MAX98357A BCLK |
| D5 | GPIO6 | MAX98357A DIN |
| D11 | GPIO42 | Sense PDM 麦 CLK（板载，无需外接） |
| D12 | GPIO41 | Sense PDM 麦 DATA（板载，无需外接） |
| D6/D7 | GPIO43/44 | USB 串口（烧录/日志） |

### 3.2 SSD1306 OLED（I2C）

```
SSD1306          XIAO ESP32S3
────────         ─────────────
VCC    ────────  3V3
GND    ────────  GND
SDA    ────────  D0 (GPIO1)
SCL    ────────  D1 (GPIO2)
```

- I2C 地址：**0x3C**（部分模块为 0x3D，需改 `board_config.h` 中 `OLED_I2C_ADDR`）
- 建议线长 < 10 cm，与 I2S 线分开走线减少干扰

### 3.3 MAX98357A I2S 功放

```
MAX98357A        XIAO ESP32S3
─────────        ─────────────
VIN    ────────  3V3（或外接 5V，GND 共地）
GND    ────────  GND
BCLK   ────────  D4 (GPIO5)
LRC    ────────  D3 (GPIO4)
DIN    ────────  D5 (GPIO6)
SD     ────────  3V3（常开，使能输出）
GAIN   ────────  悬空或按模块说明（默认增益）
SPK+/SPK- ─────  喇叭
```

- **SD 接 3V3**：芯片常开；接 GND 则静音
- **供电**：仅 3V3 时音量受限；大音量可 VIN 接 5V，**GND 必须与 XIAO 共地**
- 麦克风（PDM）与喇叭（I2S）引脚**互不占用**

### 3.4 接线总览示意

```
                    ┌─────────────────┐
   OLED I2C ────────│ D0/D1           │
   MAX98357 I2S ────│ D3/D4/D5        │  XIAO ESP32S3 Sense
   (PDM 麦板载)     │ D11/D12 (内部)  │
                    └─────────────────┘
```

---

## 4. 开发环境

### 4.1 系统依赖（Linux / WSL2）

```bash
sudo apt update
sudo apt install -y build-essential git python3 python3-pip python3-venv \
  libusb-1.0-0-dev libc6-i386 libsystemd-dev clang-format
```

### 4.2 初始化 TuyaOpen

```bash
cd /path/to/TuyaOpen
. ./export.sh
```

### 4.3 避免交互式提示（CI / 脚本环境可选）

```bash
mkdir -p .cache && touch .cache/.dont_prompt_update_platform
```

---

## 5. 工程配置与编译

### 5.1 进入工程目录

```bash
cd apps/tuya.ai/your_chat_bot
```

### 5.2 选择 XIAO 预置配置

**方式 A（推荐）**：直接复制预置文件

```bash
cp config/XIAO_ESP32S3.config app_default.config
```

**方式 B**：交互选择（需 TTY）

```bash
tos.py config choice   # 选择 XIAO_ESP32S3 相关项
```

### 5.3 关键 Kconfig 项说明

以下条目应在 `app_default.config` 中为 **y**（或已 select）：

| 配置项 | 作用 |
|--------|------|
| `CONFIG_BOARD_CHOICE_XIAO_ESP32S3` | 选择 XIAO ESP32S3 板级 |
| `CONFIG_ENABLE_ESP_DISPLAY` | 启用 ESP 显示栈 |
| `CONFIG_ENABLE_COMP_AI_DISPLAY` | 启用 AI 显示组件 |
| `CONFIG_ENABLE_AI_CHAT_CUSTOM_UI` | 使用自定义 UI（非默认 OLED/微信 UI） |
| `CONFIG_ENABLE_LIBLVGL` | LVGL |
| `CONFIG_FONT_TEXT_SIZE_14_1` | 14px 普惠字体（适合 128×64） |
| `CONFIG_BOARD_AUDIO_PDM_MIC` | Sense 板载 PDM 麦克风 |
| `CONFIG_BOARD_AUDIO_I2S_SPEAKER` | MAX98357A 外接喇叭 |
| `CONFIG_ENABLE_WAKEUP_KEYWORD_NIHAO_XIAOZHI` | 唤醒词「你好小智」 |

完整示例见：`config/XIAO_ESP32S3.config`。

### 5.4 设备授权（必做）

编辑 `include/tuya_config.h`，填入涂鸦 IoT 平台申请的 **PID / UUID / AuthKey**：

```c
#define TUYA_PRODUCT_ID     "你的PID"
#define TUYA_OPENSDK_UUID    "你的UUID"
#define TUYA_OPENSDK_AUTHKEY "你的AuthKey"
```

> 勿将真实授权信息提交到公开 Git 仓库。

### 5.5 编译

```bash
tos.py check    # 可选：检查工具链与子模块
tos.py build
```

产物路径：

```
dist/your_chat_bot_1.0.1/
```

若 Kconfig 与缓存不一致，可先清理：

```bash
rm -f .build/cache/using.config .build/cache/using.cmake
tos.py clean && tos.py build
```

---

## 6. 软件架构说明

### 6.1 板级目录

```
boards/ESP32/XIAO_ESP32S3/
├── board_config.h      # 引脚、显示、音频宏
├── xiao_esp32s3.c      # 注册 audio / button / LED / display
├── Kconfig             # 板级 Kconfig 选项
└── CMakeLists.txt
```

显示驱动共用：`boards/ESP32/common/lcd/oled_ssd1306.c`。

### 6.2 自定义 UI

| 文件 | 说明 |
|------|------|
| `include/xiao_ssd1306_ui.h` | UI 注册接口 |
| `src/xiao_ssd1306_ui.c` | 横屏 128×64 黑白 UI 实现 |
| `src/app_chat_bot.c` | 在 `ai_chat_init()` 前调用 `xiao_ssd1306_ui_register()` |

**主屏（待机）布局：**

```
待命          [WiFi]
      12:34
06/02          TUYA.AI
```

**对话屏：** 全屏黑色遮罩覆盖主屏；左上短状态（待命/聆听/思考/说话）；正文区域横向循环滚动，不超出 128×64。

### 6.3 音频通路

| 通路 | I2S 端口 | 引脚 | 说明 |
|------|----------|------|------|
| 麦克风 RX | I2S0 PDM | GPIO42 CLK, GPIO41 DIN | `CONFIG_BOARD_AUDIO_PDM_MIC` |
| 喇叭 TX | I2S1 STD | GPIO5 BCLK, GPIO4 LRC, GPIO6 DIN | `CONFIG_BOARD_AUDIO_I2S_SPEAKER` |

实现位置：

- `platform/ESP32/tuya_open_sdk/tuyaos_adapter/src/drivers/tkl_i2s.c` — 引脚表
- `boards/ESP32/common/audio/tdd_audio_no_codec.c` — 采集/播放逻辑
- `platform/ESP32/tuya_open_sdk/tuyaos_adapter/CMakeLists.txt` — 将 Kconfig 符号桥接到编译宏

**若使用外接 I2S 麦（非 Sense PDM）**：在 `app_default.config` 中设置：

```
# CONFIG_BOARD_AUDIO_PDM_MIC is not set
```

此时 D3/D4/D5 用于 I2S 麦克风 RX，**不能与 MAX98357A 喇叭方案同时按本文接线**。

### 6.4 OLED 单色显示注意

SSD1306 在 `esp_lvgl_port` 中使用 RGB565 + `monochrome=true`。已对 `esp_lvgl_port_disp.c` 做 **亮度阈值** 转换（非仅蓝色通道），避免黄字/蓝底等偏色。

UI 侧须：

- 禁用 LVGL 主题色块，使用纯黑底 `lv_color_black()` / 纯白字 `lv_color_white()`
- `board_config.h` 中 `DISPLAY_COLOR_FORMAT = LV_COLOR_FORMAT_RGB565`，`DISPLAY_MONOCHROME = true`

若屏幕方向不对，可调整 `DISPLAY_MIRROR_X` / `DISPLAY_MIRROR_Y`。

---

## 7. 烧录与串口

### 7.1 查找串口

```bash
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

XIAO 常见为 `/dev/ttyACM0`（USB CDC）。

### 7.2 烧录

```bash
cd apps/tuya.ai/your_chat_bot
tos.py flash -p /dev/ttyACM0
```

### 7.3 查看日志

```bash
tos.py monitor -p /dev/ttyACM0
```

### 7.4 启动成功日志参考

应出现类似输出：

```
use custom ai chat ui
OLED color_format=18 (0x12=RGB565 mono)
Audio RX: XIAO Sense PDM mic (GPIO41/42)
I2S RX (PDM) + TX (MAX98357 D3/D4/D5)
```

---

## 8. 功能验证清单

按顺序自检：

- [ ] 上电后 OLED 黑底白字，显示时间与「待命」
- [ ] 配网后右上 WiFi 图标变化，时间同步后走时正常
- [ ] 串口日志无 `I2S` / `OLED` 初始化失败
- [ ] 说「你好小智」可唤醒，状态变为「聆听」
- [ ] AI 回复时进入对话全屏，字幕滚动
- [ ] 喇叭可听到 TTS（可先调低默认音量 70 防削波）
- [ ] 对话结束约 8 s 回到时钟主屏

**按键**：GPIO0（BOOT）为板载用户键，具体对话模式取决于 `app_default.config` 中 chat mode 相关项（默认唤醒模式）。

**LED**：GPIO21 用户 LED（低电平点亮）。

---

## 9. 常见问题

### 9.1 OLED 无显示 / 全黑

| 可能原因 | 处理 |
|----------|------|
| SDA/SCL 接反或未接 3V3 | 核对 §3.2 |
| I2C 地址不是 0x3C | 改 `OLED_I2C_ADDR` 为 `0x3D` |
| 未选 XIAO 配置 | 确认 `CONFIG_BOARD_CHOICE_XIAO_ESP32S3=y` |
| 彩块/花屏 | 确认自定义 UI 与 RGB565 mono 补丁已编入固件 |

### 9.2 颜色异常（黄块、蓝底）

- 确认 `xiao_ssd1306_ui.c` 使用黑白配色且 label 背景透明
- 确认 `esp_lvgl_port` 单色转换使用亮度阈值（仓库内已修补）

### 9.3 喇叭无声

| 可能原因 | 处理 |
|----------|------|
| 未开 `CONFIG_BOARD_AUDIO_I2S_SPEAKER` | 设为 `y` 后全量编译 |
| SD 脚未接高 | SD 接 3V3 |
| BCLK/LRC/DIN 接错 | 严格按 §3.3 |
| 固件仍为「PDM only」 | 日志应含 `TX (MAX98357 D3/D4/D5)` |
| 音量过低 / 供电不足 | App 调高音量；VIN 改 5V 共地 |

### 9.4 麦克风无输入 / 唤醒失败

- 确认 Sense 扩展板已正确叠装
- `CONFIG_BOARD_AUDIO_PDM_MIC=y`
- 环境安静时测试唤醒词「你好小智」
- 检查 UUID/AuthKey 与 PID 是否匹配云端产品

### 9.5 编译配置不生效

```bash
rm -f .build/cache/using.config .build/cache/using.cmake
tos.py clean && tos.py build
```

### 9.6 屏幕方向/镜像不对

修改 `boards/ESP32/XIAO_ESP32S3/board_config.h`：

```c
#define DISPLAY_MIRROR_X  true   /* 按需 true/false */
#define DISPLAY_MIRROR_Y  true
```

---

## 10. 从零适配检查表（给新开发者）

1. [ ] 硬件按 §3 完成接线并上电
2. [ ] 克隆 TuyaOpen 并 `export.sh`
3. [ ] `cp config/XIAO_ESP32S3.config app_default.config`
4. [ ] 填写 `include/tuya_config.h` 授权
5. [ ] `tos.py build` 成功
6. [ ] `tos.py flash` 烧录
7. [ ] 串口确认 §7.4 日志
8. [ ] 完成 §8 功能清单
9. [ ] 按需调整 UI：`src/xiao_ssd1306_ui.c`
10. [ ] 按需调整引脚：`board_config.h` + `tkl_i2s.c`（引脚变更时）

---

## 11. 相关源文件索引

| 路径 | 说明 |
|------|------|
| `apps/tuya.ai/your_chat_bot/config/XIAO_ESP32S3.config` | 预置编译配置 |
| `apps/tuya.ai/your_chat_bot/include/tuya_config.h` | 设备授权 |
| `apps/tuya.ai/your_chat_bot/src/xiao_ssd1306_ui.c` | OLED UI |
| `boards/ESP32/XIAO_ESP32S3/board_config.h` | 板级引脚与显示参数 |
| `boards/ESP32/common/lcd/oled_ssd1306.c` | SSD1306 驱动 |
| `boards/ESP32/common/audio/tdd_audio_no_codec.c` | 音频采集/播放 |
| `platform/ESP32/.../tkl_i2s.c` | I2S 引脚与 PDM |
| `platform/ESP32/.../esp_lvgl_port_disp.c` | LVGL 单色转换 |

---

## 12. 参考链接

- [TuyaOpen 编译说明](https://tuyaopen.ai/zh/docs/quick-start/project-compilation)
- [涂鸦 IoT 应用创建（PID/授权）](https://developer.tuya.com/cn/docs/iot-device-dev/application-creation?id=Kbxw7ket3aujc)
- [Seeed XIAO ESP32S3 Sense  Wiki](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)

---

*文档版本：与 `your_chat_bot` XIAO_ESP32S3 分支适配同步（SSD1306 自定义 UI + PDM 麦 + MAX98357A 喇叭）。*
