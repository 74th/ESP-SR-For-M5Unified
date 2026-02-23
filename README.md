# ESP_SR_M5Unified

Japanese documentation: [`README_ja.md`](README_ja.md)

This library passes audio captured by `M5Unified` (`M5.Mic.record(...)`) into ESP-SR, enabling wake-word detection and voice command recognition on ESP32 (S3/P4).

This library rewrites the ESP-SR library included in [esp32-arduino](https://github.com/espressif/arduino-esp32) so it can be used with M5Unified.

- https://x.com/74th/status/2018644006021070944
- https://x.com/74th/status/2020485095518302578

## Tested hardware

- M5Stack CoreS3

## Using with Arduino IDE

1. Add the ESP32 board manager URL in Arduino IDE.
   Example: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. Install "esp32 by Espressif Systems" from Boards Manager.
3. Install "M5Unified by M5Stack" from Library Manager.
4. Download the ZIP from the [Releases page](https://github.com/74th/ESP-SR-For-M5Unified/releases), then install it via:
   - Sketch → Include Library → Add .ZIP Library...
5. Select your board:
   - Tools → Board → esp32 → `M5CoreS3`
6. Select partition scheme:
   - Tools → Partition Scheme → `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)`
7. Open and upload either:
   - `examples/EnglishCommand`
   - `examples/HiStackChanWakeUpWord`
8. In your own sketch, include both libraries (either from menu or by code):
   - Sketch → Include Library → `M5Unified`
   - Sketch → Include Library → `ESP_SR_M5Unified`
   - Or add:
     - `#include <M5Unified.h>`
     - `#include <ESP_SR_M5Unified.h>`
9. Run upload once.
   You may get an error like:
   `No such file or directory: '/../sketches/29D8C511C331F3ECE9B67A907A88C7AF/srmodels.bin'`
   Copy one of the following `srmodels.bin` files into the path shown in the error message:
   - [examples/HiStackChanWakeUpWord/srmodels.bin](examples/HiStackChanWakeUpWord/srmodels.bin): HiStackChan wake-word only
   - [examples/EnglishCommand/srmodels.bin](examples/EnglishCommand/srmodels.bin): HiStackChan wake-word + English command
10. Upload again.
   This time, upload should complete and `srmodels.bin` will be flashed to the model partition.


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

## Implementation Guide

The implementation flow used in `examples` is:

1. Include `M5Unified` and `ESP_SR_M5Unified`.
2. Initialize `M5.Mic` at 16kHz.
3. Register an event callback with `ESP_SR_M5.onEvent(...)`.
4. Start recognition with `ESP_SR_M5.begin(...)`.
   - Wake-word only: `ESP_SR_M5.begin(nullptr, 0, SR_MODE_WAKEWORD, SR_CHANNELS_MONO)`
   - Wake-word + command: pass a `sr_cmd_t` array
5. In `loop()`, continuously pass microphone samples from `M5.Mic.record(...)` to `ESP_SR_M5.feedAudio(...)`.
6. In the callback, handle `SR_EVENT_WAKEWORD` / `SR_EVENT_COMMAND` / `SR_EVENT_TIMEOUT` and switch modes with `ESP_SR_M5.setMode(...)` when needed.

Minimal wake-word-only example:

```cpp
#include <M5Unified.h>
#include <ESP_SR_M5Unified.h>

void onSrEvent(sr_event_t event, int command_id, int phrase_id) {
  (void)command_id;
  (void)phrase_id;
  if (event == SR_EVENT_WAKEWORD) {
    ESP_SR_M5.setMode(SR_MODE_WAKEWORD);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  auto mic_cfg = M5.Mic.config();
  mic_cfg.sample_rate = 16000;
  mic_cfg.stereo = false;
  M5.Mic.config(mic_cfg);
  M5.Mic.begin();

  ESP_SR_M5.onEvent(onSrEvent);
  ESP_SR_M5.begin(nullptr, 0, SR_MODE_WAKEWORD, SR_CHANNELS_MONO);
}

void loop() {
  M5.update();
  static int16_t audio_buf[256];
  if (M5.Mic.record(audio_buf, 256, 16000, false)) {
    ESP_SR_M5.feedAudio(audio_buf, 256);
  }
}
```

See these examples for full implementations:

- Wake-word only: `examples/HiStackChanWakeUpWord/HiStackChanWakeUpWord.ino`
- Wake-word + command: `examples/EnglishCommand/EnglishCommand.ino`

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

## included resouces

- partitions_esp_sr_16.csv: https://github.com/espressif/arduino-esp32/blob/89bcb9c2c7abb6fc90784cef4a870e0e9ff03579/tools/partitions/esp_sr_16.csv
