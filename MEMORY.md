# Phoenix Hexapod ESP32 — Project Memory

## Current State

- **Branch**: `dev/esp32_Code_C`
- **Target**: ESP32-S3 (ESP-IDF v5.5.1)
- **Toolchain**: `C:/Espressif/tools/xtensa-esp-elf/esp-14.2.0_20241119` (GCC 14.2.0)
- **Python env**: `C:/Espressif/python_env/idf5.5_py3.11_env` (Python 3.11.2)
- **Build**: ✅ 编译通过 (2026-07-07) — `Phoenix_Hexapod_ESP32.bin` 289KB, flash 72% free
- **Last commit**: `8fcabdb` — Documents

## Build Command

VSCode ESP-IDF 扩展的 `idf.py` 不可用（Python 3.13 vs 3.11 不匹配）。手动构建需先设置环境：

```bat
set IDF_PATH=C:\Espressif\frameworks\esp-idf-v5.5.1
set IDF_PYTHON_ENV_PATH=C:\Espressif\python_env\idf5.5_py3.11_env
set PATH=C:\Espressif\python_env\idf5.5_py3.11_env\Scripts;C:\Espressif\tools\cmake\3.30.2\bin;C:\Espressif\tools\ninja\1.12.1;C:\Espressif\tools\xtensa-esp-elf\esp-14.2.0_20241119\xtensa-esp-elf\bin;C:\Espressif\tools\idf-git\cmd;C:\Espressif\tools\dfu-util\0.11.0-esp32-20221026\dfu-util;C:\Espressif\tools\ccache\4.10.2;%PATH%
cd /d c:\work\esp-idf\Phoenix_Hexapod_ESP32
python C:\Espressif\frameworks\esp-idf-v5.5.1\tools\idf.py build
```

> ⚠️ `&&` 前不能有空格，否则变量值会包含尾部空格。

## Architecture

```
components/
├── Phoenix/          # 核心运动学 (BodyIK, Gait, IK)
├── PCA9685/          # I2C PWM 驱动 (前向声明避免 esp_driver_i2c 传递依赖)
├── Servo/            # 舵机抽象层
├── PS2/              # PS2 手柄 (SPI)
└── SerialProtocol/   # 上位机串口协议 (0xFE 0xFE 帧)
```

## Known Issues

1. **构建环境**: `export.bat` 因 Python 版本不匹配无法直接使用，需手动设置 PATH
2. **GCC ICE**: `esp_lcd` 组件的 `esp_lcd_panel_rgb.c` 在 `-Og` 优化时触发 GCC 14.2.0 SegFault — 通过 `EXCLUDE_COMPONENTS` 排除
3. **ESP_ROM_ELF_DIR**: 构建时非致命警告，gdbinit 生成失败，不影响固件

## Reference Docs

- [开发记录-2026-07-06](Docs/开发记录-2026-07-06.md) — 初版软件实现
- [开发记录-2026-07-07](Docs/开发记录-2026-07-07.md) — 编译修复与构建验证
