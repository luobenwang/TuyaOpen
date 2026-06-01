# TuyaOpen 全志 T113 (glibc) 移植教程

本文档记录了如何把 `apps/tuya.ai/your_chat_bot` 这个 Demo 编译到 **全志 Allwinner T113（Cortex‑A7 / armv7‑a / 硬浮点 / glibc）** 开发板上，以及如何把它移植到你自己的 T113 板子。

> 适用对象：在 x86_64 Linux（含 WSL2）主机上**交叉编译**，目标板为运行 Linux（glibc）的全志 T113 系列（T113-S3 / D1s 同架构思路类似）。

---

## 1. 总览：本次集成做了什么

为支持 T113，新增/修改了下列内容：

### 新增（板级 BSP，全部在 `boards/LINUX/t113_glibc/`）

| 文件 | 作用 |
|------|------|
| `Kconfig` | 板级配置项，定义 `CHIP_CHOICE`/`BOARD_CHOICE` = `t113_glibc` |
| `CMakeLists.txt` | 板级源码编译规则，并把适配层头文件路径加入 |
| `board_com_api.c` / `board_com_api.h` | `board_register_hardware()` 板级硬件注册入口（音频/按键按 Kconfig 可选） |
| `toolchain_wrapper/t113-gcc`、`t113-g++` | 交叉编译器**包装脚本**（见第 3 节，移植的关键） |
| `compat-include/linux/gpio.h` | 补齐工具链缺失的 GPIO 字符设备 UAPI 头 |
| `tkl_audio_ai_placeholder.c` | VAD / KWS 占位实现（armv7 无预编译 MNN 库，见第 6 节） |
| `toolchain-sunxi-glibc/` | 厂商提供的 sunxi OpenWrt glibc 交叉工具链（解压自 `t113_glibc.tar.gz`） |

### 修改（SDK 框架，已做到对其它板子无副作用）

| 文件 | 改动 |
|------|------|
| `boards/LINUX/Kconfig` | 在“Choice a board”里注册 `BOARD_CHOICE_T113_GLIBC` |
| `platform/LINUX/compiler_setup.cmake` | 新增 `t113_glibc` 分支：把编译器指向板内包装脚本 |
| `platform/LINUX/toolchain_file.cmake` | 新增 `t113_glibc` 分支：`CMAKE_SYSTEM_PROCESSOR = arm` |
| `platform/LINUX/platform_prepare.py` | 新增 `t113_glibc` 处理：工具链随板内置，仅修复可执行权限 |

### 新增（工程配置）

- `apps/tuya.ai/your_chat_bot/config/t113_glibc.config`：该 Demo 在 T113 上的验证配置。

---

## 2. 快速编译

```bash
cd /path/to/TuyaOpen
. ./export.sh                       # 激活环境（创建/复用 .venv，导出 tos.py）

cd apps/tuya.ai/your_chat_bot
mkdir -p .cache && touch .cache/.dont_prompt_update_platform   # 非交互环境避免平台更新提示

# 选择 T113 配置（任选其一）
cp config/t113_glibc.config app_default.config   # 方式 A：直接套用预置配置
# tos.py config choice                            # 方式 B：交互式选择（需要 TTY）

tos.py build
```

编译产物（交叉编译出的 ARM ELF）：

```
apps/tuya.ai/your_chat_bot/dist/your_chat_bot_1.0.1/your_chat_bot_1.0.1.elf
```

验证架构：

```bash
file dist/your_chat_bot_1.0.1/your_chat_bot_1.0.1.elf
# ELF 32-bit LSB executable, ARM, EABI5, hard-float ABI,
# dynamically linked, interpreter /lib/ld-linux-armhf.so.3
```

> 重新选择配置或修改 `app_default.config` 后若改动未生效，执行 `tos.py clean -f` 再 `tos.py build`（`using.config` 缓存只在全量清理或 `config choice` 时刷新）。

---

## 3. 工具链：移植中最关键的部分

T113 用的是厂商的 **sunxi OpenWrt glibc** 工具链（`arm-openwrt-linux-gnueabi-gcc` 6.4.1，armv7‑a，硬浮点）。它和 SDK 里其它板子用的 ARM GNU 工具链不同，有 3 个“坑”，都通过板内的**包装脚本** `toolchain_wrapper/t113-gcc` 解决：

1. **可重定位（relocatable）sysroot**：头文件在 `<toolchain>/include`、库在 `<toolchain>/lib`，没有标准的 `usr/` 布局。包装脚本注入：
   - `-isystem <toolchain>/include`（头文件）
   - `-L<toolchain>/lib`（库）
   - `-B<toolchain>/lib/`（`crt1.o`/`crti.o` 等启动文件）
2. **驱动调用裸 `as`/`ld`**：gcc 默认去找 `as`，会误用宿主机 x86 的汇编器报 `invalid -march=armv7-a`。包装脚本注入 `-B<toolchain>/bin/arm-openwrt-linux-gnueabi-` 指向自带汇编器/链接器。
3. **`STAGING_DIR` 警告**：OpenWrt 版 gcc 未设置 `STAGING_DIR` 会告警，脚本里 `export STAGING_DIR` 消除之。

此外包装脚本还加了 `-Ulinux`，并提供 `compat-include`：

- `-Ulinux`：gcc 6.4 会把遗留的预定义宏 `linux`（=1）在 `__has_include(<linux/...>)` 里展开成 `1`，导致 `tkl_spi.c` 误判 `<linux/spi/spidev.h>` 不存在而走有 bug 的 fallback。`-Ulinux` 取消该宏（标准代码应使用 `__linux__`）。
- `compat-include/linux/gpio.h`：这套老内核头缺少 GPIO 字符设备 UAPI，`tkl_gpio.c` 直接 `#include <linux/gpio.h>` 会失败；补一份与上游内核 ABI 一致的头即可。

> 包装脚本用**相对自身位置**解析工具链路径（不写死绝对路径），所以整个 `boards/LINUX/t113_glibc/` 目录可以连同工具链一起拷到任意路径/任意机器使用。

### 工具链权限

通过压缩包解压可能丢失可执行位。`platform_prepare.py` 的 `prepare_t113_glibc()` 会在每次构建前自动给 `toolchain/bin` 与 `toolchain_wrapper` 下的文件补上可执行权限；手动修复：

```bash
chmod -R u+rwX boards/LINUX/t113_glibc/toolchain-sunxi-glibc/toolchain/bin
chmod +x boards/LINUX/t113_glibc/toolchain_wrapper/*
```

> 解压工具链请务必用 `tar xzf t113_glibc.tar.gz`，它包含 93 个符号链接；用不保留符号链接的工具（部分图形界面解压器、跨 Windows 盘符）会破坏工具链。

---

## 4. 部署到真实 T113 板子

1. 把 ELF 拷贝到板子（NFS / scp / U 盘）：
   ```bash
   scp dist/your_chat_bot_1.0.1/your_chat_bot_1.0.1.elf root@<板子IP>:/root/
   ```
2. 运行依赖：板子 rootfs 需要 **glibc**（与工具链 glibc 2.23 兼容）、`libpthread`、`libm`、`libstdc++`。如果板子 rootfs 没有 `libstdc++.so.6`，从工具链 `toolchain/lib/` 拷过去并设置 `LD_LIBRARY_PATH`。
3. 设备授权（必须）：TuyaOpen 设备需要授权码才能连云。两种方式：
   - 在 `tuya_config.h` 里填入 `TUYA_OPENSDK_UUID` / `TUYA_OPENSDK_AUTHKEY`（参考 `apps/.../include/tuya_config.h` 与“设备授权”技能）；
   - 或运行时通过授权流程写入。
4. 运行：
   ```bash
   ./your_chat_bot_1.0.1.elf
   ```
   日志默认走标准输出（`CONFIG_TKL_UART_REDIRECT_LOG_TO_STDOUT=y`）。

---

## 5. 已知限制（重要）

当前 `t113_glibc.config` 是一个**可编译、可联网运行的最小骨架**，受限于 armv7 没有现成的厂商预编译库：

| 功能 | 状态 | 说明 |
|------|------|------|
| 云连接 / MQTT / AI 文本链路 | ✅ 正常 | 纯软件，交叉编译通过 |
| 板级 GPIO/I2C/SPI/UART 适配 | ✅ 编译通过 | SPI 用真实 `spidev`，GPIO 用补充的 UAPI 头 |
| 本地 VAD（静音检测） | ⚠️ 占位 | `tkl_audio_ai_placeholder.c` 返回“无语音” |
| 本地 KWS（唤醒词） | ⚠️ 占位 | 无 armv7 版 MNN，唤醒词不可用 |
| ALSA 音频采集/播放 | ❌ 未开启 | 工具链 sysroot 无 `libasound` |

> 这意味着：默认产物适合验证**编译链路 + 云端链路**。要做完整语音对话，见第 6 节补齐音频。

---

## 6. 把 Demo 完整移植到你自己的 T113 板

### 6.1 打开音频（ALSA）

1. 准备 armv7 的 `libasound`（交叉编译 alsa-lib，或从板子 rootfs 取 `libasound.so*` 与头文件），放到工具链 sysroot：
   ```
   boards/LINUX/t113_glibc/toolchain-sunxi-glibc/toolchain/include/alsa/...
   boards/LINUX/t113_glibc/toolchain-sunxi-glibc/toolchain/lib/libasound.so*
   ```
2. 在 `config/t113_glibc.config` 增加：
   ```
   CONFIG_ENABLE_AUDIO_ALSA=y
   CONFIG_ALSA_DEVICE_CAPTURE="plughw:0,0"
   CONFIG_ALSA_DEVICE_PLAYBACK="default"
   CONFIG_AUDIO_CODEC_NAME="alsa_audio"
   ```
   `board_com_api.c` 已带条件编译，开启后会自动注册 ALSA 设备。

### 6.2 打开本地 VAD / 唤醒词（KWS）

需要 armv7 版的 `libaudio_subsys` / `libMNN` / `libopus`（Tuya 目前只提供了 aarch64 版给 DshanPi/树莓派）。拿到后：

1. 新建目录并放入对应库与头：
   ```
   platform/LINUX/tuyaos_adapter/src/tkl_audio/libs/t113_glibc/
       audio_subsys/  (libaudio_subsys.* + include/)
       MNN/           (libMNN.*        + include/)
       opus/          (libopus.*       + include/)
       alsa/          (libasound.*     + include/)
   ```
   （目录名必须等于 `PLATFORM_CHIP`，即 `t113_glibc`。）
2. 在配置里开启音频：`CONFIG_ENABLE_AUDIO=y`（或在板级 `Kconfig` 的 `BOARD_CONFIG` 里 `select ENABLE_AUDIO`）。
   此时会编译真实的 `platform/LINUX/.../tkl_vad.c` / `tkl_kws.c`，**自动取代** `tkl_audio_ai_placeholder.c`（占位实现仅在符号缺失时兜底）。
3. 放置 KWS 模型并指向：
   ```
   CONFIG_KWS_MODEL_PATH="~/tuyaopen_models/mdtc_chunk_300ms.mnn"
   CONFIG_KWS_MODEL_TOKEN_PATH="~/tuyaopen_models/tokens.txt"
   ```

### 6.3 适配你自己的板级硬件

- 在 `board_com_api.c` 的 `board_register_hardware()` 里注册你的按键/LED/音频编解码器等。
- 如需按键唤醒，开启板级 `CONFIG_ENABLE_KEYBOARD_INPUT=y` 并参考 `boards/LINUX/DshanPi_A1/tdd_button_keyboard.c` 提供实现（可拷贝到本板目录）。

---

## 7. 新增一块基于本工具链的 T113 衍生板

复制 `boards/LINUX/t113_glibc/` 为新目录，改名 `Kconfig` 里的 `CHIP_CHOICE`/`BOARD_CHOICE`，在 `boards/LINUX/Kconfig` 注册新的 `BOARD_CHOICE_xxx`，并在 `platform/LINUX/compiler_setup.cmake` 增加对应分支（或让新板复用 `t113_glibc` 的工具链路径）。

---

## 8. 排错速查

| 现象 | 原因 | 处理 |
|------|------|------|
| `invalid -march= option: 'armv7-a'` | gcc 调用了宿主机 `as` | 确认走的是 `toolchain_wrapper/t113-gcc`（含 `-B.../bin/arm-openwrt-linux-gnueabi-`） |
| `linux/gpio.h: No such file` | 老内核头缺 GPIO UAPI | 确认 `compat-include` 已被包装脚本 `-isystem` 引入 |
| `unknown type name '__u64'`（spidev） | `__has_include` 被 `linux` 宏污染 | 确认包装脚本带 `-Ulinux` |
| `cannot find crt1.o` | 启动文件路径缺失 | 确认包装脚本带 `-B.../lib/` |
| `undefined reference to tkl_kws_*/tkl_vad_*` | 关了音频又没有占位实现 | 保留 `tkl_audio_ai_placeholder.c`，或开启真实音频库 |
| `Permission denied` 调用 gcc | 解压丢失可执行位 | 重新 `tos.py build`（会自动修复），或手动 `chmod` |
| 配置改了不生效 | `using.config` 缓存 | `tos.py clean -f && tos.py build` |
| `bits/c++config.h: No such file`（编译 C++ STL 时） | 随包的 C++ 头不完整 | 当前 Demo 不编译 STL；如需 C++ STL，请补全工具链 `arm-openwrt-linux-gnueabi/include/c++/6.4.1/` 下的目标子目录头 |
```
