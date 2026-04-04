#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <nvs.h>
#include <nvs_flash.h>
// 引入官方的 LVGL 移植层
#include "lvgl.h"
#include "esp_lvgl_port.h"
// 🌟 补坑 1：加入 LCD 面板操作的头文件
#include <esp_lcd_panel_ops.h>
// 你自己的 UI
#include "ui/SmileAvatar.hpp"

// 引入小智的大脑与硬件配置
#include "application.h"
#include "board.h"
#include "config.h" // 包含 DISPLAY_WIDTH 等宏定义

static const char* TAG = "MAIN";

// 🌟 核心：全局企鹅对象。
// 注意这里绝对不能加 static！因为 m5stack_core_s3.cc 里的代理屏幕需要用 extern 找到它
SmileAvatar* my_avatar = nullptr;

// 🌟 核心：接收从 m5stack_core_s3.cc 透传过来的真实屏幕句柄
extern esp_lcd_panel_io_handle_t global_panel_io;
extern esp_lcd_panel_handle_t global_panel;

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== Penguin's Ant Robot Booting ===");
// ==========================================
    // 🌟 第一步：必须最先初始化 NVS (Flash)！
    // ==========================================
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    // ==========================================
    // 1. 小智硬件底座全面接管 (电源、音频、触摸、SPI 屏幕总线)
    // ==========================================
    // 这一步执行完，global_panel_io 和 global_panel 就有值了
    Board& board = Board::GetInstance();

    // ==========================================
    // 2. 画板交接：将屏幕控制权交给 LVGL
    // ==========================================
    if (global_panel_io == nullptr || global_panel == nullptr) {
        ESP_LOGE(TAG, "致命错误：未能获取底层屏幕句柄！请检查透传是否成功。");
        return;
    }

    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    esp_err_t err = lvgl_port_init(&lvgl_cfg);
    if (err != ESP_OK) ESP_LOGE(TAG, "LVGL 初始化失败!");

    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = global_panel_io,
        .panel_handle = global_panel,
        .buffer_size = DISPLAY_WIDTH * DISPLAY_HEIGHT / 10, // 分配 1/10 屏幕的 DMA 缓存
        .double_buffer = true,
        .hres = DISPLAY_WIDTH,
        .vres = DISPLAY_HEIGHT,
        .monochrome = false,
        .rotation = {
            .swap_xy = DISPLAY_SWAP_XY,
            .mirror_x = DISPLAY_MIRROR_X,
            .mirror_y = DISPLAY_MIRROR_Y,
        },
        .color_format = LV_COLOR_FORMAT_RGB565,
        .flags = {
            .buff_dma = true,
            .buff_spiram = false, // 优先使用内部 SRAM 以保证刷屏帧率
        }
    };
    
    // 把底层的硬件配置添加给 LVGL
    lvgl_port_add_disp(&disp_cfg);
// ==========================================
// ==========================================
    // 🌟 补坑 2：强制唤醒 LCD 芯片并拉满背光！(极其重要)
    // ==========================================
    esp_lcd_panel_disp_on_off(global_panel, true);
    Board::GetInstance().GetBacklight()->SetBrightness(100);

    // ==========================================
    // 3. 请企鹅入场
    // ==========================================
    lvgl_port_lock(0); // 操作 LVGL 必须加锁
    my_avatar = new SmileAvatar(lv_scr_act());
    my_avatar->setEmotion(AvatarEmotion::PLEASE); // 刚开机，露出期待的表情
    lvgl_port_unlock();

    // ==========================================
    // 4. 唤醒小智大脑！
    // ==========================================
    ESP_LOGI(TAG, "启动小智大脑中枢...");
    
    // Application::Start() 会自动接管 Wi-Fi 扫描、配网、WebSocket 连接和录音/播放。
    // 当它的状态改变时，会去调用我们的 AvatarBridgeDisplay，从而改变 my_avatar 的表情！
    Application::GetInstance().Start();

    // app_main 到这里就可以结束了，LVGL 的渲染和小智的逻辑都会在它们各自的后台 Task 里跑
}