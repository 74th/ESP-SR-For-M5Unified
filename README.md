# ESP_SR_M5Unified

Japanese documentation: [`README_ja.md`](README_ja.md)

This library passes audio captured by `M5Unified` (`M5.Mic.record(...)`) into ESP-SR, enabling wake-word detection and voice command recognition on ESP32 (S3/P4).

This library rewrites the ESP-SR library included in [esp32-arduino](https://github.com/espressif/arduino-esp32) so it can be used with M5Unified.

## Tested hardware

- M5Stack CoreS3
- M5Stack AtomS3R & Atomic Echo Base
- M5Stack Ecno S3R

## Demos

- https://x.com/74th/status/2018644006021070944
- https://x.com/74th/status/2020485095518302578
- https://x.com/74th/status/2025868448669298742

## Using with Arduino IDE

### 1. Install Espressif's official ESP32 board package

In Arduino IDE, add the ESP32 board package URL in Preferences → Additional Board Manager URLs.

Example: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

Install "esp32 by Espressif Systems" from Boards Manager.

Install "M5Unified by M5Stack" from Library Manager.

### 2. Install ESP-SR For M5Unified

Download the ZIP from the [Releases page](https://github.com/74th/ESP-SR-For-M5Unified/releases), then install it via Sketch → Include Library → Add .ZIP Library...

### 3. Select board

#### For CoreS3 / official Stack-chan

Select Tools → Board → esp32 → `M5CoreS3`.

#### For AtomS3R / Atomic Echo S3R

Select Tools → Board → esp32 → `M5AtomS3`.

Select Tools → PSRAM → `OPI PSRAM`.

### 4. Configure partition

You need to add a partition for the model.
The settings below use relatively large partition sizes.
If you need more space for non-model partitions, adjust as needed.

#### For CoreS3 / official Stack-chan

Select Tools → Partition Schema → `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)`.

This setting allocates an 8MB partition for model data.

#### For AtomS3R / Atom Echo S3R

Copy [./partitions_esp_sr_8.csv](./partitions_esp_sr_8.csv) into your sketch directory as `partitions.csv`.

This setting allocates 3MB for the model partition and keeps app/SPIFFS partitions smaller.

### 5. Include libraries

In your sketch, add these two libraries (from menu or in code):

- Sketch → Include Library → `M5Unified`
- Sketch → Include Library → `ESP_SR_M5Unified`
- Or add:
  - `#include <M5Unified.h>`
  - `#include <ESP_SR_M5Unified.h>`

> [!NOTE]
> If added from the menu, `esp32-hal-sr-m5.h` may also be inserted automatically. It is not needed, so remove it.

### 6. Implement your sketch

Use `examples/EnglishCommand` or `examples/HiStackChanWakeUpWord` as references.

Also see the implementation guide below.

### 7. Upload

Run upload once.

You may get an error like:
`No such file or directory: '/../sketches/29D8C511C331F3ECE9B67A907A88C7AF/srmodels.bin'`

Copy `srmodels.bin` to the path shown in the error:

- [examples/HiStackChanWakeUpWord/srmodels.bin](examples/HiStackChanWakeUpWord/srmodels.bin): HiStackChan wake-word only
- [examples/EnglishCommand/srmodels.bin](examples/EnglishCommand/srmodels.bin): HiStackChan wake-word + English command

Then run upload again. `srmodels.bin` will be written to the model partition.


## Using with PlatformIO

Each example includes its own `platformio.ini`, so you can use those as reference.

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

You also need a model partition and must write `srmodels.bin` to that partition.
First, copy a partition file into your project as `partitions.csv`:

- For CoreS3 (Flash 16MB): [./partitions_esp_sr_16.csv](./partitions_esp_sr_16.csv)
- For Atom S3R / Atom Echo S3R (Flash 8MB): [./partitions_esp_sr_8.csv](./partitions_esp_sr_8.csv)

Then set `board_build.partitions` in `platformio.ini`:

```ini
board_build.partitions = partitions.csv
```

There is a helper script for flashing `srmodels.bin`: [./scripts/flash_srmodels.py](./scripts/flash_srmodels.py)
Copy this file into your project, then add the following to `extra_scripts`:

```ini
extra_scripts = post:scripts/flash_srmodels.py
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

### When using Atom S3R and Atomic Echo Base

Set `external_speaker.atomic_echo = 1;` in config.

```cpp
auto cfg = M5.config();
cfg.external_speaker.atomic_echo = 1;
M5.begin(cfg);
```

### When using Atom Echo S3R

Set `internal_mic = true;` in config.

```cpp
auto cfg = M5.config();
cfg.internal_mic = true;
M5.begin(cfg);
```

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
- See [docs/build-srmodels.md](docs/build-srmodels.md) for how to build `srmodels.bin`.

### How `srmodels.bin` is flashed in Arduino IDE

When using this library in Arduino IDE, select this partition scheme:

- `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)`

When uploading, use the following procedure to write `srmodels.bin` into the model partition:

1. Run upload once.
2. If you get an error like `No such file or directory: '/../sketches/.../srmodels.bin'`,
   copy `srmodels.bin` into the location shown in the error message.
   - [examples/HiStackChanWakeUpWord/srmodels.bin](examples/HiStackChanWakeUpWord/srmodels.bin): HiStackChan wake-word only
   - [examples/EnglishCommand/srmodels.bin](examples/EnglishCommand/srmodels.bin): HiStackChan wake-word + English command
3. Run upload again. `srmodels.bin` will be written to the model partition.

## Model license

- Wake-word / command models used by this library are from `https://github.com/espressif/esp-sr`.
- Licensing terms of model files depend on the `esp-sr` project.
- Always check the `esp-sr` repository license and related documentation before use or redistribution.

## Included resources

- partitions_esp_sr_16.csv: https://github.com/espressif/arduino-esp32/blob/89bcb9c2c7abb6fc90784cef4a870e0e9ff03579/tools/partitions/esp_sr_16.csv
