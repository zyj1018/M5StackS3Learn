#include "penguin_app.h"
#include <esp_log.h>
#include "hal/hal.h"
#include "esp_lvgl_port.h"

static const char* TAG = "PenguinApp";

PenguinApp::PenguinApp() {}

PenguinApp::~PenguinApp() {}

void PenguinApp::onCreate() {
    ESP_LOGI(TAG, "PenguinApp created");
    if (lvgl_port_lock(0)) {
        my_avatar_ = new SmileAvatar(lv_scr_act());
        my_avatar_->setEmotion(AvatarEmotion::PLEASE);
        lvgl_port_unlock();
    }
}

void PenguinApp::onDestroy() {
    if (my_avatar_) {
        if (lvgl_port_lock(0)) {
            delete my_avatar_;
            my_avatar_ = nullptr;
            lvgl_port_unlock();
        }
    }
}

void PenguinApp::onEvent(const AppEvent& event) {
    if (event.type == EventType::EVENT_EMOTION) {
        handleEmotion(event.data);
    } else if (event.type == EventType::EVENT_STATUS) {
        handleStatus(event.data);
    }
}

void PenguinApp::handleEmotion(const std::string& emo) {
    if (!my_avatar_) return;
    
    ESP_LOGI(TAG, "LLM Emotion: %s", emo.c_str());
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;
    
    if (emo == "neutral" || emo == "thinking" || emo == "relaxed") {
        target_emo = AvatarEmotion::NEUTRAL;
        if (HAL::global_motion) HAL::global_motion->goHome();
    } else if (emo == "happy" || emo == "glad" || emo == "confident") {
        target_emo = AvatarEmotion::HAPPY;
        if (HAL::global_motion) HAL::global_motion->lookAtNormalized(0, 0.5);
    } else if (emo == "laughing" || emo == "cheer" || emo == "excited") {
        target_emo = AvatarEmotion::CHEER;
        if (HAL::global_motion) HAL::global_motion->lookAtNormalized(0, 0.8);
    } else if (emo == "angry" || emo == "mad") {
        target_emo = AvatarEmotion::ANGRY;
    } else if (emo == "sad" || emo == "sorrow" || emo == "crying" || emo == "embarrassed") {
        target_emo = AvatarEmotion::SAD;
        if (HAL::global_motion) HAL::global_motion->lookAtNormalized(0, -0.8);
    } else if (emo == "fear" || emo == "scared") {
        target_emo = AvatarEmotion::FEAR;
    } else if (emo == "surprised" || emo == "shocked" || emo == "confused") {
        target_emo = AvatarEmotion::SURPRISE;
    } else if (emo == "cute" || emo == "kawaii" || emo == "loving" || emo == "kissy" || emo == "delicious") {
        target_emo = AvatarEmotion::CUTE;
    } else if (emo == "naughty" || emo == "playful" || emo == "funny" || emo == "silly" || emo == "winking") {
        target_emo = AvatarEmotion::NAUGHTY;
    } else if (emo == "please" || emo == "pleading") {
        target_emo = AvatarEmotion::PLEASE;
    } else if (emo == "mock" || emo == "sarcastic" || emo == "cool") {
        target_emo = AvatarEmotion::MOCK;
    } else if (emo == "disdain" || emo == "disgust") {
        target_emo = AvatarEmotion::DISDAIN;
    } else if (emo == "microchip_ai" || emo == "boring" || emo == "sleepy") {
        target_emo = AvatarEmotion::ROLL_EYES;
    } else {
        target_emo = AvatarEmotion::NEUTRAL;
    }
    
    if (lvgl_port_lock(0)) {
        my_avatar_->setEmotion(target_emo);
        lvgl_port_unlock();
    }
}

void PenguinApp::handleStatus(const std::string& stat) {
    if (!my_avatar_) return;
    
    ESP_LOGI(TAG, "LLM Status: %s", stat.c_str());
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;
    bool need_change = false;
    
    if (stat == "聆听中" || stat == "Listening") {
        target_emo = AvatarEmotion::PLEASE;
        need_change = true;
    } else if (stat == "连接中" || stat == "Connecting" || stat == "检查新版本...") {
        target_emo = AvatarEmotion::ROLL_EYES;
        need_change = true;
    }
    
    if (need_change) {
        if (lvgl_port_lock(0)) {
            my_avatar_->setEmotion(target_emo);
            lvgl_port_unlock();
        }
    }
}
