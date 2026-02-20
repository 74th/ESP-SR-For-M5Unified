# ESP_SR_M5Unified

> [!CAUTION]
> This repostiroty is work in progress.

Japanese documentation: [`README_ja.md`](README_ja.md)

This library passes audio captured by `M5Unified` (`M5.Mic.record(...)`) into ESP-SR, enabling wake-word detection and voice command recognition on ESP32 (S3/P4).

## Using with Arduino IDE

1. Add the ESP32 board manager URL in Arduino IDE.
   Example: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. Install the ESP32 core from Boards Manager.
3. Download this repository as ZIP and install it via "Add .ZIP Library".
4. Open and upload either:
   - `examples/EnglishCommand`
   - `examples/HiStackChanWakeUpWord`

Note: "Additional Board Manager URLs" in Arduino IDE is for board packages, not standalone libraries.
This library is distributed as an Arduino library (`library.properties`), so use ZIP install (or future Library Manager publication).

## Using with PlatformIO

To use `ESP_SR`, set this platform:

```ini
platform = https://github.com/pioarduino/platform-espressif32.git#55.03.30-2
```

Then add one of these to `lib_deps` in `platformio.ini`:

```ini
lib_deps =
    m5stack/M5Unified@^0.2.10
    https://github.com/74th/ESP-SR-For-M5Unified.git
```

For local relative path usage:

```ini
lib_deps =
    m5stack/M5Unified@^0.2.10
    ../../
```

## Examples

- PlatformIO:
  - `examples/EnglishCommand_platformio`
  - `examples/HiStackChanWakeUpWord_platformio`
- Arduino IDE:
  - `examples/EnglishCommand/EnglishCommand.ino`
  - `examples/HiStackChanWakeUpWord/HiStackChanWakeUpWord.ino`

## srmodels.bin

ESP-SR uses `srmodels.bin`.

- This repository includes per-example `srmodels.bin` files:
  - `examples/HiStackChanWakeUpWord/srmodels.bin`
  - `examples/EnglishCommand/srmodels.bin`
  - `examples/HiStackChanWakeUpWord_platformio/srmodels.bin`
  - `examples/EnglishCommand_platformio/srmodels.bin`
- Model contents differ by example:
  - `HiStackChanWakeUpWord`: HiStackChan wake-word model
  - `EnglishCommand`: HiStackChan wake-word + English command model
- In PlatformIO examples, placing `srmodels.bin` in each example folder allows `scripts/flash_srmodels.py` to flash it to the `model` partition after firmware upload.
- Partition table used there: `partitions_esp_sr_16.csv`.
- For your own project, copy the appropriate `srmodels.bin` from the example directory into your project root.
- See `docs/build-srmodels.md` for how to build `srmodels.bin`.

### How `srmodels.bin` is flashed in Arduino IDE

When using this library in Arduino IDE, select this partition scheme:

- `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)`

In `arduino-esp32` (3.3.6), `srmodels.bin` is handled roughly as follows:

1. A build hook copies `srmodels.bin` into the build output when `ESP_SR` library is in use
   (`platform.txt`: `recipe.hooks.objcopy.postobjcopy.2.pattern`)
2. On upload, selecting `Partition Scheme = esp_sr_16` adds this upload flag:
   `0xD10000 {build.path}/srmodels.bin`
   and flashes it to the model region
   (`boards.txt`: `*.menu.PartitionScheme.esp_sr_16.upload.extra_flags`)

This repository assumes the `ESP SR 16M` partition scheme for Arduino IDE usage.
If you get an error saying `srmodels.bin` is missing, confirm that Arduino core's `ESP_SR` library is included in the build.

## Model license

- Wake-word / command models used by this library are from `https://github.com/espressif/esp-sr`.
- Licensing terms of model files depend on the `esp-sr` project.
- Always check the `esp-sr` repository license and related documentation before use or redistribution.
