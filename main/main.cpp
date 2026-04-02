#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "ui/SmileAvatar.hpp" 

// CoreS3 BSP (板级支持包) 或你自己的屏幕/触摸驱动头文件
#include "bsp/esp-bsp.h"
#include "bsp/display.h" 
#include "bsp/touch.h"
// 引入我们刚写的网络模块
#include "network/WifiManager.hpp"

// 替换为这一行
#include "driver/i2c_master.h"
#include "esp_heap_caps.h" // 引入高级内存分配器

static SmileAvatar* my_avatar = nullptr;

// 声明全局的扬声器和麦克风设备句柄
esp_codec_dev_handle_t spk_codec_dev = NULL;
esp_codec_dev_handle_t mic_codec_dev = NULL;

#define AUDIO_BUFFER_SIZE 1024

// 触摸点击事件回调 (由 LVGL 内部线程调用，不需要加锁)
static void avatar_event_cb(lv_event_t * e) {
    // 使用静态整数记录当前的表情索引
    static int current_idx = 0;
    if (my_avatar == nullptr) return;

    // 每次点击，索引 + 1，如果超过 12 (即 DISDAIN)，就回到 0 (NEUTRAL)
    current_idx++;
    if (current_idx > 12) {
        current_idx = 0; 
    }
    
    // 将 int 强制转换为 AvatarEmotion 枚举并设置
    my_avatar->setEmotion(static_cast<AvatarEmotion>(current_idx));
}
// 新增：一个后台独立任务，专门用来监控 Wi-Fi 状态并控制表情
static void wifi_monitor_task(void *arg) {
    // 刚开机，告诉 UI 正在努力连网，露出“楚楚可怜/期待”的表情
    if (my_avatar) {
        bsp_display_lock(0);
        my_avatar->setEmotion(AvatarEmotion::PLEASE);
        bsp_display_unlock();
    }

    // 死等 FreeRTOS 事件组的标志位（非阻塞，不会卡死 UI 动画）
    EventBits_t bits = xEventGroupWaitBits(Network::s_wifi_event_group,
            Network::WIFI_CONNECTED_BIT | Network::WIFI_FAIL_BIT,
            pdFALSE, pdFALSE, portMAX_DELAY);

    bsp_display_lock(0);
    // 根据连网结果切换表情
    if (bits & Network::WIFI_CONNECTED_BIT) {
        my_avatar->setEmotion(AvatarEmotion::HAPPY); // 连网成功，开心月牙眼！
    } else if (bits & Network::WIFI_FAIL_BIT) {
        my_avatar->setEmotion(AvatarEmotion::SAD);   // 连网失败，沮丧八字眼
    }
    bsp_display_unlock();

    // 任务完成，自我销毁
    vTaskDelete(NULL);
}
static const char *TAG = "AUDIO_TEST"; // 定义 Log 标签

#define RECORD_TIME_SEC 2
#define RECORD_BUFFER_SIZE (16000 * 2 * 1 * RECORD_TIME_SEC)
void audio_loopback_task(void *pvParameters)
{
    ESP_LOGI(TAG, "==> 进入【对讲机模式】音频 Task");

    // ==========================================
    // 🌟 核心魔法：使用 heap_caps_malloc 强制从 8MB 的外部 PSRAM 借用内存！
    // 别说 64KB，在这里借 1MB 都轻轻松松！
    // ==========================================
    uint8_t *tape_buffer = (uint8_t *)heap_caps_malloc(RECORD_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
    if (tape_buffer == NULL) {
        // 如果连外挂内存都失败了（极小概率），那就降级，只录 0.5 秒
        ESP_LOGE(TAG, "PSRAM 内存分配失败！尝试降级申请内部内存...");
        tape_buffer = (uint8_t *)malloc(16000 * 2 * 1 * 0.5); // 16KB
        if (tape_buffer == NULL) {
             ESP_LOGE(TAG, "彻底没内存了！");
             vTaskDelete(NULL);
        }
    }

    while (1) {
        // ... 下面的录音和播放逻辑保持完全不变 ...
        ESP_LOGW(TAG, "🎤 [录音中...] 请对着麦克风说话 (2秒)...");
        esp_codec_dev_set_out_vol(spk_codec_dev, 0); 
        
        esp_err_t read_res = esp_codec_dev_read(mic_codec_dev, (void*)tape_buffer, RECORD_BUFFER_SIZE);
        
        if (read_res == ESP_OK) {
            ESP_LOGI(TAG, "🔊 [播放中...] 正在回放刚才的声音...");
            esp_codec_dev_set_out_vol(spk_codec_dev, 80); 
            esp_codec_dev_write(spk_codec_dev, (void*)tape_buffer, RECORD_BUFFER_SIZE);
            vTaskDelay(pdMS_TO_TICKS(500)); 
        } else {
            ESP_LOGE(TAG, "读取麦克风失败!");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

extern "C" void app_main(void) {
    // 1. 一键初始化显示屏、触摸屏并启动 LVGL 后台任务！
    bsp_display_start();
    bsp_display_backlight_on();

    // ==========================================
    // 2. 初始化硬件 (⚠️ 必须放在 UI 锁的外面！)
    // ==========================================
    bsp_audio_init(NULL);
    spk_codec_dev = bsp_audio_codec_speaker_init();
    mic_codec_dev = bsp_audio_codec_microphone_init();

    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = 16,
        .channel = 1,
        .channel_mask = 0,
        .sample_rate = 16000,
        .mclk_multiple = 0,
    };
    
    // 打开设备
    esp_codec_dev_open(spk_codec_dev, &fs);
    esp_codec_dev_open(mic_codec_dev, &fs);
    
    // 🌟 核心魔法：解除音量封印，拉到最大！
    if (spk_codec_dev) {
        esp_codec_dev_set_out_vol(spk_codec_dev, 100); 
    }

    // ==========================================
    // 3. 初始化 UI (✅ 只有 LVGL 的操作才需要加锁)
    // ==========================================
    bsp_display_lock(0);
    my_avatar = new SmileAvatar(lv_scr_act());
    lv_obj_add_event_cb(my_avatar->getView(), avatar_event_cb, LV_EVENT_CLICKED, NULL);
    bsp_display_unlock();

    // ==========================================
    // 4. 启动各种后台任务
    // ==========================================
    // 触发 Wi-Fi 异步连接
    Network::init_wifi_sta();
    xTaskCreate(wifi_monitor_task, "wifi_monitor", 4096, NULL, 5, NULL);

    // 启动全双工回环测试任务
    if (spk_codec_dev && mic_codec_dev) {
        xTaskCreate(audio_loopback_task, "audio_loop", 4096, NULL, 5, NULL);
    }
}