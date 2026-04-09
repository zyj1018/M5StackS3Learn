#include "hal.h"
#include <esp_log.h>
// MCP 相关的头文件
// 注意：以下头文件路径可能需要根据你实际克隆下来的 xiaozhi-esp32 仓库路径进行微调
// 常见的注册单例为 McpServer::GetInstance().AddTool() 或者 Application::GetInstance().AddMcpTool()
#include "mcp_server.h"
#include <string>

static const char* TAG = "HAL_MCP";

namespace HAL {

/**
 * @brief 注册 MCP 工具，允许大模型在对话中主动调用舵机控制
 */
void xiaozhi_mcp_init() {
    ESP_LOGI(TAG, "Registering MCP Tools for Xiaozhi...");

    // 获取小智的 MCP Server 实例
    auto& mcp_server = McpServer::GetInstance();

    // ---------------------------------------------------------
    // 工具 1：控制水平方向舵机 (X轴)
    // ---------------------------------------------------------
    mcp_server.AddTool("control_servo_x", 
        "控制机器人头部的水平旋转。参数 angle 是 0 到 180 之间的整数（90为正前方，0为最左，180为最右）",
        [](const cJSON* args) -> std::string {
            if (!args || !cJSON_HasObjectItem(args, "angle")) {
                return "{\"status\":\"error\", \"message\":\"Missing 'angle' parameter\"}";
            }
            
            int angle = cJSON_GetObjectItem(args, "angle")->valueint;
            
            if (servo_x) {
                // 平滑移动到指定角度，15ms的步进延迟
                servo_x->smoothMove(angle, 15);
                ESP_LOGI(TAG, "MCP Tool called: control_servo_x, angle=%d", angle);
                return "{\"status\":\"success\", \"message\":\"Servo X moved\"}";
            } else {
                return "{\"status\":\"error\", \"message\":\"Servo X is not initialized\"}";
            }
        });

    // ---------------------------------------------------------
    // 工具 2：控制垂直方向舵机 (Y轴)
    // ---------------------------------------------------------
    mcp_server.AddTool("control_servo_y", 
        "控制机器人头部的垂直旋转（点头/抬头）。参数 angle 是 0 到 180 之间的整数（90为平视）",
        [](const cJSON* args) -> std::string {
            if (!args || !cJSON_HasObjectItem(args, "angle")) {
                return "{\"status\":\"error\", \"message\":\"Missing 'angle' parameter\"}";
            }
            
            int angle = cJSON_GetObjectItem(args, "angle")->valueint;
            
            if (servo_y) {
                servo_y->smoothMove(angle, 15);
                ESP_LOGI(TAG, "MCP Tool called: control_servo_y, angle=%d", angle);
                return "{\"status\":\"success\", \"message\":\"Servo Y moved\"}";
            } else {
                return "{\"status\":\"error\", \"message\":\"Servo Y is not initialized\"}";
            }
        });

    ESP_LOGI(TAG, "MCP Tools registration completed.");
}

} // namespace HAL