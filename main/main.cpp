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
#include "cores3_audio_codec.h" // 引入你刚才展示的头文件
#include "driver/i2c.h"

static SmileAvatar* my_avatar = nullptr;


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

// 假设每次处理 1024 个采样点
#define SAMPLES_PER_CHUNK 1024

// 定义全局指针，方便 Task 访问
CoreS3AudioCodec* audio_codec = nullptr;

void audio_loopback_task(void *pvParameters)
{
    // 因为是 16bit 音频，所以 buffer 使用 int16_t 类型
    int16_t *audio_buffer = (int16_t *)malloc(SAMPLES_PER_CHUNK * sizeof(int16_t));
    
    while (1) {
        // 1. 从麦克风读取数据 (调用 StackChain 封装的 Read)
        int samples_read = audio_codec->Read(audio_buffer, SAMPLES_PER_CHUNK);
        
        if (samples_read > 0) {
            // 2. 将数据直接写入扬声器 (调用 StackChain 封装的 Write)
            audio_codec->Write(audio_buffer, samples_read);
        }
        
        // 防止看门狗超时
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

extern "C" void app_main(void) {
// 1. 一键初始化显示屏、触摸屏并启动 LVGL 后台任务！
    bsp_display_start();

// 2. 打开屏幕背光 (非常重要，否则黑屏)
    bsp_display_backlight_on();
// 3. 获取 LVGL 多线程锁 (0 表示无限等待)
    // 在主任务中操作 LVGL 对象，必须被 Lock/Unlock 包裹
    bsp_display_lock(0);
// 1. 初始化全局 I2C 主机 (CoreS3 内部总线，通常是 GPIO11/12)
    // 注意：这里需要根据你的实际 I2C 初始化代码来配置
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_12,
        .scl_io_num = GPIO_NUM_11,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {.clk_speed = 400000},
    };
    i2c_param_config(I2C_NUM_0, &i2c_conf);
    i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0);

    // 2. 实例化这个强悍的音频编解码器类
    // 传入的引脚必须和 CoreS3 实际引脚对应 (参考我上个回复给你的引脚表)
    audio_codec = new CoreS3AudioCodec(
        (void*)I2C_NUM_0, 
        16000, 16000,           // 采样率：16kHz 对语音识别最好
        GPIO_NUM_0,             // MCLK
        GPIO_NUM_34,            // BCLK
        GPIO_NUM_33,            // WS
        GPIO_NUM_13,            // DOUT (Speaker)
        GPIO_NUM_14,            // DIN (Mic)
        0x36,                   // AW88298 I2C 地址
        0x40,                   // ES7210 I2C 地址
        false                   // 不开启硬件回声消除参考
    );

    // 3. 启用麦克风和扬声器
    audio_codec->EnableInput(true);
    audio_codec->EnableOutput(true);
    audio_codec->SetOutputVolume(70); // 设置一个适中的音量
    // 3. 实例化 UI
    // CoreS3 屏幕正好是 320x240，完美匹配我们的设计
    my_avatar = new SmileAvatar(lv_scr_act());

    // 4. 绑定触摸事件
    lv_obj_add_event_cb(my_avatar->getView(), avatar_event_cb, LV_EVENT_CLICKED, NULL);

// 6. 释放锁，允许后台的 LVGL 任务开始渲染画面
    bsp_display_unlock();

    // 2. 触发 Wi-Fi 异步连接 (瞬间返回，绝不卡主线程)
    Network::init_wifi_sta();

    // 3. 开启一个后台任务，等 Wi-Fi 结果来切换表情
    xTaskCreate(wifi_monitor_task, "wifi_monitor", 4096, NULL, 5, NULL);

    xTaskCreate(audio_loopback_task, "audio_loop", 4096, NULL, 5, NULL);

}