#include "hal.h"
#include <esp_log.h>
#include <nvs.h>
#include <nvs_flash.h>

static const char* TAG = "HAL";

namespace HAL {

// 定义全局的舵机控制指针
SG90Servo* servo_x = nullptr;
SG90Servo* servo_y = nullptr;

/**
 * @brief 初始化两路舵机
 */
static void servo_init() {
    ESP_LOGI(TAG, "Initializing Servos...");
    // 注意：这里的 GPIO 引脚 (GPIO_NUM_1 和 GPIO_NUM_2) 仅为示例。
    // 请根据你 CoreS3 实际连接的扩展引脚 (如 PORT B 或 PORT C) 进行修改。
    // 两个舵机必须使用不同的 LEDC 通道 (CHANNEL_0 和 CHANNEL_1)。
    servo_x = new SG90Servo(GPIO_NUM_1, LEDC_CHANNEL_0, LEDC_TIMER_0);
    servo_y = new SG90Servo(GPIO_NUM_2, LEDC_CHANNEL_1, LEDC_TIMER_0);
    ESP_LOGI(TAG, "Servos initialized successfully.");
}

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

    // ==========================================
    // 2. 初始化外部驱动 (舵机)
    // ==========================================
    servo_init();

    // ==========================================
    // 3. 将硬件功能注册给 MCP
    // ==========================================
    xiaozhi_mcp_init();
}

} // namespace HAL
