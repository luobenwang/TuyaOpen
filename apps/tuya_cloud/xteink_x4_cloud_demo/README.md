# xteink_x4_cloud_demo

XTEINK X4 (ESP32-C3 e-paper) Tuya cloud switch demo.

- **Cloud**: based on [switch_demo](../switch_demo) (Wi-Fi/BLE provisioning, MQTT, DP receive/report)
- **Display**: same LVGL + EPD UI as [boards/ESP32/XTEINK_X4/example/lvgl_demo](../../../boards/ESP32/XTEINK_X4/example/lvgl_demo)
- **Flash**: 16 MB partition layout

## Quick start

1. Create a product on [Tuya IoT Platform](https://iot.tuya.com) and copy the PID into `src/tuya_config.h` (`TUYA_PRODUCT_ID`).
2. Add OpenSDK `uuid` / `authkey` via `src/tuya_config_secrets.h` (copy from `tuya_config_secrets.h.example`) or use pre-burned authorization.
3. Build and flash:

```bash
cd apps/tuya_cloud/xteink_x4_cloud_demo
cp config/XTEINK_X4_cloud_only.config app_default.config   # cloud-only (recommended)
# or: cp config/XTEINK_X4.config app_default.config          # cloud + LVGL UI
tos.py build
tos.py flash monitor
```

**Cloud-only preset** (`XTEINK_X4_cloud_only.config`) disables LVGL/AI and applies ESP32-C3
WiFi/mbedTLS buffer trims so IoT DNS HTTPS (device activation) has enough heap.

4. Use Tuya Smart App to provision the device (BLE + AP). Footer on the EPD dashboard shows cloud status (`Netcfg BLE/AP`, `MQTT connected`, etc.).

## CLI (serial)

Same as switch_demo: `switch on/off`, `netmgr`, `reset`, `mem`, ...

## Files

| File | Description |
|------|-------------|
| `src/tuya_main.c` | Cloud stack + display start |
| `src/xteink_x4_display.c` | LVGL/EPD UI (from lvgl_demo) |
| `config/XTEINK_X4.config` | Board + LVGL + 16M flash preset |
