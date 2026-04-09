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
    PropertyList perform_motion_props;
    perform_motion_props.AddProperty(Property("action", kPropertyTypeString));
    
    mcp_server.AddTool("perform_motion",
        "控制机器人执行预设的高级复合动作。参数 action 必须是: 'nod', 'shakeHead', 'lookAround'",
        perform_motion_props,
        [](const PropertyList& args) -> ReturnValue {
            std::string action = "";
            try {
                action = args["action"].value<std::string>();
            } catch (const std::exception& e) {
                return std::string("{\"status\":\"error\", \"message\":\"Missing 'action' parameter\"}");
            }

            if (action.empty()) {
                return std::string("{\"status\":\"error\", \"message\":\"Missing 'action' parameter\"}");
            }

            if (global_motion) {
                if (action == "nod") {
                    global_motion->nod();
                } else if (action == "shakeHead") {
                    global_motion->shakeHead();
                } else if (action == "lookAround") {
                    global_motion->lookAround();
                } else {
                    return std::string("{\"status\":\"error\", \"message\":\"Unknown action: " + action + "\"}");
                }
                ESP_LOGI(TAG, "MCP Tool called: perform_motion, action=%s", action.c_str());
                return std::string("{\"status\":\"success\", \"message\":\"Action " + action + " performed\"}");
            } else {
                return std::string("{\"status\":\"error\", \"message\":\"Motion controller is not initialized\"}");
            }
        });

    // ---------------------------------------------------------
    // 工具 2：精确控制舵机角度
    // ---------------------------------------------------------
    PropertyList set_servo_pose_props;
    set_servo_pose_props.AddProperty(Property("x_angle", kPropertyTypeInteger, 0, 180));
    set_servo_pose_props.AddProperty(Property("y_angle", kPropertyTypeInteger, 0, 180));

    mcp_server.AddTool("set_servo_pose",
        "控制机器人头部的精确角度, 90为正前方平视",
        set_servo_pose_props,
        [](const PropertyList& args) -> ReturnValue {
            int x_angle = -1;
            int y_angle = -1;
            
            try {
                x_angle = args["x_angle"].value<int>();
                y_angle = args["y_angle"].value<int>();
            } catch (const std::exception& e) {
                return std::string("{\"status\":\"error\", \"message\":\"Missing or invalid 'x_angle' or 'y_angle' parameter\"}");
            }
            
            // 假设默认返回值为 0 表示未获取到（如果小智底层没有默认返回处理，这步可根据需求简化）
            if (x_angle < 0 || y_angle < 0) {
                return std::string("{\"status\":\"error\", \"message\":\"Missing or invalid 'x_angle' or 'y_angle' parameter\"}");
            }

            if (global_motion) {
                global_motion->setPose(x_angle, y_angle);
                ESP_LOGI(TAG, "MCP Tool called: set_servo_pose, x=%d, y=%d", x_angle, y_angle);
                return std::string("{\"status\":\"success\", \"message\":\"Pose updated\"}");
            } else {
                return std::string("{\"status\":\"error\", \"message\":\"Motion controller is not initialized\"}");
            }
        });

    ESP_LOGI(TAG, "MCP Tools registration completed.");
}

} // namespace HAL