#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MICROPHONE";

void app_main()
{
    // Configure ADC peripheral
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_0);

    // Read from microphone
    while (true) {
        int data = adc1_get_raw(ADC1_CHANNEL_4);
        if(data != 0){
            ESP_LOGI(TAG, "Data: %d", data);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}