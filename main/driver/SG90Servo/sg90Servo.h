#pragma once

#include <driver/gpio.h>
#include <driver/ledc.h>
#include "my_stack/motion/servo.h"

/**
 * @brief SG90s 舵机控制类，适配 StackChan Motion 体系
 * 
 * 继承自 stackchan::motion::Servo，利用底层 LEDC 生成 PWM 信号控制。
 * 平滑动画和物理插值由基类 Servo::update() 处理，本类只需实现真实的物理角度输出 set_angle_impl()。
 */
class SG90Servo : public stackchan::motion::Servo {
public:
    /**
     * @brief 构造并初始化舵机
     */
    SG90Servo(gpio_num_t pin, ledc_channel_t channel = LEDC_CHANNEL_0, ledc_timer_t timer = LEDC_TIMER_0);
    
    virtual ~SG90Servo();

    /**
     * @brief 立即设置底层物理角度 (覆写基类纯虚函数)
     * 被基类 update() 中的弹簧动画引擎调用
     */
    void set_angle_impl(int angle) override;

    // 以下两个方法保留为了向下兼容，但推荐使用基类的 move() 等方法
    void setAngle(int angle);
    void smoothMove(int target_angle, int step_delay_ms = 15);

private:
    gpio_num_t pin_;
    ledc_channel_t channel_;
    ledc_timer_t timer_;
    int current_angle_;
};
