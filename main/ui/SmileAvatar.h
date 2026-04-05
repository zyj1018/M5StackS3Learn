#pragma once

#include "lvgl.h"
#include "smooth_lvgl.hpp" 
#include <memory>

using namespace smooth_ui_toolkit;           
using namespace smooth_ui_toolkit::lvgl_cpp; 

enum class AvatarEmotion {
    NEUTRAL, HAPPY, ANGRY, SAD, FEAR, SURPRISE, 
    CUTE, NAUGHTY, PLEASE, CHEER, MOCK, ROLL_EYES, DISDAIN
};

class SmileAvatar {
private:
    std::unique_ptr<Container> _pannel;
    
    // 左眼组件组
    std::unique_ptr<Container> _l_eye_cont;    // 透明外框
    std::unique_ptr<Container> _l_eye_base;    // 白眼球 
    std::unique_ptr<Container> _l_crescent_mask; // 黑圆内切遮罩 (用于抠月牙)
    std::unique_ptr<Container> _l_eyelid;      // 黑方块遮罩 (用于愤怒/闭眼)
    
    // 右眼组件组
    std::unique_ptr<Container> _r_eye_cont;
    std::unique_ptr<Container> _r_eye_base;
    std::unique_ptr<Container> _r_crescent_mask;
    std::unique_ptr<Container> _r_eyelid;
    
    std::unique_ptr<Container> _mouth;
    std::unique_ptr<Container> _bubble;
    lv_obj_t* _bubble_label;

    // --- 动画变量库 ---
    AnimateValue _l_eye_y = -16; AnimateValue _r_eye_y = -16;
    AnimateValue _l_eye_r = 16;  AnimateValue _r_eye_r = 16;
    AnimateValue _l_eye_angle = 0; AnimateValue _r_eye_angle = 0;

    // 控制月牙遮罩的偏移量
    AnimateValue _l_crescent_y = -32; AnimateValue _r_crescent_y = -32;
    AnimateValue _l_crescent_x = 0;   AnimateValue _r_crescent_x = 0;

    // 控制方块遮罩的偏移量
    AnimateValue _l_eyelid_y = -32;   AnimateValue _r_eyelid_y = -32; 

    AnimateValue _mouth_x = 0; AnimateValue _mouth_y = 26;
    AnimateValue _mouth_w = 60; AnimateValue _mouth_h = 30;
    AnimateValue _mouth_angle = 0; 

    lv_timer_t* _anim_timer;

    static void _timer_cb(lv_timer_t* timer) {
        SmileAvatar* avatar = static_cast<SmileAvatar*>(lv_timer_get_user_data(timer));
        if (avatar) avatar->_update_ui();
    }

    void _update_ui() {
        _l_eye_cont->setY(_l_eye_y); _r_eye_cont->setY(_r_eye_y);
        
        lv_obj_set_style_transform_angle(_l_eye_cont->get(), _l_eye_angle, 0);
        lv_obj_set_style_transform_angle(_r_eye_cont->get(), _r_eye_angle, 0);
        lv_obj_set_style_transform_angle(_mouth->get(), _mouth_angle, 0);
        
        _l_eye_base->setRadius(_l_eye_r);
        _r_eye_base->setRadius(_r_eye_r);

        _l_crescent_mask->setX(_l_crescent_x); _l_crescent_mask->setY(_l_crescent_y);
        _r_crescent_mask->setX(_r_crescent_x); _r_crescent_mask->setY(_r_crescent_y);

        _l_eyelid->setY(_l_eyelid_y); _r_eyelid->setY(_r_eyelid_y);
        
        _mouth->setX(_mouth_x); _mouth->setY(_mouth_y);
        _mouth->setSize(_mouth_w, _mouth_h);
    }

    const char* _get_emotion_name(AvatarEmotion emotion) {
        switch (emotion) {
            case AvatarEmotion::NEUTRAL:   return "NEUTRAL"; case AvatarEmotion::HAPPY:     return "HAPPY";
            case AvatarEmotion::ANGRY:     return "ANGRY";   case AvatarEmotion::SAD:       return "SAD";
            case AvatarEmotion::FEAR:      return "FEAR";    case AvatarEmotion::SURPRISE:  return "SURPRISE";
            case AvatarEmotion::CUTE:      return "CUTE";    case AvatarEmotion::NAUGHTY:   return "NAUGHTY";
            case AvatarEmotion::PLEASE:    return "PLEASE";  case AvatarEmotion::CHEER:     return "CHEER";
            case AvatarEmotion::MOCK:      return "MOCK";    case AvatarEmotion::ROLL_EYES: return "ROLL EYES";
            case AvatarEmotion::DISDAIN:   return "DISDAIN"; default: return "UNKNOWN";
        }
    }

public:
    SmileAvatar(lv_obj_t* parent);

    ~SmileAvatar();

    lv_obj_t* getView();

    void setEmotion(AvatarEmotion emotion);
};