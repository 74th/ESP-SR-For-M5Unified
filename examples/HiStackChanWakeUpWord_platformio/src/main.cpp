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

        delay(1200);

        Serial.println("Listening...");

        if (display_available) {
            M5.Display.fillScreen(TFT_BLACK);
            M5.Display.setCursor(0, 0);
            M5.Display.setTextSize(2);
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.println("Listening...");
        }

        ESP_SR_M5.setMode(SR_MODE_WAKEWORD);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    auto cfg = M5.config();

    if (M5.getBoard() == m5::board_t::board_M5AtomS3R)
    {
        // ATOM S3R with Atomic ECHO BASE
        cfg.external_speaker.atomic_echo = 1;
    }

    M5.begin(cfg);

    display_available = M5.getDisplayCount() > 0;

    if (display_available)
    {
        M5.Display.setRotation(1);
        M5.Display.setTextSize(2);
        M5.Display.println("Init...");
        M5.Display.println(M5.getBoard());
    }
    Serial.println("Init...");
    Serial.println(M5.getBoard());

    delay(2000);

    auto mic_cfg = M5.Mic.config();
    mic_cfg.sample_rate = 16000;
    mic_cfg.stereo = USE_STEREO;
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

        if (display_available) {
            M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
            M5.Display.println("Listening...");
            M5.Display.setTextSize(1);
            M5.Display.println("Say: 'Hi Stack Chan'");
        }
    }
    else
    {
        Serial.println("Init Failed!");

        if (display_available) {
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
    // static int16_t audio_buf[AUDIO_SAMPLE_SIZE];
    // bool ok = M5.Mic.record(audio_buf, AUDIO_SAMPLE_SIZE, 16000, false);
    // if (ok)
    // {
    //     ESP_SR_M5.feedAudio(audio_buf, AUDIO_SAMPLE_SIZE);
    // }

    constexpr size_t kAudioSampleSize = 256;
    static int16_t audio_buf[kAudioSampleSize];
    static uint32_t last_log_time_ = 0;
    static uint32_t loop_count_ = 0;
    static uint32_t error_count_ = 0;

    bool success = M5.Mic.record(audio_buf, kAudioSampleSize, 16000, false);
    if (success)
    {
        ESP_SR_M5.feedAudio(audio_buf, kAudioSampleSize);

        uint32_t now = millis();
        if (now - last_log_time_ >= 1000)
        {
            int32_t sum = 0;
            for (int i = 0; i < 10; i++)
            {
                sum += abs(audio_buf[i]);
            }
            Serial.printf("idle loop: count=%lu, avg_level=%ld, errors=%lu, interval=%lu ms\r\n",
                          static_cast<unsigned long>(loop_count_),
                          static_cast<long>(sum / 10),
                          static_cast<unsigned long>(error_count_),
                          static_cast<unsigned long>(now - last_log_time_));
            last_log_time_ = now;
        }
        loop_count_++;
    }
    else
    {
        error_count_++;
        if (error_count_ % 100 == 0)
        {
            Serial.printf("WARNING: M5.Mic.record failed, count=%lu\r\n", static_cast<unsigned long>(error_count_));
        }
    }

#endif
}
