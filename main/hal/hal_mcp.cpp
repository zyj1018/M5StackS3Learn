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
    // 工具 1：控制机器人做复合动作 (利用 Motion 类)
    // ---------------------------------------------------------
    mcp_server.AddTool("perform_motion", 
        "控制机器人执行预设的高级复合动作。参数 action 必须是以下之一: 'nod' (点头), 'shakeHead' (摇头), 'lookAround' (环顾四周)",
        [](const cJSON* args) -> std::string {
            if (!args || !cJSON_HasObjectItem(args, "action")) {
                return "{\"status\":\"error\", \"message\":\"Missing 'action' parameter\"}";
            }
            
            std::string action = cJSON_GetObjectItem(args, "action")->valuestring;
            
            if (global_motion) {
                if (action == "nod") {
                    global_motion->nod();
                } else if (action == "shakeHead") {
                    global_motion->shakeHead();
                } else if (action == "lookAround") {
                    global_motion->lookAround();
                } else {
                    return "{\"status\":\"error\", \"message\":\"Unknown action: " + action + "\"}";
                }
                ESP_LOGI(TAG, "MCP Tool called: perform_motion, action=%s", action.c_str());
                return "{\"status\":\"success\", \"message\":\"Action " + action + " performed\"}";
            } else {
                return "{\"status\":\"error\", \"message\":\"Motion controller is not initialized\"}";
            }
        });

    // ---------------------------------------------------------
    // 工具 2：精确控制舵机角度
    // ---------------------------------------------------------
    mcp_server.AddTool("set_servo_pose", 
        "控制机器人头部的精确角度。参数 x_angle 和 y_angle 都是 0 到 180 之间的整数（90为正前方平视）",
        [](const cJSON* args) -> std::string {
            if (!args || !cJSON_HasObjectItem(args, "x_angle") || !cJSON_HasObjectItem(args, "y_angle")) {
                return "{\"status\":\"error\", \"message\":\"Missing 'x_angle' or 'y_angle' parameter\"}";
            }
            
            int x_angle = cJSON_GetObjectItem(args, "x_angle")->valueint;
            int y_angle = cJSON_GetObjectItem(args, "y_angle")->valueint;
            
            if (global_motion) {
                global_motion->setPose(x_angle, y_angle);
                ESP_LOGI(TAG, "MCP Tool called: set_servo_pose, x=%d, y=%d", x_angle, y_angle);
                return "{\"status\":\"success\", \"message\":\"Pose updated\"}";
            } else {
                return "{\"status\":\"error\", \"message\":\"Motion controller is not initialized\"}";
            }
        });

    ESP_LOGI(TAG, "MCP Tools registration completed.");
}

} // namespace HAL