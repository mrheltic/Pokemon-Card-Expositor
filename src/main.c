#include "stdio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "MAIN"

void app_main() {
    printf("Hello from the Serial port!\n");
    ESP_LOGI(TAG, "This is a log message from ESP-IDF");

    while(1) {
        // Add a delay to avoid flooding the serial port
        vTaskDelay(100);
        printf("Running...\n");
    }
}