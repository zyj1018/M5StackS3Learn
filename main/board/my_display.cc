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
#include "ui/SmileAvatar.hpp"  // 我们的企鹅


#define TAG "MyDisplay"

extern SmileAvatar* my_avatar;

MyDisplay::MyDisplay() {}
MyDisplay::~MyDisplay() {} 
// 拦截 1：小智切换情绪
// 拦截 1：小智切换情绪
void MyDisplay::SetEmotion(const char* emotion) {
    if (!my_avatar || !emotion) return;
    
    std::string emo(emotion);
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;
    
    // 🌟 加上这句日志！以后你可以在串口里看到大模型到底发来了什么词！
    ESP_LOGI(TAG, "大模型下发情绪标签: %s", emo.c_str());

    // 将小智的字符串情绪翻译为企鹅的枚举（涵盖所有常见的大模型情绪词）
    if (emo == "neutral") target_emo = AvatarEmotion::NEUTRAL; // 中性/无情绪
    else if (emo == "happy" || emo == "glad") target_emo = AvatarEmotion::HAPPY; // 开心/喜悦
    else if (emo == "angry" || emo == "mad") target_emo = AvatarEmotion::ANGRY; // 生气/愤怒
    else if (emo == "sad" || emo == "sorrow") target_emo = AvatarEmotion::SAD; // 难过/悲伤
    else if (emo == "fear" || emo == "scared") target_emo = AvatarEmotion::FEAR; // 害怕/恐惧
    else if (emo == "surprised" || emo == "shocked") target_emo = AvatarEmotion::SURPRISE; // 惊讶/震惊
    else if (emo == "cute" || emo == "kawaii") target_emo = AvatarEmotion::CUTE; // 可爱/卖萌
    else if (emo == "naughty" || emo == "playful") target_emo = AvatarEmotion::NAUGHTY; // 调皮/顽皮
    else if (emo == "please" || emo == "pleading") target_emo = AvatarEmotion::PLEASE; // 祈求/恳求
    else if (emo == "cheer" || emo == "excited") target_emo = AvatarEmotion::CHEER; // 兴奋/欢呼
    else if (emo == "mock" || emo == "sarcastic") target_emo = AvatarEmotion::MOCK; // 嘲讽/阴阳怪气
    else if (emo == "disdain" || emo == "disgust") target_emo = AvatarEmotion::DISDAIN; // 鄙夷/嫌弃
    // 特殊状态：例如 OTA、连接中、或者模型发呆
    else if (emo == "microchip_ai" || emo == "boring") target_emo = AvatarEmotion::ROLL_EYES; // 无语/发呆/翻白眼

    // 🌟 终极护甲：加上 LVGL 互斥锁，防止多线程踩踏内存！
    if (lvgl_port_lock(0)) {
        my_avatar->setEmotion(target_emo);
        lvgl_port_unlock();
    }
}

// 拦截 2：小智切换状态（听、想、说）
void MyDisplay::SetStatus(const char* status) {
    ESP_LOGE(TAG, "SetStatus: %s", status);
    if (!my_avatar) return;
    
    std::string stat(status);
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;
    bool need_change = false;

    if (stat == "聆听中" || stat == "Listening") { 
        target_emo = AvatarEmotion::PLEASE;
        need_change = true;
    } else if (stat == "连接中" || stat == "Connecting" || stat == "检查新版本...") { // 顺手把OTA状态也加上
        target_emo = AvatarEmotion::ROLL_EYES;
        need_change = true;
    }

    // 🌟 终极护甲：加上 LVGL 互斥锁
    if (need_change) {
        if (lvgl_port_lock(0)) {
            my_avatar->setEmotion(target_emo);
            lvgl_port_unlock();
        }
    }
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
