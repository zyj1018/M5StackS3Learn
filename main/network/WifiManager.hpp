// main/network/WifiManager.hpp
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// 替换为你家的 Wi-Fi 名称和密码
#define ESP_WIFI_SSID      "Xiaomi 15 Pro"
#define ESP_WIFI_PASS      "zyj.1018"
#define MAXIMUM_RETRY      5

namespace Network {

    // 定义 FreeRTOS 事件组标志位
    const int WIFI_CONNECTED_BIT = BIT0;
    const int WIFI_FAIL_BIT      = BIT1;

    static EventGroupHandle_t s_wifi_event_group;
    static const char *TAG = "WIFI_MANAGER";
    static int s_retry_num = 0;

    // Wi-Fi 异步事件回调函数
    static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
            ESP_LOGI(TAG, "正在连接 Wi-Fi...");
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            if (s_retry_num < MAXIMUM_RETRY) {
                esp_wifi_connect();
                s_retry_num++;
                ESP_LOGI(TAG, "重试连接 Wi-Fi (%d/%d)...", s_retry_num, MAXIMUM_RETRY);
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                ESP_LOGE(TAG, "连接 Wi-Fi 失败！");
            }
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "连接成功！获取到 IP 地址: " IPSTR, IP2STR(&event->ip_info.ip));
            s_retry_num = 0;
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }

    // Wi-Fi 初始化核心函数
    inline void init_wifi_sta(void) {
        // 1. 初始化 NVS (Wi-Fi 驱动的硬性要求)
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        s_wifi_event_group = xEventGroupCreate();

        // 2. 初始化底层网络接口和默认事件循环
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_sta();

        // 3. 配置 Wi-Fi 驱动参数
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        // 4. 注册事件回调监听器 (监听 Wi-Fi 状态和 IP 分配)
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
        ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

        // 5. 写入账号密码
        wifi_config_t wifi_config = {};
        snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ESP_WIFI_SSID);
        snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", ESP_WIFI_PASS);
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        // 6. 启动 Wi-Fi
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    }
}