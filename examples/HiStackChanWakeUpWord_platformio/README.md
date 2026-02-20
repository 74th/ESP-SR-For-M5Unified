# HiStackChanWakeUpWord (PlatformIO)

## 使い方

1. `srmodels.bin` をこのフォルダに置く。
2. `platformio.ini` の `lib_deps` は既定で `../../`（このリポジトリ相対）を参照。
3. `pio run -t upload` で書き込み。

`extra_scripts = post:../../scripts/flash_srmodels.py` により、通常のファーム書き込み後に `srmodels.bin` を model パーティションへ書き込みます。
