# ESP_SR_M5Unified

> [!CAUTION]
> このリポジトリはまだ作業中です。

M5Unified の `M5.Mic.record(...)` で取得した音声を ESP-SR に渡して、ESP32(S3/P4) でウェイクワード検出・音声コマンド認識を使うためのライブラリです。

## Arduino IDE で使う

1. Arduino IDE の「追加のボードマネージャのURL」には、ESP32 ボード定義の URL を追加します。
   例: `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
2. ボードマネージャから ESP32 をインストールします。
3. このリポジトリを ZIP ダウンロードして「ライブラリを追加(.ZIP)」でインストールします。
4. `examples/EnglishCommand` または `examples/HiStackChanWakeUpWord` を開いて書き込みます。

注意: Arduino IDE の「追加のボードマネージャURL」は本来ボード定義配布用で、単体ライブラリの直接配布には使えません。
このライブラリは `library.properties` 対応済みなので、ZIP インストールまたはライブラリマネージャ経由公開で利用できます。

## PlatformIO で使う

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

ローカル相対パスなら以下です。

```ini
lib_deps =
    m5stack/M5Unified@^0.2.10
    ../../
```

## Examples

- PlatformIO 用:
  - `examples/EnglishCommand_platformio`
  - `examples/HiStackChanWakeUpWord_platformio`
- Arduino IDE 用:
  - `examples/EnglishCommand/EnglishCommand.ino`
  - `examples/HiStackChanWakeUpWord/HiStackChanWakeUpWord.ino`

## 実装方法

`examples` と同じ実装フローは以下です。

1. `M5Unified` と `ESP_SR_M5Unified` を include
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

`arduino-esp32` (3.3.6) の実装では、概ね次の流れで `srmodels.bin` が書き込まれます。

1. ビルド時フックで、`ESP_SR` ライブラリが使われている場合に `srmodels.bin` をビルド出力へコピー
   (`platform.txt` の `recipe.hooks.objcopy.postobjcopy.2.pattern`)
2. 書き込み時、`Partition Scheme = esp_sr_16` の設定で `upload.extra_flags` に
   `0xD10000 {build.path}/srmodels.bin` が追加され、モデル領域へ書き込み
   (`boards.txt` の `*.menu.PartitionScheme.esp_sr_16.upload.extra_flags`)

このリポジトリでは、Arduino IDE では上記 `ESP SR 16M` パーティションを前提として使用してください。
もし `srmodels.bin` が見つからないエラーになる場合は、Arduino コア付属の `ESP_SR` ライブラリがビルド対象に入っているか確認してください。

## モデルライセンス

- このライブラリで使う WakeUpWord / Command モデルは、`https://github.com/espressif/esp-sr` に含まれるモデルを利用しています。
- モデルファイルのライセンス条件は `esp-sr` 側の規約・ライセンスに依存します。
- 利用・再配布時は必ず `esp-sr` リポジトリのライセンスと関連ドキュメントを確認してください。
