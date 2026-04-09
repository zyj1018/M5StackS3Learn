#pragma once

namespace HAL {

/**
 * @brief 硬件抽象层初始化
 * 
 * 包含整个系统级别的硬件及基础服务的初始化，例如 NVS (Non-Volatile Storage) 的挂载等。
 * 建议在 app_main() 启动的最早期调用。
 */
void init();

} // namespace HAL
