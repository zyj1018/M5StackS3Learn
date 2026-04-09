/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "motion.h"
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace uitk;
using namespace stackchan::motion;

void Motion::init()
{
    _yaw_servo->init();
    _pitch_servo->init();
}

void Motion::update()
{
    _yaw_servo->update();
    _pitch_servo->update();
}

Servo& Motion::yawServo()
{
    return *_yaw_servo;
}

Servo& Motion::pitchServo()
{
    return *_pitch_servo;
}

void Motion::moveYaw(int angle)
{
    _yaw_servo->move(angle);
}

void Motion::moveYawWithSpeed(int angle, int speed)
{
    _yaw_servo->moveWithSpeed(angle, speed);
}

void Motion::movePitch(int angle)
{
    _pitch_servo->move(angle);
}

void Motion::movePitchWithSpeed(int angle, int speed)
{
    _pitch_servo->moveWithSpeed(angle, speed);
}

void Motion::move(int yawAngle, int pitchAngle)
{
    _yaw_servo->move(yawAngle);
    _pitch_servo->move(pitchAngle);
}

void Motion::moveWithSpeed(int yawAngle, int pitchAngle, int speed)
{
    _yaw_servo->moveWithSpeed(yawAngle, speed);
    _pitch_servo->moveWithSpeed(pitchAngle, speed);
}

void Motion::goHome(int speed)
{
    _yaw_servo->moveWithSpeed(0, speed);
    _pitch_servo->moveWithSpeed(0, speed);
}

void Motion::stop()
{
    _yaw_servo->move(_yaw_servo->getCurrentAngle());
    _pitch_servo->move(_pitch_servo->getCurrentAngle());
}

void Motion::lookAtNormalized(float x, float y, int speed)
{
    int yaw_angle =
        uitk::map_range(x, -1.0f, 1.0f, (float)_yaw_servo->getAngleLimit().x, (float)_yaw_servo->getAngleLimit().y);
    int pitch_angle =
        uitk::map_range(y, -1.0f, 1.0f, (float)_pitch_servo->getAngleLimit().x, (float)_pitch_servo->getAngleLimit().y);
    moveWithSpeed(yaw_angle, pitch_angle, speed);
}

void Motion::lookAtPoint(float x, float y, float z, int speed)
{
    // Yaw: 绕 Z 轴旋转。使用 atan2(y, x)
    float yaw_rad = std::atan2(y, x);

    // Pitch: 俯仰。使用 atan2(z, sqrt(x*x + y*y))
    float ground_dist = std::sqrt(x * x + y * y);
    float pitch_rad   = std::atan2(z, ground_dist);

    // 将弧度转换为你的舵机单位 (-1280~1280 等)
    int yaw_angle   = static_cast<int>(to_degrees(yaw_rad) * 10);
    int pitch_angle = static_cast<int>(to_degrees(pitch_rad) * 10);

    moveWithSpeed(yaw_angle, pitch_angle, speed);
}

void Motion::setPose(int x_angle, int y_angle) {
    if (_yaw_servo) _yaw_servo->moveWithSpeed(x_angle, 800);
    if (_pitch_servo) _pitch_servo->moveWithSpeed(y_angle, 800);
}

void Motion::nod() {
    if (_pitch_servo) {
        // 假设 90 是平视，60 是低头
        _pitch_servo->moveWithSpeed(60, 600); // 低头
        vTaskDelay(pdMS_TO_TICKS(300));
        _pitch_servo->moveWithSpeed(90, 600); // 抬起
        vTaskDelay(pdMS_TO_TICKS(300));
        _pitch_servo->moveWithSpeed(60, 600); // 低头
        vTaskDelay(pdMS_TO_TICKS(300));
        _pitch_servo->moveWithSpeed(90, 600); // 抬起
    }
}

void Motion::shakeHead() {
    if (_yaw_servo) {
        // 假设 90 是正视，60 是左，120 是右
        _yaw_servo->moveWithSpeed(60, 600);
        vTaskDelay(pdMS_TO_TICKS(300));
        _yaw_servo->moveWithSpeed(120, 600);
        vTaskDelay(pdMS_TO_TICKS(300));
        _yaw_servo->moveWithSpeed(60, 600);
        vTaskDelay(pdMS_TO_TICKS(300));
        _yaw_servo->moveWithSpeed(90, 600); // 归位
    }
}

void Motion::lookAround() {
    if (_yaw_servo && _pitch_servo) {
        // 左上看
        _yaw_servo->moveWithSpeed(45, 500);
        _pitch_servo->moveWithSpeed(120, 500);
        vTaskDelay(pdMS_TO_TICKS(800));
        
        // 右下看
        _yaw_servo->moveWithSpeed(135, 500);
        _pitch_servo->moveWithSpeed(60, 500);
        vTaskDelay(pdMS_TO_TICKS(800));
        
        // 归中
        setPose(90, 90);
    }
}

bool Motion::isMoving()
{
    return _yaw_servo->isMoving() || _pitch_servo->isMoving();
}

int Motion::getCurrentYawAngle()
{
    return _yaw_servo->getCurrentAngle();
}

int Motion::getCurrentPitchAngle()
{
    return _pitch_servo->getCurrentAngle();
}

uitk::Vector2i Motion::getCurrentAngles()
{
    return uitk::Vector2i(_yaw_servo->getCurrentAngle(), _pitch_servo->getCurrentAngle());
}

void Motion::setTorqueEnabled(bool enabled)
{
    _yaw_servo->setTorqueEnabled(enabled);
    _pitch_servo->setTorqueEnabled(enabled);
}

void Motion::setAutoTorqueReleaseEnabled(bool enabled)
{
    _yaw_servo->setAutoTorqueReleaseEnabled(enabled);
    _pitch_servo->setAutoTorqueReleaseEnabled(enabled);
}

void Motion::setAutoAngleSyncEnabled(bool enabled)
{
    _yaw_servo->setAutoAngleSyncEnabled(enabled);
    _pitch_servo->setAutoAngleSyncEnabled(enabled);
}