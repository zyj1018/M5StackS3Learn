#pragma once

#include <driver/gpio.h>
#include <driver/ledc.h>

/**
 * @brief SG90s 舵机控制类
 * 
 * 基于 ESP-IDF 的 LEDC (LED Control) PWM 外设实现对 SG90s 舵机的控制。
 * SG90s 工作频率为 50Hz (周期 20ms)，脉宽 0.5ms ~ 2.5ms 对应 0° ~ 180°。
 */
class SG90Servo {
public:
    /**
     * @brief 构造并初始化舵机
     * 
     * @param pin 舵机 PWM 控制信号连接的 GPIO 引脚
     * @param channel LEDC 通道 (例如 LEDC_CHANNEL_0)
     * @param timer LEDC 定时器 (例如 LEDC_TIMER_0)
     */
    SG90Servo(gpio_num_t pin, ledc_channel_t channel = LEDC_CHANNEL_0, ledc_timer_t timer = LEDC_TIMER_0);
    
    ~SG90Servo();

    /**
     * @brief 设置舵机角度
     * 
     * @param angle 目标角度 (0 到 180)
     */
    void setAngle(int angle);

    /**
     * @brief 平滑转动到指定角度 (可选使用任务或直接阻塞)
     * 
     * @param target_angle 目标角度
     * @param step_delay_ms 每步延迟的毫秒数，控制速度
     */
    void smoothMove(int target_angle, int step_delay_ms = 15);

private:
    gpio_num_t pin_;
    ledc_channel_t channel_;
    ledc_timer_t timer_;
    int current_angle_;
};
