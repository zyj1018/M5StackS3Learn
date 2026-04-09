#include "hal.h"
#include <esp_log.h>
// MCP 相关的头文件
#include "mcp_server.h"
#include <string>
#include <vector>

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
    PropertyList perform_motion_props = {
        {"action", "string", "动作类型，必须是以下之一: 'nod' (点头), 'shakeHead' (摇头), 'lookAround' (环顾四周)"}
    };
    
    mcp_server.AddTool("perform_motion",
        "控制机器人执行预设的高级复合动作",
        perform_motion_props,
        [](const PropertyList& args) -> ReturnValue {
            std::string action = "";
            for (const auto& arg : args) {
                if (arg.name == "action") {
                    action = arg.value;
                    break;
                }
            }

            if (action.empty()) {
                return {"error", "Missing 'action' parameter"};
            }

            if (global_motion) {
                if (action == "nod") {
                    global_motion->nod();
                } else if (action == "shakeHead") {
                    global_motion->shakeHead();
                } else if (action == "lookAround") {
                    global_motion->lookAround();
                } else {
                    return {"error", "Unknown action: " + action};
                }
                ESP_LOGI(TAG, "MCP Tool called: perform_motion, action=%s", action.c_str());
                return {"success", "Action " + action + " performed"};
            } else {
                return {"error", "Motion controller is not initialized"};
            }
        });

    // ---------------------------------------------------------
    // 工具 2：精确控制舵机角度
    // ---------------------------------------------------------
    PropertyList set_servo_pose_props = {
        {"x_angle", "integer", "水平旋转角度，0 到 180 之间的整数（90为正前方平视）"},
        {"y_angle", "integer", "垂直旋转角度，0 到 180 之间的整数（90为平视）"}
    };

    mcp_server.AddTool("set_servo_pose",
        "控制机器人头部的精确角度",
        set_servo_pose_props,
        [](const PropertyList& args) -> ReturnValue {
            int x_angle = -1;
            int y_angle = -1;
            
            for (const auto& arg : args) {
                if (arg.name == "x_angle") {
                    x_angle = std::stoi(arg.value);
                } else if (arg.name == "y_angle") {
                    y_angle = std::stoi(arg.value);
                }
            }

            if (x_angle == -1 || y_angle == -1) {
                return {"error", "Missing 'x_angle' or 'y_angle' parameter"};
            }

            if (global_motion) {
                global_motion->setPose(x_angle, y_angle);
                ESP_LOGI(TAG, "MCP Tool called: set_servo_pose, x=%d, y=%d", x_angle, y_angle);
                return {"success", "Pose updated"};
            } else {
                return {"error", "Motion controller is not initialized"};
            }
        });

    ESP_LOGI(TAG, "MCP Tools registration completed.");
}

} // namespace HAL