#include <M5Unified.h>
#include <ESP_SR_M5Unified.h>

#define WAKEWORD_ONLY 1

// ステレオにすると凄く誤検出する？！
// ひとまずモノラルで！
#define USE_STEREO 0

#if WAKEWORD_ONLY
// wakewordだけやりたいならコマンドは 0 個で試す
static const sr_cmd_t *sr_commands = nullptr;
static constexpr size_t sr_commands_len = 0;
#else
// コマンド
static const sr_cmd_t sr_commands[] = {
    {0, "Turn on the light", "TkN nN jc LiT"},
    {0, "Switch on the light", "SWgp nN jc LiT"},
    {1, "Turn off the light", "TkN eF jc LiT"},
    {1, "Switch off the light", "SWgp eF jc LiT"},
    {1, "Go dark", "Gb DnRK"},
    {2, "Start fan", "STnRT FaN"},
    {3, "Stop fan", "STnP FaN"},
};
static constexpr size_t sr_commands_len = 7;
#endif

void onSrEvent(sr_event_t event, int command_id, int phrase_id)
{
  switch (event)
  {
  case SR_EVENT_WAKEWORD:
    M5.Display.fillScreen(TFT_GREEN);
    M5.Display.setCursor(10, 10);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_BLACK, TFT_GREEN);
#if WAKEWORD_ONLY
    M5.Display.println("WakeWord!");
    M5.Display.println("Detected!");
    // 2秒後に自動的にWakeWordモードに戻す
    delay(2000);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.println("Listening...");
    ESP_SR_M5.setMode(SR_MODE_WAKEWORD); // Switch back to WakeWord detection
#else
    M5.Display.println("Command?");
    // コマンドモードへ切り替え
    ESP_SR_M5.setMode(SR_MODE_COMMAND);
#endif
    break;

  case SR_EVENT_TIMEOUT:
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.println("Listening...");
    ESP_SR_M5.setMode(SR_MODE_WAKEWORD); // Switch back to WakeWord detection
    break;

  case SR_EVENT_COMMAND:
    Serial.printf("Command %d Detected! %s\n", command_id, sr_commands[phrase_id].str);
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_BLACK, TFT_BLUE);
    M5.Display.println("Command!");
    M5.Display.println(command_id);
    M5.Display.println(sr_commands[phrase_id].str);
    ESP_SR_M5.setMode(SR_MODE_COMMAND);
    break;

  default:
    Serial.println("Unknown Event!");
    Serial.println(event);
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);
  Serial.println("\n\n==== ESP_SR_M5Unified Test ====");

  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(2);
  M5.Display.println("Init...");

  auto mic_cfg = M5.Mic.config();
  mic_cfg.sample_rate = 16000;
#if USE_STEREO
  mic_cfg.stereo = true;
#else
  mic_cfg.stereo = false;
#endif
  M5.Mic.config(mic_cfg);
  M5.Mic.begin();

  Serial.println("Mic initialized");

  // ESP_SR_M5Unifiedを初期化
  ESP_SR_M5.onEvent(onSrEvent);

  Serial.println("Starting ESP_SR_M5...");

#if USE_STEREO
  bool success = ESP_SR_M5.begin(
      sr_commands,
      sr_commands_len,
      SR_CHANNELS_STEREO
  );
#else
#if WAKEWORD_ONLY
  // モノラル、WakeWordのみ
  bool success = ESP_SR_M5.begin();
#else
  // モノラル、WakeWord&コマンド
  bool success = ESP_SR_M5.begin(
      sr_commands,
      sr_commands_len
  );
#endif
#endif

  Serial.printf("ESP_SR_M5.begin() = %d\n", success);

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);

  if (success)
  {
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.println("Listening...");
    M5.Display.println();
    M5.Display.setTextSize(1);
    M5.Display.println("Say: 'Hi Stack Chan'");
    Serial.println("Ready to listen!");
  }
  else
  {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.println("Init Failed!");
    Serial.println("ERROR: Init Failed!");
  }
}

#define AUDIO_SAMPLE_SIZE 256

void loop()
{
  static bool waiting_wake_up_word = true;

  static uint32_t loop_count = 0;
  static uint32_t error_count = 0;
  static uint32_t last_log_time = 0;

  M5.update();

  // M5.Micから256サンプル(モノラル)を読み取る
#if USE_STEREO
  static int16_t audio_buf[AUDIO_SAMPLE_SIZE * 2];
#else
  static int16_t audio_buf[AUDIO_SAMPLE_SIZE];
#endif

#if USE_STEREO
  // ステレオ、16bit、16kHz
  bool success = M5.Mic.record(audio_buf, AUDIO_SAMPLE_SIZE, 16000, true);
#else
  // モノラル、16bit、16kHz
  bool success = M5.Mic.record(audio_buf, AUDIO_SAMPLE_SIZE, 16000, false);
#endif

  if (M5.BtnA.wasClicked())
  {
    waiting_wake_up_word = !waiting_wake_up_word;
    Serial.println("BtnA clicked ");
    if (waiting_wake_up_word)
    {
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setCursor(0, 0);
      M5.Display.setTextSize(2);
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Display.println("Listening...");
      ESP_SR_M5.resume();
    }
    else
    {
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setCursor(0, 0);
      M5.Display.setTextSize(2);
      M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
      M5.Display.println("zzz...");
      ESP_SR_M5.pause();
    }
  }

  if (!waiting_wake_up_word)
  {
    // ウェイクワード待機モードでないなら音声データを捨てる
    return;
  }

  if (success)
  {
    ESP_SR_M5.feedAudio(audio_buf, AUDIO_SAMPLE_SIZE);


    uint32_t now = millis();
    if (now - last_log_time >= 1000)
    {
      int32_t sum = 0;
      for (int i = 0; i < 10; i++)
      {
        sum += abs(audio_buf[i]);
      }
      Serial.printf("loop: count=%d, avg_level=%d, errors=%d, interval=%dms\n",
                    loop_count, sum / 10, error_count, now - last_log_time);
      last_log_time = now;
    }
    loop_count++;
  }
  else
  {
    error_count++;
    if (error_count % 100 == 0)
    {
      Serial.printf("WARNING: M5.Mic.record failed, count=%d\n", error_count);
    }
  }

  // delay()なし - M5.Mic.record()が適切に待機
}
