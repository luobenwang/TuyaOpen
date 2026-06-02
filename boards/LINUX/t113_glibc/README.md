# Allwinner T113 (glibc) board support

## Why `platform_overlay` exists

`platform/LINUX` is **not** part of the main TuyaOpen tree. It is downloaded from
[ TuyaOpen-ubuntu ](https://github.com/tuya/TuyaOpen-ubuntu) on first `tos.py build`.

T113 changes to `platform_prepare.py`, `compiler_setup.cmake`, `toolchain_file.cmake`,
and `tkl_memory.c` are stored under `platform_overlay/`. Before each build,
`apply_platform_overlay.py` copies them into `platform/LINUX/`.

**You must push these paths to GitHub** (not only `boards/` BSP files):

- `boards/LINUX/t113_glibc/platform_overlay/`
- `boards/LINUX/t113_glibc/apply_platform_overlay.py`
- `tools/cli_command/cli_build.py` (overlay hook)
- `boards/LINUX/Kconfig` (BOARD_CHOICE_T113_GLIBC)
- `boards/LINUX/TKL_Kconfig` (LINUX_USE_LIBC_MALLOC, etc.)

## Toolchain

Either:

1. Use the committed `toolchain-sunxi-glibc/` tree (large), or
2. Extract the archive:

```bash
cd boards/LINUX
tar xzf t113_glibc.tar.gz
chmod +x t113_glibc/toolchain_wrapper/*
chmod -R u+rwX t113_glibc/toolchain-sunxi-glibc/toolchain/bin
```

## Build

```bash
. ./export.sh
cd apps/tuya.ai/your_chat_bot   # or switch_demo
cp config/t113_glibc.config app_default.config
tos.py build
```

See [PORTING_GUIDE_zh.md](./PORTING_GUIDE_zh.md) for full details.
