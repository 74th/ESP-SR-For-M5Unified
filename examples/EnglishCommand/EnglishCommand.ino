#include <M5Unified.h>
#include <ESP_SR_M5Unified.h>

#define USE_STEREO 0
#define AUDIO_SAMPLE_SIZE 256

static const sr_cmd_t sr_commands[] = {
    {0, "Turn on the light", "TkN nN jc LiT"},
    {0, "Switch on the light", "SWgp nN jc LiT"},
    {1, "Turn off the light", "TkN eF jc LiT"},
    {1, "Switch off the light", "SWgp eF jc LiT"},
    {1, "Go dark", "Gb DnRK"},
    {2, "Start fan", "STnRT FaN"},
    {3, "Stop fan", "STnP FaN"},
};
static constexpr size_t sr_commands_len = sizeof(sr_commands) / sizeof(sr_commands[0]);

void onSrEvent(sr_event_t event, int command_id, int phrase_id)
{
  switch (event)
  {
  case SR_EVENT_WAKEWORD:
    M5.Display.fillScreen(TFT_GREEN);
    M5.Display.setCursor(10, 10);
    M5.Display.setTextSize(3);
    M5.Display.setTextColor(TFT_BLACK, TFT_GREEN);
    M5.Display.println("Command?");
    ESP_SR_M5.setMode(SR_MODE_COMMAND);
    break;

  case SR_EVENT_TIMEOUT:
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(0, 0);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.println("Listening...");
    ESP_SR_M5.setMode(SR_MODE_WAKEWORD);
    break;

  case SR_EVENT_COMMAND:
    M5.Display.fillScreen(TFT_BLUE);
    M5.Display.setCursor(10, 10);
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_BLACK, TFT_BLUE);
    M5.Display.printf("Command: %d\n", command_id);
    if (phrase_id >= 0 && static_cast<size_t>(phrase_id) < sr_commands_len)
    {
      M5.Display.println(sr_commands[phrase_id].str);
      Serial.printf("Command %d detected: %s\n", command_id, sr_commands[phrase_id].str);
    }
    ESP_SR_M5.setMode(SR_MODE_WAKEWORD);
    break;

  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setTextSize(2);
  M5.Display.println("Init...");

  auto mic_cfg = M5.Mic.config();
  mic_cfg.sample_rate = 16000;
  mic_cfg.stereo = USE_STEREO;
  M5.Mic.config(mic_cfg);
  M5.Mic.begin();

  ESP_SR_M5.onEvent(onSrEvent);

  bool success = ESP_SR_M5.begin(
      sr_commands,
      sr_commands_len,
      SR_MODE_WAKEWORD,
      USE_STEREO ? SR_CHANNELS_STEREO : SR_CHANNELS_MONO);

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);

  if (success)
  {
    M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5.Display.println("Listening...");
    M5.Display.setTextSize(1);
    M5.Display.println("Say: 'Hi Stack Chan'");
  }
  else
  {
    M5.Display.setTextColor(TFT_RED, TFT_BLACK);
    M5.Display.println("Init Failed!");
  }
}

void loop()
{
  M5.update();

#if USE_STEREO
  static int16_t audio_buf[AUDIO_SAMPLE_SIZE * 2];
  bool ok = M5.Mic.record(audio_buf, AUDIO_SAMPLE_SIZE, 16000, true);
  if (ok)
  {
    ESP_SR_M5.feedAudio(audio_buf, AUDIO_SAMPLE_SIZE * 2);
  }
#else
  static int16_t audio_buf[AUDIO_SAMPLE_SIZE];
  bool ok = M5.Mic.record(audio_buf, AUDIO_SAMPLE_SIZE, 16000, false);
  if (ok)
  {
    ESP_SR_M5.feedAudio(audio_buf, AUDIO_SAMPLE_SIZE);
  }
#endif
}
