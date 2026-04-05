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
#include "ui/SmileAvatar.h"  // 我们的企鹅


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

    // 将小智的字符串情绪翻译为现有的枚举（已涵盖大模型下发的所有新情绪词）
    
    // 1. 平和/思考类 -> 对应 NEUTRAL (中性)
    if (emo == "neutral" || emo == "thinking" || emo == "relaxed") {
        target_emo = AvatarEmotion::NEUTRAL; 
    }
    // 2. 开心/自信类 -> 对应 HAPPY (开心)
    else if (emo == "happy" || emo == "glad" || emo == "confident") {
        target_emo = AvatarEmotion::HAPPY; 
    }
    // 3. 大笑/激动类 -> 对应 CHEER (欢呼/兴奋)
    else if (emo == "laughing" || emo == "cheer" || emo == "excited") {
        target_emo = AvatarEmotion::CHEER; 
    }
    // 4. 愤怒类 -> 对应 ANGRY (生气)
    else if (emo == "angry" || emo == "mad") {
        target_emo = AvatarEmotion::ANGRY; 
    }
    // 5. 悲伤/尴尬类 -> 对应 SAD (难过)
    else if (emo == "sad" || emo == "sorrow" || emo == "crying" || emo == "embarrassed") {
        target_emo = AvatarEmotion::SAD; 
    }
    // 6. 恐惧类 -> 对应 FEAR (害怕)
    else if (emo == "fear" || emo == "scared") {
        target_emo = AvatarEmotion::FEAR; 
    }
    // 7. 惊讶/懵逼类 -> 对应 SURPRISE (惊讶)
    else if (emo == "surprised" || emo == "shocked" || emo == "confused") {
        target_emo = AvatarEmotion::SURPRISE; 
    }
    // 8. 充满爱意/撒娇类 -> 对应 CUTE (可爱)
    else if (emo == "cute" || emo == "kawaii" || emo == "loving" || emo == "kissy" || emo == "delicious") {
        target_emo = AvatarEmotion::CUTE; 
    }
    // 9. 搞怪/调皮类 -> 对应 NAUGHTY (调皮)
    else if (emo == "naughty" || emo == "playful" || emo == "funny" || emo == "silly" || emo == "winking") {
        target_emo = AvatarEmotion::NAUGHTY; 
    }
    // 10. 恳求类 -> 对应 PLEASE (祈求)
    else if (emo == "please" || emo == "pleading") {
        target_emo = AvatarEmotion::PLEASE; 
    }
    // 11. 耍酷/嘲讽类 -> 对应 MOCK (嘲讽/耍帅)
    else if (emo == "mock" || emo == "sarcastic" || emo == "cool") {
        target_emo = AvatarEmotion::MOCK; 
    }
    // 12. 嫌弃类 -> 对应 DISDAIN (鄙夷)
    else if (emo == "disdain" || emo == "disgust") {
        target_emo = AvatarEmotion::DISDAIN; 
    }
    // 13. 困倦/发呆/无语类 -> 对应 ROLL_EYES (翻白眼/半闭眼)
    else if (emo == "microchip_ai" || emo == "boring" || emo == "sleepy") {
        target_emo = AvatarEmotion::ROLL_EYES; 
    }
    // 14. 终极兜底（如果未来大模型又发明了新词汇，默认回落到中性表情，防止崩溃）
    else {
        target_emo = AvatarEmotion::NEUTRAL;
    }

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
