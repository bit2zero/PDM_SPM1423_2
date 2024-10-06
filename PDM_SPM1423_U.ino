#include <M5Unified.h>
#include <M5GFX.h>
#include <driver/i2s.h>

#define I2S_PIN_WS  1 // for Serial Clock
#define I2S_PIN_DIN 2 // for data
#define SAMPLE_RATE 16000

M5GFX DispBuff;
M5Canvas canvas(&DispBuff);

void i2sInit()
{
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
    .sample_rate =  SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
    .communication_format = (i2s_comm_format_t)I2S_COMM_FORMAT_I2S, // default MSB
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false, // no use clock source APLL
    .tx_desc_auto_clear = true, // 0 clear at no send buffer
    .fixed_mclk = 0 // for using APLL
  };

    // I2S Pins configuration
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_PIN_NO_CHANGE,
    .ws_io_num = I2S_PIN_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_PIN_DIN
  };

  // Initialize I2S
  esp_err_t _err;
  _err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (_err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", _err);
    while (true);
  }
  _err = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (_err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", _err);
    while (true);
  }
  // i2s_zero_dma_buffer(I2S_NUM_0);
  Serial.println("I2S driver installed.");
}

void mic_record_task()
{
  int16_t i2s_read_buffer[320];
  size_t bytes_read;

  i2s_read(I2S_NUM_0, &i2s_read_buffer, sizeof(i2s_read_buffer), &bytes_read, portMAX_DELAY);

  DispBuff.startWrite();

  canvas.createSprite(320, 320);
  for (int i = 0; i < 320; i++) {
    int16_t _sample = (i2s_read_buffer[i] - 1000);
    float adc_data = (float)map(_sample, -1000, 1000, -160, 160) - 50;

    Serial.printf("%3.1f\n", adc_data);
    canvas.drawLine(i, 130, i, 130 - adc_data * 2, GREEN);
    canvas.drawPixel(i, 130 - adc_data * 2, WHITE);
  }
  canvas.pushSprite(0, 0);

  DispBuff.endWrite();

  delay(10);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(115200);

  i2sInit();

  DispBuff.begin();
  DispBuff.fillScreen(BLACK);

  delay(5000);
}


void loop() {
  M5.update();

  mic_record_task();
}
