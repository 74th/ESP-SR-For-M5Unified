#include <M5Unified.h>
#include <ESP_SR_M5Unified.h>

#define USE_STEREO 0
#define AUDIO_SAMPLE_SIZE 256

static bool display_available = false;

void onSrEvent(sr_event_t event, int command_id, int phrase_id)
{
    (void)command_id;
    (void)phrase_id;

    if (event == SR_EVENT_WAKEWORD)
    {
        Serial.println("WakeWord Detected!");
        if (display_available)
        {
            M5.Display.fillScreen(TFT_GREEN);
            M5.Display.setCursor(10, 10);
            M5.Display.setTextSize(3);
            M5.Display.setTextColor(TFT_BLACK, TFT_GREEN);
            M5.Display.println("WakeWord!");
            M5.Display.println("Detected!");
        }
#ifdef ARDUINO_ATOM_ECHOS3R
        digitalWrite(G8, HIGH);
#endif

        delay(1200);

        Serial.println("Listening...");

        if (display_available)
        {
            M5.Display.fillScreen(TFT_BLACK);
            M5.Display.setCursor(0, 0);
            M5.Display.setTextSize(2);
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.println("Listening...");
        }
#ifdef ARDUINO_ATOM_ECHOS3R
        digitalWrite(G8, LOW);
#endif

        ESP_SR_M5.setMode(SR_MODE_WAKEWORD);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    auto cfg = M5.config();

#ifdef ARDUINO_ATOM_ECHOS3R
    cfg.internal_mic = true;
#endif

#ifdef WITH_ECHO_BASE
    // ATOM S3R with Atomic ECHO BASE
    cfg.external_speaker.atomic_echo = 1;
#endif

    M5.begin(cfg);

    display_available = M5.getDisplayCount() > 0;

    Serial.println("Init...");
    if (display_available)
    {
        M5.Display.setRotation(1);
        M5.Display.setTextSize(2);
        M5.Display.println("Init...");
    }
    if (cfg.external_speaker.atomic_echo)
    {
        Serial.println("ECHO BASE");
        if (display_available)
        {
            M5.Display.println("ECHO BASE");
        }
    }

#ifdef ARDUINO_ATOM_ECHOS3R
    pinMode(G8, OUTPUT);
#endif

    delay(2000);

    auto mic_cfg = M5.Mic.config();
    mic_cfg.sample_rate = 16000;
    M5.Mic.config(mic_cfg);
    M5.Mic.begin();

    ESP_SR_M5.onEvent(onSrEvent);

    bool success = ESP_SR_M5.begin(
        nullptr,
        0,
        SR_MODE_WAKEWORD,
        USE_STEREO ? SR_CHANNELS_STEREO : SR_CHANNELS_MONO);

    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(0, 0);

    if (success)
    {
        Serial.println("Listening...");
        Serial.println("Say: 'Hi Stack Chan'");

        if (display_available)
        {
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.println("Listening...");
            M5.Display.setTextSize(1);
            M5.Display.println("Say: 'Hi Stack Chan'");
        }
    }
    else
    {
        Serial.println("Init Failed!");

        if (display_available)
        {
            M5.Display.setTextColor(TFT_RED, TFT_BLACK);
            M5.Display.println("Init Failed!");
        }
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
