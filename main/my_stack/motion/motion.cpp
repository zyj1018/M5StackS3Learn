#include "motion.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "Motion";

namespace my_stack {
namespace motion {

Motion::Motion(SG90Servo* servo_x, SG90Servo* servo_y) 
    : servo_x_(servo_x), servo_y_(servo_y) {}

Motion::~Motion() {}

void Motion::init() {
    ESP_LOGI(TAG, "Motion initialized. Moving to zero position.");
    setPose(90, 90); // 开机归中，平视前方
}

void Motion::setPose(int x_angle, int y_angle) {
    if (servo_x_) servo_x_->smoothMove(x_angle, 15);
    if (servo_y_) servo_y_->smoothMove(y_angle, 15);
}

void Motion::nod() {
    ESP_LOGI(TAG, "Motion: Nodding");
    if (servo_y_) {
        // 假设 90 是平视，60 是低头
        servo_y_->smoothMove(60, 10); // 低头
        vTaskDelay(pdMS_TO_TICKS(100));
        servo_y_->smoothMove(90, 10); // 抬起
        vTaskDelay(pdMS_TO_TICKS(100));
        servo_y_->smoothMove(60, 10); // 低头
        vTaskDelay(pdMS_TO_TICKS(100));
        servo_y_->smoothMove(90, 10); // 抬起
    }
}

void Motion::shakeHead() {
    ESP_LOGI(TAG, "Motion: Shaking head");
    if (servo_x_) {
        // 假设 90 是正视，60 是左，120 是右
        servo_x_->smoothMove(60, 10);
        vTaskDelay(pdMS_TO_TICKS(100));
        servo_x_->smoothMove(120, 10);
        vTaskDelay(pdMS_TO_TICKS(100));
        servo_x_->smoothMove(60, 10);
        vTaskDelay(pdMS_TO_TICKS(100));
        servo_x_->smoothMove(90, 10); // 归位
    }
}

void Motion::lookAround() {
    ESP_LOGI(TAG, "Motion: Looking around");
    if (servo_x_ && servo_y_) {
        // 左上看
        servo_x_->smoothMove(45, 15);
        servo_y_->smoothMove(120, 15);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 右下看
        servo_x_->smoothMove(135, 15);
        servo_y_->smoothMove(60, 15);
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // 归中
        setPose(90, 90);
    }
}

} // namespace motion
} // namespace my_stack
