#include "I2S.h"

void I2S_Init( i2s_mode_t MODE, int SAMPLE_RATE,  i2s_bits_per_sample_t BPS) {

  // Initialize I2S for audio output
  i2s_config_t i2s_config_out = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | MODE),
    .sample_rate = SAMPLE_RATE, //16000
    .bits_per_sample = BPS,     //16位
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  //单声道
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_STAND_I2S),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 1024,
  };


  i2s_pin_config_t pin_config = {
    .bck_io_num = i2s_out_bclk.toInt(),
    .ws_io_num = i2s_out_lrc.toInt(),
    .data_out_num = i2s_out_dout.toInt(),
    .data_in_num = -1
  };


  i2s_driver_install(I2S_OUT_PORT, &i2s_config_out, 0, NULL);
  i2s_set_pin(I2S_OUT_PORT, &pin_config);
}


void I2S_Write(uint8_t* data, int numData) {
  //i2s_write_bytes( (const char *)data, numData, portMAX_DELAY);
  size_t bytes_written = 0;
  //注意： 此处必须要用 (int16_t*)data
  i2s_write(I2S_OUT_PORT, (int16_t*)data, numData, &bytes_written, portMAX_DELAY);
}

void I2S_uninstall()
{
  i2s_driver_uninstall(I2S_OUT_PORT);
}

void clearAudio(void) {
  // 清空I2S DMA缓冲区
  i2s_zero_dma_buffer(I2S_OUT_PORT);
  //Serial.print("clearAudio");
}
