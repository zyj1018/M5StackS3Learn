#pragma once

#include "driver/SG90Servo/sg90Servo.h"
#include <memory>
#include <string>

namespace my_stack {
namespace motion {

/**
 * @brief 机器人运动控制器 (仿 StackChan Motion 架构)
 * 
 * 将底层两个独立的 SG90s 舵机封装为复合的机器动作，
 * 为上层 (如 MCP 工具) 提供“点头”、“摇头”、“环顾”等高级语意接口。
 */
class Motion {
public:
    Motion(SG90Servo* servo_x, SG90Servo* servo_y);
    ~Motion();

    /**
     * @brief 初始化运动系统，使其归中
     */
    void init();
    
    /**
     * @brief 设定绝对姿态角度
     * @param x_angle 左右水平角度 (0~180)
     * @param y_angle 上下垂直角度 (0~180)
     */
    void setPose(int x_angle, int y_angle);

    /**
     * @brief 动作：点头
     */
    void nod();

    /**
     * @brief 动作：摇头
     */
    void shakeHead();

    /**
     * @brief 动作：环顾四周
     */
    void lookAround();

private:
    SG90Servo* servo_x_;
    SG90Servo* servo_y_;
};

} // namespace motion
} // namespace my_stack
