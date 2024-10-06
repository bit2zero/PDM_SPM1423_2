#include <M5Unified.h>
#include <M5GFX.h>
#include <driver/i2s.h>

#define I2S_PIN_WS  1 // for Serial Clock
#define I2S_PIN_DIN 2 // for data
#define SAMPLE_RATE 16000

const int sampleCount = 320;   // 一度に取得するサンプル数
int16_t samples[sampleCount];  // サンプルを保存するバッファ

M5GFX DispBuff;
M5Canvas canvas(&DispBuff);

void setup() {
  auto cfg = M5.config();
  cfg.internal_mic = false;  // 内蔵マイクの使用を指定
  M5.begin(cfg);

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S, // default MSB
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = sampleCount,
    .use_apll = false, // no use clock source APLL
    .tx_desc_auto_clear = false, // 0 clear at no send buffer
    .fixed_mclk = 0 // for using APLL
  };

  // Initialize I2S
  esp_err_t _err;
  _err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (_err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", _err);
    while (true);
  }

  if(!cfg.internal_mic) {
    // I2S Pins configuration
    i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_PIN_NO_CHANGE,
      .ws_io_num = I2S_PIN_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_PIN_DIN
    };
    _err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (_err != ESP_OK) {
      Serial.printf("Failed setting pin: %d\n", _err);
      while (true);
    }
  }
  
  i2s_zero_dma_buffer(I2S_NUM_0);

  DispBuff.begin();
  DispBuff.fillScreen(BLACK);

  delay(1000);
}


void loop() {
  M5.update();

  size_t bytes_read;
  i2s_read(I2S_NUM_0, (void*)samples, sizeof(samples), &bytes_read, portMAX_DELAY);

  DispBuff.startWrite();

  canvas.createSprite(320, 240);
  for (int i = 0; i < sampleCount - 1; i++) {
    int y1 = map(samples[i], -32768, 32767, 0, 240);    // サンプル値を画面のY座標にマッピング
    int y2 = map(samples[i + 1], -32768, 32767, 0, 240); // 次のサンプルのY座標

    canvas.drawLine(i, y1, i + 1, y2, GREEN);
  }
  canvas.pushSprite(0, 0);

  DispBuff.endWrite();

  delay(10);
}
