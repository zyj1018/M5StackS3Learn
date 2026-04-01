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
    SmileAvatar(lv_obj_t* parent) {
        lv_color_t primaryColor   = lv_color_white(); // 发光五官
        lv_color_t secondaryColor = lv_color_black(); // 脸皮与遮罩

        // --- 辅助魔法：一键干掉 LVGL 的默认边框和滚动条 ---
        auto clean_style = [](std::unique_ptr<Container>& cont) {
            cont->setBorderWidth(0);
            cont->setPadding(0, 0, 0, 0);
            cont->removeFlag(LV_OBJ_FLAG_SCROLLABLE);
        };

        _pannel = std::make_unique<Container>(parent);
        _pannel->align(LV_ALIGN_CENTER, 0, 0);
        _pannel->setSize(320, 240); 
        _pannel->setRadius(0); 
        _pannel->setBgColor(secondaryColor); 
        clean_style(_pannel);

        // --- 左眼 ---
        _l_eye_cont = std::make_unique<Container>(_pannel->get());
        _l_eye_cont->align(LV_ALIGN_CENTER, -70, -16);
        _l_eye_cont->setSize(32, 32); 
        _l_eye_cont->setBgOpa(0); 
        clean_style(_l_eye_cont);

        _l_eye_base = std::make_unique<Container>(_l_eye_cont->get());
        _l_eye_base->align(LV_ALIGN_CENTER, 0, 0); 
        _l_eye_base->setSize(32, 32); 
        _l_eye_base->setRadius(16); 
        _l_eye_base->setBgColor(primaryColor); 
        clean_style(_l_eye_base);

        _l_crescent_mask = std::make_unique<Container>(_l_eye_base->get()); 
        _l_crescent_mask->align(LV_ALIGN_TOP_MID, 0, -32); 
        _l_crescent_mask->setSize(32, 32); 
        _l_crescent_mask->setRadius(16); 
        _l_crescent_mask->setBgColor(secondaryColor); 
        clean_style(_l_crescent_mask);

        _l_eyelid = std::make_unique<Container>(_l_eye_base->get()); 
        _l_eyelid->align(LV_ALIGN_TOP_MID, 0, -32); 
        _l_eyelid->setSize(32, 32); 
        _l_eyelid->setRadius(0); 
        _l_eyelid->setBgColor(secondaryColor); 
        clean_style(_l_eyelid);

        // --- 右眼 ---
        _r_eye_cont = std::make_unique<Container>(_pannel->get());
        _r_eye_cont->align(LV_ALIGN_CENTER, 70, -16);
        _r_eye_cont->setSize(32, 32);
        _r_eye_cont->setBgOpa(0);
        clean_style(_r_eye_cont);

        _r_eye_base = std::make_unique<Container>(_r_eye_cont->get());
        _r_eye_base->align(LV_ALIGN_CENTER, 0, 0);
        _r_eye_base->setSize(32, 32);
        _r_eye_base->setRadius(16);
        _r_eye_base->setBgColor(primaryColor);
        clean_style(_r_eye_base);

        _r_crescent_mask = std::make_unique<Container>(_r_eye_base->get());
        _r_crescent_mask->align(LV_ALIGN_TOP_MID, 0, -32);
        _r_crescent_mask->setSize(32, 32);
        _r_crescent_mask->setRadius(16);
        _r_crescent_mask->setBgColor(secondaryColor);
        clean_style(_r_crescent_mask);

        _r_eyelid = std::make_unique<Container>(_r_eye_base->get());
        _r_eyelid->align(LV_ALIGN_TOP_MID, 0, -32);
        _r_eyelid->setSize(32, 32);
        _r_eyelid->setRadius(0);
        _r_eyelid->setBgColor(secondaryColor);
        clean_style(_r_eyelid);

        // --- 嘴巴 ---
        _mouth = std::make_unique<Container>(_pannel->get());
        _mouth->align(LV_ALIGN_CENTER, 0, 26); 
        _mouth->setSize(60, 30); 
        _mouth->setRadius(15);   
        _mouth->setBgColor(primaryColor);
        clean_style(_mouth);

        // --- 状态气泡 ---
        _bubble = std::make_unique<Container>(_pannel->get());
        _bubble->align(LV_ALIGN_CENTER, 90, 45); 
        _bubble->setSize(100, 26);  
        _bubble->setRadius(13);     
        _bubble->setBgColor(secondaryColor); 
        _bubble->setBorderWidth(2); // 气泡故意留2个像素的边框
        _bubble->setBorderColor(primaryColor); // 边框设为白色
        _bubble->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

        _bubble_label = lv_label_create(_bubble->get());
        lv_obj_align(_bubble_label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(_bubble_label, primaryColor, 0); 
        lv_label_set_text(_bubble_label, "NEUTRAL"); 

        _anim_timer = lv_timer_create(_timer_cb, 16, this);
    }

    ~SmileAvatar() {
        if (_anim_timer) lv_timer_del(_anim_timer);
    }

    lv_obj_t* getView() const { return _pannel->get(); }

    void setEmotion(AvatarEmotion emotion) {
        lv_label_set_text(_bubble_label, _get_emotion_name(emotion));

        // 默认基础参数 (正圆状态)
        _l_eye_y = -16; _r_eye_y = -16;
        _l_eye_r = 16;  _r_eye_r = 16;
        _l_eye_angle = 0; _r_eye_angle = 0;
        
        _l_crescent_x = 0; _r_crescent_x = 0;
        _l_crescent_y = -32; _r_crescent_y = -32; 

        _l_eyelid_y = -32; _r_eyelid_y = -32; 
        _mouth_x = 0; _mouth_y = 26; _mouth_angle = 0;
        _mouth_w = 60; _mouth_h = 30; 

        // --- 表情魔法参数 (暗黑模式专属完美调校版) ---
        switch (emotion) {
            case AvatarEmotion::NEUTRAL: 
                break;
                
            case AvatarEmotion::HAPPY: 
                // 笑眼：Y为正数，黑色遮罩下沉盖住底部，抠出完美的上弯月牙 ^
                _l_crescent_y = 14; _r_crescent_y = 14; 
                _l_eye_angle = 150; _r_eye_angle = -150;
                _mouth_w = 70; _mouth_h = 40; 
                break;

            case AvatarEmotion::ANGRY: 
                _l_eye_angle = 450; _r_eye_angle = -450; 
                _l_eyelid_y = -16; _r_eyelid_y = -16; 
                _mouth_w = 40; _mouth_h = 10; _mouth_y = 35;
                break;

            case AvatarEmotion::SAD: 
                _l_eye_angle = -300; _r_eye_angle = 300; 
                _l_eyelid_y = -14; _r_eyelid_y = -14;
                _mouth_w = 60; _mouth_h = 5; _mouth_y = 40;
                break;

            case AvatarEmotion::FEAR: 
                _l_eye_r = 8; _r_eye_r = 8;
                _l_eye_y = -20; _r_eye_y = -20;
                _mouth_w = 20; _mouth_h = 30; _mouth_y = 35;
                break;

            case AvatarEmotion::SURPRISE: 
                _l_eye_y = -25; _r_eye_y = -25;
                _mouth_w = 30; _mouth_h = 35; _mouth_y = 30;
                break;

            case AvatarEmotion::CUTE: 
                _l_crescent_y = -16; _r_crescent_y = -16; 
                _l_eye_angle = -100; _r_eye_angle = 100;
                _mouth_w = 30; _mouth_h = 15; _mouth_y = 20; 
                break;

            case AvatarEmotion::NAUGHTY: 
                _l_eyelid_y = -4; 
                _r_crescent_y = 14; 
                _r_eye_angle = -150;
                _mouth_w = 50; _mouth_h = 25; _mouth_angle = 150; _mouth_x = 10;      
                break;
                
            case AvatarEmotion::PLEASE: 
                _l_crescent_y = -18; _r_crescent_y = -18;
                _l_eye_y = -10; _r_eye_y = -10;
                _mouth_w = 50; _mouth_h = 25; _mouth_y = 25;
                break;

            case AvatarEmotion::CHEER: 
                _l_crescent_y = 12; _r_crescent_y = 12;
                _l_eye_angle = 200; _r_eye_angle = -200;
                _mouth_w = 70; _mouth_h = 30; _mouth_y = 20;
                break;

            case AvatarEmotion::MOCK: 
                _l_crescent_y = 20; _r_crescent_y = 20; 
                _mouth_w = 80; _mouth_h = 40; _mouth_angle = -100; 
                break;

            case AvatarEmotion::ROLL_EYES: 
                _l_eye_y = -35; _r_eye_y = -35;
                _l_crescent_y = 16; _r_crescent_y = 16; 
                _mouth_w = 40; _mouth_h = 5; _mouth_y = 30;
                break;

            case AvatarEmotion::DISDAIN: 
                _l_eyelid_y = -10; 
                _r_crescent_y = 12; 
                _r_eye_angle = -100;
                _mouth_w = 35; _mouth_h = 8; 
                _mouth_angle = -180; 
                _mouth_x = 25; _mouth_y = 15; 
                break;
        }
    }
};