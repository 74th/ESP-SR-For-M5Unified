# ESP_SR_M5Unified

M5Unified の `M5.Mic.record(...)` で取得した音声を ESP-SR に渡して、ESP32(S3/P4) でウェイクワード検出・音声コマンド認識を使うためのライブラリです。

[esp32-arduino](https://github.com/espressif/arduino-esp32)に含まれる ESP-SR ライブラリを M5Unified で使えるようにリライトしています。

## 動作確認済み

- M5Stack CoreS3
- M5Stack AtomS3R & Atomic Echo Base
- M5Stack Ecno S3R

## デモ

- https://x.com/74th/status/2018644006021070944
- https://x.com/74th/status/2020485095518302578

## Arduino IDE で使う

### Espressif 公式の ESP32 ボード定義のインストール

Arduino IDEの「基本設定」の「追加のボードマネージャのURL」に、ESP32 ボード定義の URL を追加します。

例: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

ボードマネージャから "esp32 by Espressif Systems" をインストールします。

ライブラリマネージャから "M5Unified by M5Stack" をインストールします。

### ESP-SR For M5Unified のインストール

[このリポジトリのReleaseページ](https://github.com/74th/ESP-SR-For-M5Unified/releases) から ZIP ファイルをダウンロードしてメニューの「スケッチ」→「ライブラリをインクルード」→「.ZIP形式のライブラリをインストール」でインストールします。

### ボードの選択

#### CoreS3、公式スタックチャンの場合

メニューの「ツール」→「ボード: xxx」→「esp32」→「M5CoreS3」を選択します。

#### AtomS3R、Atomic Echo S3R の場合

メニューの「ツール」→「ボード: xxx」→「esp32」→「M5AtomS3」を選択します。

メニューの「ツール」→「PSRAM: xxx」→「OPI PSRAM」を選択します。

### パーティションの設定

model パーティションを追加する必要があります。
以下の設定は大きめのパーティションを設定しています。
model 以外のパーティションのサイズが必要な場合、適宜調整してください。

#### CoreS3、公式スタックチャンの場合

メニューの「ツール」→「Partition Schema」→「ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)」を選択します。

この設定は、model用に8MBのパーティションを設定します。

#### AtomS3R、Atom Echo S3R の場合

スケッチが置かれているディレクトリに、[./partitions_esp_sr_8.csv](./partitions_esp_sr_8.csv) を、`partitions.csv` としてコピーしてください。

この設定は、modelのパーティションに3MBを設定し、app、spiffsのパーティションを小さめに設定します。

### ライブラリのインクルード

スケッチに、次の2つのライブラリを追加してください（メニュー操作またはコード追加）。
- メニュー「スケッチ」→「ライブラリをインクルード」→「M5Unified」
- メニュー「スケッチ」→「ライブラリをインクルード」→「ESP_SR_M5Unified」
- もしくは、コードに以下を追加:
    - `#include <M5Unified.h>`
    - `#include <ESP_SR_M5Unified.h>`

> [!NOTE]
> メニューから追加すると、`esp32-hal-sr-m5.h` も追加されますが、必要ではないため、削除してください。

### 実装

`examples/EnglishCommand` または `examples/HiStackChanWakeUpWord` を参考に組み込みます。

後述の実装方法も確認ください。

### 書き込み

一度「書き込み」を実行してください。

エラーになり、メッセージに `No such file or directory: '/../sketches/29D8C511C331F3ECE9B67A907A88C7AF/srmodels.bin'` のように表示されます。
この場所に、srmodels.bin をコピーしておいてください。

- [examples/HiStackChanWakeUpWord/srmodels.bin](examples/HiStackChanWakeUpWord/srmodels.bin): HiStackChanのWakeUpWordのみ
- [examples/EnglishCommand/srmodels.bin](examples/EnglishCommand/srmodels.bin): HiStackChanのWakeUpWord + English Command

その後、再度「書き込み」を実行してください。
今度はエラーにならず、書き込み後に `srmodels.bin` がモデル領域へ書き込まれます。

## PlatformIO で使う

それぞれの example に platformio.ini を用意していますので、そちらの内容を参考にしてください。

`ESP_SR` を使うため、`platform` は以下を指定してください。

```ini
platform = https://github.com/pioarduino/platform-espressif32.git#55.03.30-2
```

`platformio.ini` の `lib_deps` に以下のいずれかを追加します。

```ini
lib_deps =
    m5stack/M5Unified@^0.2.10
    https://github.com/74th/ESP-SR-For-M5Unified.git
```

modelを置くパーティションを作り、そこに `srmodels.bin` を書き込む必要があります。
まず、パーティションファイルをプロジェクト内に `partitions.csv` としてコピーしてください。

- CoreS3用(Flash 16MB): [./partitions_esp_sr_16.csv](./partitions_esp_sr_16.csv)
- Atom S3R/Atom Echo S3R用(Flash 8MB): [./partitions_esp_sr_8.csv](./partitions_esp_sr_8.csv)

platformio.ini にて、以下のように `board_build.partitions` に、コピーしたパーティションファイルを指定してください。

```ini
board_build.partitions = partitions.csv
```

`srmodels.bin` を書き込むスクリプトが [./scripts/flash_srmodels.py](./scripts/flash_srmodels.py) にあります。
このファイルをプロジェクト内にコピーしてください。
そして、`extra_scripts` に以下を追加してください。

```ini
extra_scripts = post:scripts/flash_srmodels.py
```

### AtomS3Rを利用する場合



## Examples

- PlatformIO 用:
  - `examples/EnglishCommand_platformio`
  - `examples/HiStackChanWakeUpWord_platformio`
- Arduino IDE 用:
  - `examples/EnglishCommand/EnglishCommand.ino`
  - `examples/HiStackChanWakeUpWord/HiStackChanWakeUpWord.ino`

> ![NOTE]
> EnglishCommandのモデルは2.4MB程度あるため、Flash

## 実装方法

`examples` と同じ実装フローは以下です。

1. `M5Unified.h` と `ESP_SR_M5Unified.h` をインクルード
2. `M5.Mic` を 16kHz で初期化
3. `ESP_SR_M5.onEvent(...)` でイベントコールバックを登録
4. `ESP_SR_M5.begin(...)` で開始
   - WakeWordのみ: `ESP_SR_M5.begin(nullptr, 0, SR_MODE_WAKEWORD, SR_CHANNELS_MONO)`
   - WakeWord + Command: `sr_cmd_t` 配列を渡して開始
5. `loop()` で `M5.Mic.record(...)` の結果を `ESP_SR_M5.feedAudio(...)` に渡し続ける
6. コールバックで `SR_EVENT_WAKEWORD` / `SR_EVENT_COMMAND` / `SR_EVENT_TIMEOUT` を処理し、必要に応じて `ESP_SR_M5.setMode(...)` でモードを切り替える

最小構成（WakeWordのみ）の例:

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

詳しくは以下を参照してください。

- WakeWordのみ: `examples/HiStackChanWakeUpWord/HiStackChanWakeUpWord.ino`
- WakeWord + Command: `examples/EnglishCommand/EnglishCommand.ino`

### Atom S3R と Atomic Echo Base を利用する場合

config に `external_speaker.atomic_echo = 1;` を設定する必要があります。

```cpp
auto cfg = M5.config();
cfg.external_speaker.atomic_echo = 1;
M5.begin(cfg);
```

### Atom Echo S3R を利用する場合

config に `internal_mic = true;` を設定する必要があります。

```cpp
auto cfg = M5.config();
cfg.internal_mic = true;
M5.begin(cfg);
```
## srmodels.bin

ESP-SR モデルは `srmodels.bin` を使用します。

- このリポジトリには、各 example 用の `srmodels.bin` を同梱しています。
  - `examples/HiStackChanWakeUpWord/srmodels.bin`
  - `examples/EnglishCommand/srmodels.bin`
  - `examples/HiStackChanWakeUpWord_platformio/srmodels.bin`
  - `examples/EnglishCommand_platformio/srmodels.bin`
- モデル内容は example ごとに異なります。
  - `HiStackChanWakeUpWord` 用: HiStackChan の Wake Word モデル
  - `EnglishCommand` 用: HiStackChan の Wake Word + English Command モデル
- PlatformIO examples では `srmodels.bin` を各 example フォルダに置くと、`scripts/flash_srmodels.py` がアップロード後に `model` パーティションへ書き込みます。
- パーティションは `partitions_esp_sr_16.csv` を使用します。
- 自分のプロジェクトで使う場合は、目的に合う example ディレクトリの `srmodels.bin` をプロジェクト直下へコピーして利用してください。
- `srmodels.bin` のビルド手順は `docs/build-srmodels_ja.md` を参照してください。

### Arduino IDE での `srmodels.bin` 書き込み

このライブラリを Arduino IDE で使う場合は、`Partition Scheme` で必ず以下を選択してください。

- `ESP SR 16M (3MB APP/7MB SPIFFS/2.9MB MODEL)`

書き込み時は、次の手順で `srmodels.bin` をモデル領域へ書き込みます。

1. 一度「書き込み」を実行します。
2. `No such file or directory: '/../sketches/.../srmodels.bin'` のようなエラーが出たら、
  エラーメッセージに表示された場所へ `srmodels.bin` をコピーします。
  - [examples/HiStackChanWakeUpWord/srmodels.bin](examples/HiStackChanWakeUpWord/srmodels.bin): HiStackChan の WakeUpWord のみ
  - [examples/EnglishCommand/srmodels.bin](examples/EnglishCommand/srmodels.bin): HiStackChan の WakeUpWord + English Command
3. 再度「書き込み」を実行すると、`srmodels.bin` がモデル領域へ書き込まれます。

## モデルライセンス

- このライブラリで使う WakeUpWord / Command モデルは、`https://github.com/espressif/esp-sr` に含まれるモデルを利用しています。
- モデルファイルのライセンス条件は `esp-sr` 側の規約・ライセンスに依存します。
- 利用・再配布時は必ず `esp-sr` リポジトリのライセンスと関連ドキュメントを確認してください。

## 含まれるリソース

- partitions_esp_sr_16.csv: https://github.com/espressif/arduino-esp32/blob/89bcb9c2c7abb6fc90784cef4a870e0e9ff03579/tools/partitions/esp_sr_16.csv
