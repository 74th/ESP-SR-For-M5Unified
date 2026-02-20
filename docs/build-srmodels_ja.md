# srmodels.bin の作り方

英語版ドキュメント: [`build-srmodels.md`](build-srmodels.md)

このドキュメントは、以下の調査メモを元に整理したものです。

- https://github.com/74th/test-esp32-arduino/tree/main/20260201-esp_sr

## 概要

`srmodels.bin` は ESP-SR 用の音声認識モデルをまとめたバイナリです。  
利用する Wake Word / Command モデルを選択して 1 つに束ねる必要があります。

参考:

- https://github.com/espressif/esp-sr/tree/master/docs/en/flash_model

## 事前準備

### esp-sr を取得

```bash
cd ~/ghq/github.com/espressif
git clone https://github.com/espressif/esp-sr.git
```

### esp-idf を準備

```bash
cd ~/ghq/github.com/espressif
git clone https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.5.2
./install.sh

# このシェルで有効化
. ./export.sh
```

## モデルビルド用の esp-idf プロジェクトを作る

```bash
cd ~/tmp
idf.py create-project sr_model_proj
cd sr_model_proj
idf.py set-target esp32s3

mkdir components
ln -s ~/ghq/github.com/espressif/esp-sr components/
```

## menuconfig でモデルを選ぶ

```bash
idf.py menuconfig
```

例:

1. `ESP Speech Recognition` を開く
2. `Load Multiple Wake Words (WakeNet9)` を開く
3. `Hi,Stack Chan (wn9_histackchan_tts3)` にチェック
4. 必要なら `English Speech Commands Model (english recognition (mn5q8_en))` を有効化
5. 保存して終了

`sdkconfig` を確定させるために一度ビルドします。

```bash
idf.py build
```

## srmodels.bin を生成

```bash
ESP_SR_PATH=~/ghq/github.com/espressif/esp-sr
SDKCONFIG_PATH=~/tmp/sr_model_proj/sdkconfig
python ${ESP_SR_PATH}/model/movemodel.py -d1 ${SDKCONFIG_PATH} -d2 ${ESP_SR_PATH} -d3 ./build/
```

生成物:

- `./build/srmodels.bin`

必要なプロジェクトへコピーして使用します。

```bash
cp build/srmodels.bin /path/to/your/project/
```

## 補足

- Arduino IDE で `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)` を使う場合、書き込み時に `model` 領域へ `srmodels.bin` がフラッシュされます。
- WakeWord のみ使う場合は、Command モデルを含めない `srmodels.bin` でも運用できます（このライブラリは `sr_commands_len == 0` の利用を想定）。
