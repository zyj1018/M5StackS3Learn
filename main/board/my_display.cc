#include "my_display.h"
#include <esp_log.h>
#include <esp_err.h>
#include <esp_lvgl_port.h>
#include <esp_psram.h>
#include <vector>
#include <cstring>
#include <settings.h>
#include <lvgl.h>
#include <lvgl_theme.h>
#include "app/app_manager.h"

#define TAG "MyDisplay"

MyDisplay::MyDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                                               int width, int height, int offset_x, int offset_y, bool mirror_x,
                                               bool mirror_y, bool swap_xy)
    : LvglDisplay(), panel_io_(panel_io), panel_(panel) {
        width_ = width;
        height_ = height;

    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    esp_err_t err = lvgl_port_init(&lvgl_cfg);
    if (err != ESP_OK) ESP_LOGE(TAG, "LVGL 初始化失败!");
    
        // 🌟 补坑 2：强制唤醒 LCD 芯片并拉满背光！(极其重要)
    // ==========================================
    esp_lcd_panel_disp_on_off(panel_, true);
    
    lv_init();

    
    lvgl_port_display_cfg_t disp_cfg = {
        .io_handle      = panel_io_,
        .panel_handle   = panel_,
        .control_handle = nullptr,
        .buffer_size    = static_cast<uint32_t>(width_ * 20),
        .double_buffer  = true,
        .trans_size     = 0,
        .hres           = static_cast<uint32_t>(width_),
        .vres           = static_cast<uint32_t>(height_),
        .monochrome     = false,
        .rotation =
            {
                .swap_xy  = swap_xy,
                .mirror_x = mirror_x,
                .mirror_y = mirror_y,
            },
        .color_format = LV_COLOR_FORMAT_RGB565,
        .flags =
            {
                .buff_dma     = 1,
                .buff_spiram  = 0,
                .sw_rotate    = 0,
                .swap_bytes   = 1,
                .full_refresh = 0,
                .direct_mode  = 0,
            },
    };
    
    // 把底层的硬件配置添加给 LVGL
    lvgl_port_add_disp(&disp_cfg);

    if (display_ == nullptr) {
        ESP_LOGE(TAG, "Failed to add display");
        return;
    }
    
    if (offset_x != 0 || offset_y != 0) {
        lv_display_set_offset(display_, offset_x, offset_y);
    }
}
MyDisplay::MyDisplay() {}
MyDisplay::~MyDisplay() {} 
// 拦截 1：小智切换情绪
// 拦截 1：小智切换情绪
void MyDisplay::SetEmotion(const char* emotion) {
    if (!emotion) return;

    ESP_LOGI(TAG, "大模型下发情绪标签: %s", emotion);
    AppManager::GetInstance().notifyEvent({EventType::EVENT_EMOTION, emotion});
}

// 拦截 2：小智切换状态（听、想、说）
void MyDisplay::SetStatus(const char* status) {
    if (!status) return;
    ESP_LOGE(TAG, "SetStatus: %s", status);
    AppManager::GetInstance().notifyEvent({EventType::EVENT_STATUS, status});
}
void MyDisplay::SetTheme(Theme* theme) {
}
// 拦截 3：聊天消息
void MyDisplay::SetChatMessage(const char* role, const char* content) {
    // 这里可以直接把大模型的回复文字打在企鹅头顶的气泡里！
    // my_avatar->setBubbleText(content); (如果你在 SmileAvatar 里写了这个方法的话)
}

// 把其他小智需要的纯虚函数实现为空，防止它去操作真实屏幕导致 SPI 崩溃
void MyDisplay::UpdateStatusBar(bool force) {}  
// virtual void ShowNotification(const char* message) override {}
// void MyDisplay::ShowNotification(const char* notification, int duration_ms) {
    // 这里可以直接在屏幕上打印通知，或者用其他方式提醒用户
    // 例如：Serial.println(notification);  
// }

bool MyDisplay::Lock(int timeout_ms)
{
    return lvgl_port_lock(timeout_ms);
}

void MyDisplay::Unlock()
{
    lvgl_port_unlock();
}
// (注意：如果你编译报错说有未实现的虚函数，请对照 xiaozhi-esp32/main/display.h 里的 virtual 函数，在这里全部补上空实现 {})
