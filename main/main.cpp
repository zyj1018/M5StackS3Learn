#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <nvs.h>
#include <nvs_flash.h>
// 引入官方的 LVGL 移植层
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "app/app_manager.h"
#include "app/penguin_app.h"

// 引入小智的大脑与硬件配置
#include "application.h"
#include "board.h"
#include "config.h" // 包含 DISPLAY_WIDTH 等宏定义

#include "hal/hal.h"

static const char* TAG = "MAIN";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== Penguin's Ant Robot Booting ===");
// ==========================================
    // 🌟 第一步：硬件抽象层及基础服务初始化
// ==========================================
    HAL::init();
    // ==========================================
    // 1. 小智硬件底座全面接管 (电源、音频、触摸、SPI 屏幕总线)
    // ==========================================
    Board& board = Board::GetInstance();
    board.Initialize(); // 显式调用 Initialize 初始化屏幕与 LVGL
    
    // ==========================================
    // 2. 初始化 AppManager 并启动默认 App
    // ==========================================
    ESP_LOGI(TAG, "启动应用管理器...");
    AppManager::GetInstance().startApp(new PenguinApp());

    // ==========================================
    // 3. 唤醒小智大脑！
    // ==========================================
    ESP_LOGI(TAG, "启动小智大脑中枢...");
    Application::GetInstance().Start();

    // app_main 到这里就可以结束了，LVGL 的渲染和小智的逻辑都会在它们各自的后台 Task 里跑
}