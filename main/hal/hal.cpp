#include "hal.h"
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

static const char* TAG = "HAL";

namespace HAL {

void init() {
    ESP_LOGI(TAG, "Initializing Hardware Abstraction Layer...");

    // ==========================================
    // 1. 初始化 NVS (Flash)
    // ==========================================
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs to be erased. Erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized successfully.");

    // TODO: 可以在此处添加更多的整机硬件初始化逻辑
    // 例如：I2C/SPI 总线全局初始化、外部扩展芯片的早期启动等。
}

} // namespace HAL
