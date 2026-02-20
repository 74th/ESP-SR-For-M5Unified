# How to build srmodels.bin

Japanese documentation: [`build-srmodels_ja.md`](build-srmodels_ja.md)

This document is based on the following investigation notes:

- https://github.com/74th/test-esp32-arduino/tree/main/20260201-esp_sr

## Overview

`srmodels.bin` is a bundled model binary for ESP-SR.  
You need to select required wake-word / command models and package them into one binary.

Reference:

- https://github.com/espressif/esp-sr/tree/master/docs/en/flash_model

## Prerequisites

### Clone esp-sr

```bash
cd ~/ghq/github.com/espressif
git clone https://github.com/espressif/esp-sr.git
```

### Prepare esp-idf

```bash
cd ~/ghq/github.com/espressif
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.5.2
./install.sh

# Enable ESP-IDF in this shell
. ./export.sh
```

## Create an ESP-IDF project for model build

```bash
cd ~/tmp
idf.py create-project sr_model_proj
cd sr_model_proj
idf.py set-target esp32s3

mkdir components
ln -s ~/ghq/github.com/espressif/esp-sr components/
```

## Select models in menuconfig

```bash
idf.py menuconfig
```

Example selection flow:

1. Open `ESP Speech Recognition`
2. Open `Load Multiple Wake Words (WakeNet9)`
3. Enable `Hi,Stack Chan (wn9_histackchan_tts3)`
4. If needed, enable `English Speech Commands Model (english recognition (mn5q8_en))`
5. Save and exit

Run one build to finalize `sdkconfig`.

```bash
idf.py build
```

## Generate srmodels.bin

```bash
ESP_SR_PATH=~/ghq/github.com/espressif/esp-sr
SDKCONFIG_PATH=~/tmp/sr_model_proj/sdkconfig
python ${ESP_SR_PATH}/model/movemodel.py -d1 ${SDKCONFIG_PATH} -d2 ${ESP_SR_PATH} -d3 ./build/
```

Output:

- `./build/srmodels.bin`

Copy it to your target project:

```bash
cp build/srmodels.bin /path/to/your/project/
```

## Notes

- If you use `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)` in Arduino IDE, `srmodels.bin` is flashed to the `model` partition during upload.
- For wake-word-only usage, a model set without command models can be used (this library supports `sr_commands_len == 0`).
