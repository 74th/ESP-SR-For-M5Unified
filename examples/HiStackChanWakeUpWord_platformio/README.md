# ESP-SR を使って M5Stack CoreS3 で Wake Up Words を実現

[ESP32 Arduino の ESP_SR ライブラリ](https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP_SR)を改変し、

[lib/ESP_SR_M5Unified](lib/ESP_SR_M5Unified) を作成し、M5Unifiedの`M5.Mic.record(...)` で取得した音声データを ESP-SR 側に渡して、Wake Up Words を実現する。

## TODO

- CommandModeが動いてないっぽい

## PlatformIOの準備

https://github.com/3110/m5stack-platformio-boilerplate-code の設定を使用

esp32s3でもplatform-pioarduinoを使用するように、platformio-m5stack.ini の`[esp32s3]` セクションを以下を追加。

```ini
[esp32s3]
extends = platform-pioarduino
```

[platformioで音声認識モデルを書き込むプログラム作成(ChatGPTが作った）scripts/flash_srmodels.py](./scripts/flash_srmodels.py)

パーティションファイルは、以下から、`partitions_esp_sr_16.csv` として保存

https://github.com/espressif/arduino-esp32/blob/master/tools/partitions/esp_sr_16.csv

パーティションと、スクリプトを指定した platformio-m5stack.ini に変更

```ini
[platformio]
default_envs = m5stack-cores3-m5unified

[env]
monitor_speed = 115200
monitor_filters = esp32_exception_decoder, time
upload_speed = 1500000

build_flags =
lib_deps =

lib_ldf_mode = deep

platform = arduino
board_build.embed_files =

board_build.partitions = partitions_esp_sr_16.csv
extra_scripts = post:scripts/flash_srmodels.py
```

## srmodels.bin を作る

srmodels.bin はESP-SR用の音声認識モデルをまとめたバイナリファイル。
モデルを選択して、1バイナリにまとめる必要がある。

手順

https://github.com/espressif/esp-sr/tree/master/docs/en/flash_model

なお、Arduino IDEでは以下のディレクトリに、Hi,ESP のモデルのバイナリがある。

> ~/Library/Arduino15/packages/esp32/tools/esp32s3-libs/3.3.6/esp_sr/srmodels.bin

esp-idf の menuconfig 中に機能があり、esp-idfプロジェクトが必要

esp-sr のクローン

```bash
cd ~/ghq/github.com/espressif

git clone https://github.com/espressif/esp-sr.git
```

idf.py のインストール

```bash
cd ~/ghq/github.com/espressif

git clone https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.5.2
./install.sh

# このシェルで有効化
. ./export.sh
```

esp-idfプロジェクトの作成

```bash
# ESP-IDFのプロジェクトを作る
cd ~/tmp
idf.py create-project sr_model_proj
cd sr_model_proj

# ESP32S3ターゲットに切り替え
idf.py set-target esp32s3

# ESP-SRコンポーネントを設置
mkdir components
ln -s ~/ghq/github.com/espressif/esp-sr components/
```

menuconfig を起動

```
idf.py menuconfig
```

- `ESP Speech Recognition` を選択(Enter)
- `Load Multiple Wake Words (WakeNet9)` を選択(Enter)
- `Hi,Stack Chan (wn9_histackchan_tts3)` にチェックを付ける(Enter)
- ESCで戻り、`English Speech Commands Model (english recognition (mn5q8_en))` を選択(Enter)
- `english recognition (mn5q8_en)` にチェックを付ける(Enter)
- `S` で保存、`Q` で終了

> [!IMPORTANT]
> WakeUpWordしか利用しない場合、 English Speech Commands Model は不要です。Arduino ESP_SRライブラリでは、English Speech Commands Model が無いとパニックを起こしますが、この実装では引数のsr_commands_len が0の場合は、音声認識モデルを読み込まないようにしています。

sdkconfig の生成？

```
idf.py build
```

srmodels.bin のビルド

```bash
ESP_SR_PATH=~/ghq/github.com/espressif/esp-sr
SDKCONFIG_PATH=~/tmp/sr_model_proj/sdkconfig
python ${ESP_SR_PATH}/model/movemodel.py -d1 ${SDKCONFIG_PATH} -d2 ${ESP_SR_PATH} -d3 ./build/
```

モデルをプロジェクト内にコピー

```
cp build/srmodels.bin ./
```

## 描き込み後、実行

Listening... と表示されるが、ウェイクアップワードを検出すると表示される。

https://x.com/74th/status/2018644006021070944

モデルが読み込まれると、シリアルのログにそのモデルが表示される。

```
21:16:52.589 > Starting ESP_SR_M5...
21:16:52.744 > MC Quantized wakenet9: wakenet9_tts3h12_Hi,Stack Chan_3_0.585_0.590, tigger:v4, mode:4, p:0, (Dec 16 2025 14:45:55)
21:16:53.162 > Quantized8 Multinet5: MN5Q8_v2_english_8_0.9_0.90, beam search:v2, (Dec 16 2025 14:45:55)
```
