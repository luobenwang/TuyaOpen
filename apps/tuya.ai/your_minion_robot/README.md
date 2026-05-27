# DIY Your Minion Robot Guide

[简体中文完整教程](./README_zh.md)

## Project Overview

Your Minion Robot is a desktop Minion-style robot sample built on [TuyaOpen](https://github.com/tuya/TuyaOpen) and **tuya.ai**. It shows expressions on dual round displays, supports AI voice chat, and uses the Tuya Smart App / cloud DPs to drive an **MG90S 360° continuous rotation servo** for full turns, half turns, 90° turns, reset, and dance moves.

Project path: `apps/tuya.ai/your_minion_robot`

## Demo Video

TBD: add a demo video link or QR code image.

---

## 1. Bill of Materials

Recommended hardware list. Replace purchase links as needed; items marked **TBD** can be filled in later.

| # | Item | Model / spec | Qty | Notes | Purchase link |
| --- | --- | --- | --- | --- | --- |
| 1 | Main board | Tuya **T5AI** OTTO robot core board (AI voice) | 1 | Audio, Wi-Fi, PWM, SPI | [Taobao - T5 AI voice board](https://item.taobao.com/item.htm?id=1020298212272&skuId=6194021594943) |
| 2 | Microphone | **MEMS mic** matched to T5AI | 1 | AI voice input; may be included in board kit | [Taobao - MEMS mic](https://item.taobao.com/item.htm?id=644949931333) |
| 3 | Speaker | **8Ω** small speaker (1W–3W, matched to onboard amp) | 1 | AI playback; `SPEAKER_EN` on **P27** | [Taobao - speaker](https://item.taobao.com/item.htm?id=672684930497&skuId=4888205144701) |
| 4 | Eye displays | Round SPI LCD **160×160**, **GC9D01** (1.28" dual-eye) | 2 | Matches `TUYA_T5AI_BOARD_EX_MODULE_EYES` | [Taobao - dual round LCD](https://item.taobao.com/item.htm?id=866988150753&skuId=5695084509309) |
| 5 | Servo | **MG90S 360° continuous rotation** | 1 | Do not mix with 180° position servos | [Taobao - MG90S 360°](https://item.taobao.com/item.htm?id=39376480811&skuId=5111061374106) |
| 6 | Enclosure | Minion 3D print / ready-made shell | 1 set | Models in [`hardware/3d/`](hardware/3d/) | **TBD** (print service / product link) |
| 7 | Power | 3.7V supply (board + servo) | 1 | ≥1A; red/black polarity, 1.25 mm connector | [Taobao - 3.7V power](https://item.taobao.com/item.htm?id=714971603325&skuId=5217683323385) |
| 8 | Wiring | Dupont / custom harness | several | 5 cm jumper wires | [Taobao - dupont wires](https://item.taobao.com/item.htm?id=624884354507) |
| 9 | Fasteners | M1.5 / M2, etc. | several | Match enclosure design | **TBD** |
| — | TuyaOpen license | Required if module is not pre-provisioned | 1 | Set in `include/tuya_config.h` | [Taobao](https://item.taobao.com/item.htm?ft=t&id=911596682625) · [Tuya platform](https://platform.tuya.com/purchase/index?type=6) |

### Optional

- Camera module (`ENABLE_EX_MODULE_CAMERA` in Kconfig, off by default)

---

## 2. Wiring

### 2.1 Dual eyes (only supported option: `ENABLE_EYES_TWO_LCD_SAME`)

This guide only supports **dual displays on shared SPI0** (`config/TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config`), matching the dual round LCD kit in the BOM. Both panels share **SPI0** with mirrored content. Pin map: `boards/T5AI/TUYA_T5AI_BOARD/tuya_t5ai_ex_module.h`:

| Signal | T5 pin | Notes |
| --- | --- | --- |
| SCL | P14 | SPI0 clock |
| CS | P13 | SPI0 chip select |
| SDA | P16 | SPI0 data |
| RST | P19 | Reset |
| DC | P17 | Data/command |
| BLK | P5 | Backlight (active low) |

> Other files under `config/` (single eye, dual independent SPI) are **not supported on this board** and are **not recommended**; wrong choice may leave the display off or cause pin conflicts.

### 2.2 Servo (MG90S 360°)

| Device | T5 resource | Pin (T5AI board) | Notes |
| --- | --- | --- | --- |
| Servo signal | PWM0 | **P18** | `app_servo_init(TUYA_PWM_NUM_0)` |
| Servo power | 5V / GND | — | Local 5V recommended; common ground with MCU |

PWM (50 Hz): stop ~1.5 ms (duty=750), CW ~1.0 ms (500), CCW ~2.0 ms (1000).

### 2.3 Other

| Function | Notes |
| --- | --- |
| Microphone | Onboard capture via `tdd_audio`, 16 kHz mono |
| Speaker | Amp + `SPEAKER_EN` (**P27**) |
| Chat button | `ai_chat_button` (**P12**, active low) |
| UART log | UART0, **115200** (flash, auth, debug) |

---

## 3. Assembly

1. Mount T5AI main board and power inside the body.  
2. Fix **microphone** and **speaker** (solder or plug per board guide; avoid rubbing against the servo).  
3. Mount both round displays; wire SPI and backlight per **Section 2**.  
4. Install MG90S 360° servo; signal to **PWM0 (P18)**.  
5. Check ground, polarity, and cable routing through the shell.  

**TBD:**

- 3D models: place STL/STEP files under [`hardware/3d/`](hardware/3d/)  
- Recommended assembly video (Bilibili, etc.)

---

## 4. TuyaOpen docs (read first)

- [TuyaOpen overview](https://tuyaopen.ai/docs/about-tuyaopen)
- [Environment setup](https://tuyaopen.ai/docs/quick-start/enviroment-setup)
- [Build](https://tuyaopen.ai/docs/quick-start/project-compilation)
- [Flash](https://tuyaopen.ai/docs/quick-start/firmware-burning)
- [Device authorization](https://tuyaopen.ai/docs/quick-start/equipment-authorization)
- [Create product (PID)](https://tuyaopen.ai/docs/cloud/tuya-cloud/creating-new-product)

---

## 5. Clone and build

### 5.1 Get source

```bash
git clone https://github.com/tuya/TuyaOpen.git
cd TuyaOpen
```

Project: [your_minion_robot](https://github.com/tuya/TuyaOpen/tree/master/apps/tuya.ai/your_minion_robot)

### 5.2 Environment

From repo root:

**Linux / macOS / WSL:**

```bash
. ./export.sh
```

**Windows PowerShell:**

```powershell
.\export.ps1
```

If execution policy blocks scripts:

```powershell
Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### 5.3 Product ID (PID)

1. Create a product on [Tuya IoT Platform](https://iot.tuya.com) (**TBD**: robot / smart toy category).  
2. Set **Product ID** in the project:

**Option A (non-interactive):** edit `app_default.config` or `include/tuya_config.h`:

```c
#define TUYA_PRODUCT_ID "your_PID"
```

**Option B:** `tos.py config menu` → `your_minion_robot` → `TUYA_PRODUCT_ID` (save `S`, quit `Q`).

> Sample PID in repo: `mni21caulntv15yc` — **replace with your own**.

### 5.4 Display / board config (dual SPI0 only)

```bash
cd apps/tuya.ai/your_minion_robot
```

Use **`config/TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config`** (same as `app_default.config`). Pick it via `tos.py config choice` or copy into `app_default.config`.

| Config file | Notes |
| --- | --- |
| `config/TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config` | **Only option for this guide** — dual LCD on SPI0 |
| `config/TUYA_T5AI_BOARD_EYES.config` | Single eye — **board not supported, do not use** |
| `config/TUYA_T5AI_BOARD_EYES_TWO_LCD.config` | Dual independent SPI — **board not supported, do not use** |

### 5.5 UUID / AuthKey

In `include/tuya_config.h`:

```c
#define TUYA_OPENSDK_UUID    "your_UUID"
#define TUYA_OPENSDK_AUTHKEY "your_AuthKey"
```

See [tuya.ai README](../README.md#2-confirm-tuyaopen-license-code) or [authorization doc](https://tuyaopen.ai/docs/quick-start/equipment-authorization).

> Skip if the module already has a burned-in license (`tuya_authorize_read()` at boot).

### 5.6 Build

```bash
cd apps/tuya.ai/your_minion_robot
tos.py config choice    # TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config
tos.py build
```

Output:

```text
apps/tuya.ai/your_minion_robot/dist/your_minion_robot_<version>/debug/
```

---

## 6. Flash and authorization

### 6.1 Flash

[Official flash guide](https://tuyaopen.ai/docs/quick-start/firmware-burning)

```bash
cd apps/tuya.ai/your_minion_robot
tos.py flash
```

**TBD:** BOOT/RST sequence and tool details per your T5AI board manual.

### 6.2 Write license (if needed)

```bash
tos.py monitor -b 115200
```

In the serial console:

```text
auth <uuid> <authkey>
```

---

## 7. Bring-up and testing

### 7.1 Power and provisioning

1. Power on; both eyes should show expressions.  
2. Provision Wi-Fi per board guide (same family as `your_chat_bot`).  
3. **TBD:** exact RESET / LED provisioning steps.

### 7.2 Tuya Smart App

1. Install **Tuya Smart**.  
2. Add device (**TBD**: product category name on IoT platform).  
3. Test volume, voice chat, and motion DPs.

### 7.3 Servo DP (ID 101)

Motion control uses **DP ID 101** (`DPID_DIRECTION`, enum). App labels depend on your IoT product UI.

| Value | Action |
| --- | --- |
| 0 | CW full turn (360°) |
| 1 | CCW full turn (360°) |
| 2 | CW half turn (180°) |
| 3 | CCW half turn (180°) |
| 4 | Turn right 90° (CW) |
| 5 | Turn left 90° (CCW) |
| 6 | Reset / center |
| 7 | Dance (servo + expression) |

### 7.4 AI voice

- Chat and expressions (`src/ui/image/eyes128/`).  
- Wake word / mode from `ai_components` Kconfig via `tos.py config menu`.  
- **TBD:** recommended wake word.

---

## 8. Servo calibration (developers)

MG90S **360° continuous** servos use **time** for angle; batch variance needs tuning.

Macros in `src/app_servo.c`:

```c
#define SERVO_MS_PER_90_CW   344U
#define SERVO_MS_PER_90_CCW  344U
```

1. Trigger one **360°** move (DP101 = 0 or 1).  
2. Measure actual angle `actual_degrees`.  
3. Update: `MS_PER_90_new = MS_PER_90_old × 360 / actual_degrees`  
4. Rebuild, flash, re-test 90° / 180° / 360°.

Tune `SERVO_MS_PER_90_CW` and `SERVO_MS_PER_90_CCW` separately if needed.

---

## 9. FAQ

| Symptom | Likely cause | Fix |
| --- | --- | --- |
| Cloud / activation fails | Wrong UUID/AuthKey or PID | Check `tuya_config.h` and IoT PID |
| Display off | Wiring or wrong config | Use `TUYA_T5AI_BOARD_EYES_TWO_LCD_SAME.config` only |
| Servo idle / weak | P18 wiring or power | Check PWM0; adequate servo supply |
| Turn angle wrong | Servo batch variance | Adjust `SERVO_MS_PER_90_*` (Section 8) |
| `tos.py: command not found` | Environment not loaded | Run `. ./export.sh` at repo root |

---

## 10. Resources

- **GitHub:** [TuyaOpen / your_minion_robot](https://github.com/tuya/TuyaOpen/tree/master/apps/tuya.ai/your_minion_robot)  
- **Community:** **TBD** — Tuya AI developer groups  
- Feedback welcome via GitHub Issues or Tuya developer community.

---

## Appendix: project layout

```text
your_minion_robot/
├── hardware/
│   └── 3d/                     # Enclosure STL / STEP
├── app_default.config          # Default build (PID, dual SPI0 eyes)
├── config/                     # Board configs (guide uses EYES_TWO_LCD_SAME only)
├── include/
│   ├── tuya_config.h           # PID, UUID, AuthKey
│   └── app_servo.h             # Servo DP enums
├── src/
│   ├── tuya_main.c             # Main, DP101 handler
│   ├── app_servo.c             # Servo logic and calibration
│   ├── app_chat_bot.c          # AI chat init
│   └── ui/                     # Dual-eye UI assets
├── CMakeLists.txt
└── Kconfig
```

Have fun building your Minion Robot.
