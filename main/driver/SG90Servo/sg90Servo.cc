#include "sg90Servo.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "SG90Servo";

// SG90 舵机 PWM 参数
#define SERVO_MIN_PULSEWIDTH_US 500  // 最小脉宽 0.5ms (对应 0 度)
#define SERVO_MAX_PULSEWIDTH_US 2500 // 最大脉宽 2.5ms (对应 180 度)
#define SERVO_MAX_DEGREE        180  // 最大角度 180 度
#define SERVO_PWM_FREQ_HZ       50   // PWM 频率 50Hz (周期 20ms)

// 使用 13 位分辨率，占空比范围 0 ~ 8191 (2^13 - 1)
#define LEDC_DUTY_RESOLUTION    LEDC_TIMER_13_BIT
#define LEDC_MAX_DUTY           8191

SG90Servo::SG90Servo(gpio_num_t pin, ledc_channel_t channel, ledc_timer_t timer)
    : pin_(pin), channel_(channel), timer_(timer), current_angle_(90) {

    // 1. 配置 LEDC 定时器
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = LEDC_DUTY_RESOLUTION,
        .timer_num        = timer_,
        .freq_hz          = SERVO_PWM_FREQ_HZ, 
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 2. 配置 LEDC 通道
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pin_,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = channel_,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = timer_,
        .duty           = 0, // 初始占空比为 0
        .hpoint         = 0,
        .flags          = { .output_invert = 0 }
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI(TAG, "SG90 Servo initialized on GPIO %d, Channel %d, Timer %d", pin_, channel_, timer_);

    // 适配 StackChan Servo 基类的角度范围 (例如 0~180度)
    set_angle_limit(uitk::Vector2i(0, SERVO_MAX_DEGREE));

    // 初始化后默认归中 90 度
    setAngle(current_angle_);
}

SG90Servo::~SG90Servo() {
    ledc_stop(LEDC_LOW_SPEED_MODE, channel_, 0);
    ESP_LOGI(TAG, "SG90 Servo stopped on GPIO %d", pin_);
}

void SG90Servo::setAngle(int angle) {
    if (angle < 0) angle = 0;
    if (angle > SERVO_MAX_DEGREE) angle = SERVO_MAX_DEGREE;

    // 计算当前角度对应的脉宽 (us)
    uint32_t pulse_width = SERVO_MIN_PULSEWIDTH_US + 
        (angle * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / SERVO_MAX_DEGREE);

    // 将脉宽转换为 13位分辨率下的占空比
    // 占空比 = (脉宽 / 周期) * 最大占空比值
    // 周期 = 1,000,000 us / 50 Hz = 20,000 us
    uint32_t duty = (pulse_width * LEDC_MAX_DUTY) / 20000;

    ESP_LOGD(TAG, "Setting angle to %d, pulse width %lu us, duty %lu", angle, pulse_width, duty);

    // 更新 PWM 占空比
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, channel_, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, channel_));

    current_angle_ = angle;
}

// 供 StackChan Motion 调用的底层纯虚函数实现
void SG90Servo::set_angle_impl(int angle) {
    setAngle(angle);
}

void SG90Servo::smoothMove(int target_angle, int step_delay_ms) {
    if (target_angle < 0) target_angle = 0;
    if (target_angle > SERVO_MAX_DEGREE) target_angle = SERVO_MAX_DEGREE;

    int step = (target_angle > current_angle_) ? 1 : -1;
    
    while (current_angle_ != target_angle) {
        setAngle(current_angle_ + step);
        vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
    }
}
