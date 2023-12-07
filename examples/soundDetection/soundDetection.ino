/**
 * @file      soundDetection.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-11-04
 *
 */
#include <Arduino.h>
#include <Wire.h>
#include "driver/i2s.h"
#include "esp_vad.h"
#include "select_pins.h"

#if !defined(IIS_WS) || !defined(IIS_DOUT) || !defined(IIS_SCK)
#error "Board not support."
#endif


#define VAD_SAMPLE_RATE_HZ              16000
#define VAD_FRAME_LENGTH_MS             30
#define VAD_BUFFER_LENGTH               (VAD_FRAME_LENGTH_MS * VAD_SAMPLE_RATE_HZ / 1000)
#define I2S_CH                          I2S_NUM_0

size_t bytes_read;
uint8_t status;
int16_t *vad_buff;
vad_handle_t vad_inst;


void setup()
{

    Serial.begin(115200);

    //Start while waiting for Serial monitoring
    while (!Serial);

    delay(3000);

    Serial.println();


    /*********************************
     *  step 1 : Initialize i2s device
    ***********************************/
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
        .dma_buf_count = 3,
        .dma_buf_len = 300,
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = IIS_SCK,
        .ws_io_num  = IIS_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = IIS_DOUT
    };

    i2s_driver_install(I2S_CH, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_CH, &pin_config);
    i2s_zero_dma_buffer(I2S_CH);

    /*********************************
     *  step 2 : Initialize multinet
    ***********************************/

    vad_inst = vad_create(VAD_MODE_0);
    vad_buff = (int16_t *)malloc(VAD_BUFFER_LENGTH * sizeof(short));
    if (vad_buff == NULL) {
        Serial.println("Memory allocation failed!");
        while (1) {
            delay(1000);
        }
    }
}

void loop()
{
    i2s_read(I2S_CH, (char *)vad_buff, VAD_BUFFER_LENGTH * sizeof(short), &bytes_read, portMAX_DELAY);
    // Feed samples to the VAD process and get the result
    vad_state_t vad_state = vad_process(vad_inst, vad_buff, VAD_SAMPLE_RATE_HZ, VAD_FRAME_LENGTH_MS);
    if (vad_state == VAD_SPEECH) {
        Serial.print(millis());
        Serial.println(":Speech detected");
    }
}

