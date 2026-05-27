# DIY Your Minion Robot 指南

[English guide](./README.md)

## 项目概述

Your Minion Robot 是基于 [TuyaOpen](https://github.com/tuya/TuyaOpen) 与 **tuya.ai** 能力的桌面小黄人风格机器人示例。设备通过双圆形屏幕显示表情，支持 AI 语音对话，并可通过涂鸦智能 App / 云端 DP 控制 **MG90S 360° 连续旋转舵机** 完成转圈、半圈、90° 转向与跳舞等动作。

工程路径：`apps/tuya.ai/your_minion_robot`

## 演示视频

待确认：请补充演示视频链接或二维码图片。

---

## 一、材料清单

以下为推荐硬件清单；「购买链接」列可按实际渠道替换，标 **待确认** 的项请后续补全。

| 序号 | 名称 | 型号 / 规格 | 数量 | 说明 | 购买链接 |
| --- | --- | --- | --- | --- | --- |
| 1 | 主控开发板 | Tuya **T5AI** OTTO 机器人开发核心板（AI 语音） | 1 | 需支持音频、Wi-Fi、PWM、SPI | [淘宝 - T5 AI 语音主控板](https://item.taobao.com/item.htm?id=1020298212272&skuId=6194021594943) |
| 2 | 麦克风 | 与 T5AI 主控配套的 **MEMS 麦克风** | 1 | AI 语音采集；部分主控套件已含 | [淘宝 - MEMS 麦克风](https://item.taobao.com/item.htm?id=644949931333) |
| 3 | 喇叭 | **8Ω** 小扬声器（与主控功放匹配，如 1W～3W） | 1 | AI 语音播放；板级 `SPEAKER_EN`（**P27**） | [淘宝 - 扬声器](https://item.taobao.com/item.htm?id=672684930497&skuId=4888205144701) |
| 4 | 眼睛屏幕 | 圆形 SPI LCD，**160×160**，驱动 **GC9D01**（1.28 寸双眼） | 2 | 与 `TUYA_T5AI_BOARD_EX_MODULE_EYES` 扩展匹配 | [淘宝 - 双眼圆屏](https://item.taobao.com/item.htm?id=866988150753&skuId=5695084509309) |
| 5 | 舵机 | **MG90S 360° 连续旋转舵机** | 1 | 勿与普通 180° 定位舵机混用 | [淘宝 - MG90S 360°](https://item.taobao.com/item.htm?id=39376480811&skuId=5111061374106) |
| 6 | 机身外壳 | Minion 造型 3D 打印件 / 成品外壳 | 1 套 | 模型见工程 [`hardware/3d/`](hardware/3d/) |  |
| 7 | 电源 | 3.7V 电源（舵机与主板） | 1 | 电流建议 ≥1A；红黑正向，1.25 端子 | [淘宝 - 3.7V 电源](https://item.taobao.com/item.htm?id=714971603325&skuId=5217683323385) |
| 8 | 线材 | 杜邦线 / 定制线束 | 若干 | 跳线 5 cm | [淘宝 - 杜邦线](https://item.taobao.com/item.htm?id=624884354507) |
| 9 | 螺丝等 | M1.5 / M2 等 | 若干 | 与外壳设计一致 |  |
| — | TuyaOpen 授权码 | 模组未预烧录时必购 | 1 | 写入 `include/tuya_config.h` | [淘宝](https://item.taobao.com/item.htm?ft=t&id=911596682625) · [涂鸦生产平台](https://platform.tuya.com/purchase/index?type=6) |

### 可选材料

- 摄像头模块（Kconfig 中 `ENABLE_EX_MODULE_CAMERA`，默认关闭）

---

## 二、硬件接线图

### 2.1 双眼屏幕（本教程唯一方案：`ENABLE_EYES_TWO_LCD_SAME`）

本教程仅支持 **双眼共用 SPI0**（`config/TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config`），与材料清单中的双眼圆屏套件一致。两屏共用 **SPI0**，镜像显示。引脚定义见 `boards/T5AI/TUYA_T5AI_BOARD/tuya_t5ai_ex_module.h`：

| 信号 | T5 引脚 | 说明 |
| --- | --- | --- |
| SCL | P14 | SPI0 时钟 |
| CS | P13 | SPI0 片选 |
| SDA | P16 | SPI0 数据 |
| RST | P19 | 屏幕复位 |
| DC | P17 | 数据/命令 |
| BLK | P5 | 背光（低电平有效） |

> 工程 `config/` 下另有单屏、双屏独立 SPI 等配置文件，**当前开发板硬件暂不支持，不建议选用**；若误选可能导致屏幕不亮或引脚冲突。

### 2.2 舵机（MG90S 360°）

| 设备 | T5 资源 | 引脚（参考 T5AI 板卡） | 说明 |
| --- | --- | --- | --- |
| 舵机信号 | PWM0 | **P18** | 代码中 `app_servo_init(TUYA_PWM_NUM_0)` |
| 舵机电源 | 5V / GND | — | 建议就近供电，信号地与主板共地 |

PWM 参数（50Hz）：停止约 1.5ms（duty=750），顺时针约 1.0ms（duty=500），逆时针约 2.0ms（duty=1000）。

### 2.3 其他

| 功能 | 说明 |
| --- | --- |
| 麦克风 | 板载音频采集（`tdd_audio`，16 kHz 单声道） |
| 喇叭 | 功放输出 + `SPEAKER_EN`（**P27**） |
| 对话按键 | 默认 `ai_chat_button`（板级 **P12**，低电平有效） |
| 串口日志 | UART0，波特率 **115200**（烧录、授权、调试） |

---

## 三、组装教程

1. 安装 T5AI 主控与电源模块到机身内腔。  
2. 固定 **麦克风**、**喇叭**（若为主控套件分体件，按板卡说明焊接或插接；注意极性与固定，避免与舵机共振摩擦）。  
3. 固定左右两块圆形屏幕，按 **第二章** 接好 SPI 与背光。  
4. 安装 MG90S 360° 舵机（用于机身旋转 / 转向），信号线接 **PWM0（P18）**。  
5. 检查共地、电源极性，外壳开孔与走线。  

**待确认**：

- 3D 模型文件：放入工程目录 [`hardware/3d/`](hardware/3d/)（STL / STEP 等），详见该目录说明  
- 推荐组装视频（B 站 / 小红书等）  

---

## 四、TuyaOpen 开发文档（重要）

请先阅读官方文档完成环境搭建：

- [涂鸦 TuyaOpen 官方文档](https://tuyaopen.ai/zh/docs/about-tuyaopen)
- [环境准备](https://tuyaopen.ai/zh/docs/quick-start/enviroment-setup)
- [项目编译](https://tuyaopen.ai/zh/docs/quick-start/project-compilation)
- [固件烧录](https://tuyaopen.ai/zh/docs/quick-start/firmware-burning)
- [设备授权](https://tuyaopen.ai/zh/docs/quick-start/equipment-authorization)
- [创建产品（PID）](https://tuyaopen.ai/zh/docs/cloud/tuya-cloud/creating-new-product)

---

## 五、代码下载与编译

### 5.1 获取代码

```bash
git clone https://github.com/tuya/TuyaOpen.git
cd TuyaOpen
```

本工程目录：[your_minion_robot](https://github.com/tuya/TuyaOpen/tree/master/apps/tuya.ai/your_minion_robot)

### 5.2 配置编译环境

在仓库根目录激活 `tos.py`：

**Linux / macOS / WSL：**

```bash
. ./export.sh
```

**Windows PowerShell：**

```powershell
.\export.ps1
```

若提示执行策略限制，可先执行：

```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### 5.3 创建产品并修改 PID （建议直接用默认的PID，本章节跳过）

1. 在 [涂鸦 IoT 平台](https://iot.tuya.com) 创建产品（品类：**待确认**：建议与「机器人 / 智能玩具」相关品类一致）。  
2. 获取 **Product ID（PID）**，并在工程中配置：

**方式 A（推荐，非交互）：** 编辑 `app_default.config` 或 `include/tuya_config.h`：

```c
#define TUYA_PRODUCT_ID "你的PID"
```

**方式 B：** 使用菜单配置：

```bash
cd apps/tuya.ai/your_minion_robot
tos.py config menu
```

路径示例：

```text
(Top) → configure app (your_minion_robot) → TUYA_PRODUCT_ID
```

保存：`S`，退出：`Q`。

> 仓库默认示例 PID 为 `mni21caulntv15yc`（`app_default.config`），**请替换为你自己的产品 PID**。

### 5.4 屏幕与板级配置（仅双眼共用 SPI0）

进入工程目录：

```bash
cd apps/tuya.ai/your_minion_robot
```

**请使用** `config/TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config`（与仓库 `app_default.config` 一致：双眼共用 SPI0）。可通过 `tos.py config choice` 选择该项，或将其内容复制为 `app_default.config` 后直接编译。

| 配置文件 | 说明 |
| --- | --- |
| `config/TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config` | **本教程唯一推荐**：双眼共用 SPI0 |
| `config/TUYA_T5AI_BOARD_EYES.config` | 单屏眼睛方案 — **开发板暂不支持，不建议选** |
| `config/TUYA_T5AI_BOARD_EYES_TWO_LCD.config` | 双眼独立 SPI（QSPI0 + SPI0）— **开发板暂不支持，不建议选** |

### 5.5 配置 UUID / AuthKey（授权码）

在 `include/tuya_config.h` 中填写 TuyaOpen 专用授权码：

```c
#define TUYA_OPENSDK_UUID    "你的UUID"
#define TUYA_OPENSDK_AUTHKEY "你的AuthKey"
```

获取方式见 [tuya.ai README](../README_zh.md#2-确认-tuyaopen-授权码) 或 [设备授权文档](https://tuyaopen.ai/zh/docs/quick-start/equipment-authorization)。

> 若模组出厂已烧录授权，可跳过手动填写；设备启动时会通过 `tuya_authorize_read()` 自动读取。

### 5.6 编译固件

```bash
cd apps/tuya.ai/your_minion_robot
tos.py config choice    # 选择 TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config
tos.py build
```

编译成功后，固件位于：

```text
apps/tuya.ai/your_minion_robot/dist/your_minion_robot_<版本>/debug/
```

---

## 六、固件烧录与授权

### 6.1 烧录

参考官方文档：[固件烧录](https://tuyaopen.ai/zh/docs/quick-start/firmware-burning)

```bash
cd apps/tuya.ai/your_minion_robot
tos.py flash
```

**待确认**：具体烧录工具（串口 / USB）、进入烧录模式按键组合（BOOT + RST 等）以你所用 T5AI 板卡说明书为准。

### 6.2 写入授权（如未预烧录）

```bash
tos.py monitor -b 115200
```

串口内执行：

```text
auth <uuid> <authkey>
```

---

## 七、安装与效果确认

### 7.1 上电与配网

1. 上电，观察双眼屏幕是否正常点亮并显示表情。  
2. 按板卡说明进入配网（蓝牙 / Wi-Fi AP 等，与 `your_chat_bot` 类工程一致）。  
3. **待确认**：配网操作步骤（RESET 次数、指示灯含义）。  

### 7.2 涂鸦智能 App

1. 安装 **涂鸦智能** App。  
2. 添加设备，选择对应产品品类（**待确认**：IoT 平台产品名称）。  
3. 进入设备面板，测试音量、对话与运动控制 DP。

### 7.3 舵机控制 DP 说明

固件中运动控制对应 **DP ID：101**（`DPID_DIRECTION`，枚举型）。App 面板字段名以 IoT 平台配置为准。

| 枚举值 | 功能 |
| --- | --- |
| 0 | 顺时针转一圈（360°） |
| 1 | 逆时针转一圈（360°） |
| 2 | 顺时针转半圈（180°） |
| 3 | 逆时针转半圈（180°） |
| 4 | 右转 90°（顺时针） |
| 5 | 左转 90°（逆时针） |
| 6 | 回正 |
| 7 | 跳舞模式（舵机动作 + 表情联动） |

### 7.4 AI 语音对话

- 支持 AI 聊天与表情联动（双眼 UI 资源在 `src/ui/image/eyes128/`）。  
- 默认唤醒词与对话模式取决于 `ai_components` 中 Kconfig 配置，可在 `tos.py config menu` 中调整。  
- **待确认**：推荐唤醒词、默认对话模式说明。  

---

## 八、舵机角度校准（开发者）

MG90S **360° 连续旋转**舵机通过 **转动时间** 换算角度，不同批次需微调。

校准宏位于 `src/app_servo.c`：

```c
#define SERVO_MS_PER_90_CW   344U
#define SERVO_MS_PER_90_CCW  344U
```

**校准步骤：**

1. 下发一次 **360°** 指令（DP101 = 0 或 1）。  
2. 目测实际转角 `actual_degrees`。  
3. 更新：`MS_PER_90_new = MS_PER_90_old × 360 / actual_degrees`  
4. 重新编译烧录，复测 90° / 180° / 360°。

若顺时针与逆时针偏差不同，可分别修改 `SERVO_MS_PER_90_CW` 与 `SERVO_MS_PER_90_CCW`。

---

## 九、常见问题

| 现象 | 可能原因 | 处理建议 |
| --- | --- | --- |
| 无法连云 / 激活失败 | UUID/AuthKey 错误或 PID 不匹配 | 检查 `tuya_config.h` 与 IoT 平台 PID |
| 屏幕不亮 | 接线 / 配置不匹配 | 确认使用 `TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config`，勿选单屏或双 SPI 方案 |
| 舵机不转 | PWM 线未接 P18 或供电不足 | 查接线、独立 5V 供电 |
| 转角偏大或偏小 | 连续舵机批次差异 | 按第八章调整 `SERVO_MS_PER_90_*` |
| `tos.py: command not found` | 未执行 `export.sh` | 回到仓库根目录重新激活环境 |

---

## 十、资源与支持

- **GitHub 工程**：[TuyaOpen / your_minion_robot](https://github.com/tuya/TuyaOpen/tree/master/apps/tuya.ai/your_minion_robot)  
- **技术交流**：**待确认** — 涂鸦 AI 开发者 QQ 群 / 微信群二维码  
- **社区分享**：欢迎在 GitHub Issue 或涂鸦开发者社区反馈改进建议  

---

## 附录：工程结构速览

```text
your_minion_robot/
├── hardware/
│   └── 3d/                     # 机身外壳等 3D 模型（STL / STEP）
├── app_default.config          # 默认编译配置（可改 PID、双眼 SPI 模式）
├── config/                     # 板级配置（教程仅用 EYES_TWO_LCD_SAME）
├── include/
│   ├── tuya_config.h           # PID、UUID、AuthKey
│   └── app_servo.h             # 舵机 DP 枚举说明
├── src/
│   ├── tuya_main.c             # 主程序、DP101 处理
│   ├── app_servo.c             # 舵机逻辑与校准
│   ├── app_chat_bot.c          # AI 对话初始化
│   └── ui/                     # 双眼表情 UI
├── CMakeLists.txt
└── Kconfig
```

祝您顺利打造属于自己的 Your Minion Robot。
