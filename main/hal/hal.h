#pragma once

#include "sg90Servo.h"
#include "my_stack/motion/motion.h"

namespace HAL {

// 暴露全局的舵机控制指针，以便在其它文件 (如 my_display.cc 或 application.cc) 中直接调用
extern SG90Servo* servo_x; // 例如：水平方向舵机
extern SG90Servo* servo_y; // 例如：垂直方向舵机

// 暴露高级 Motion 实例指针
namespace stackchan {
namespace motion {
class Motion;
}
}
extern stackchan::motion::Motion* global_motion;

/**
 * @brief MCP 工具注册初始化
 * 
 * 将舵机等硬件控制功能注册给小智大脑 (MCP Server)
 */
void xiaozhi_mcp_init();

/**
 * @brief 硬件抽象层初始化
 * 
 * 包含整个系统级别的硬件及基础服务的初始化，例如 NVS (Non-Volatile Storage) 的挂载等。
 * 建议在 app_main() 启动的最早期调用。
 */
void init();

} // namespace HAL
