# 轻量级 AppManager 与 PenguinApp 架构设计

## 1. 概述
当前项目中，UI 实例 (`SmileAvatar`) 直接在底层屏幕驱动 (`MyDisplay`) 中实例化和被调用，而舵机控制 (`Motion`) 仅通过全局指针暴露，缺乏统一的应用层管理。
为了支持未来可能扩展的多个应用场景（如设置页、时钟页、游戏页等），并解耦硬件驱动与上层应用逻辑，本项目将引入一套轻量级的 `AppManager`。

`AppManager` 将统一管理应用程序的生命周期（初始化、前后台切换、销毁），同时通过事件总线机制将大模型（LLM）下发的情绪与状态事件路由到当前前台的 App。

## 2. 核心架构组件

### 2.1 事件系统 (`AppEvent`)
将来自小智大脑（如 `MyDisplay` 拦截到的数据）的信息封装为统一的事件结构体，避免底层驱动直接调用上层 UI。
*   **EventType**: 枚举，包含 `EVENT_EMOTION`（情绪事件）、`EVENT_STATUS`（状态事件）、`EVENT_MESSAGE`（消息事件）等。
*   **AppEvent**: 包含事件类型和字符串数据（例如 `emotion: "happy"`, `status: "聆听中"`）。

### 2.2 App 基类 (`App`)
所有的应用都必须继承自 `App` 抽象基类。
*   **生命周期方法**：
    *   `virtual void onCreate()`: App 首次创建时调用，用于初始化 LVGL 对象、分配内存。
    *   `virtual void onResume()`: App 切换到前台时调用，用于恢复动画、注册传感器监听。
    *   `virtual void onPause()`: App 切换到后台时调用，用于暂停动画、保存状态。
    *   `virtual void onDestroy()`: App 被销毁时调用，用于清理内存、销毁 LVGL 对象。
*   **事件处理方法**：
    *   `virtual void onEvent(const AppEvent& event)`: 处理来自 `AppManager` 分发的事件。

### 2.3 应用管理器 (`AppManager`)
单例模式，负责管理所有 `App` 的生命周期和事件分发。
*   **内部状态**：使用 `std::vector<App*>` 作为 App 栈（Stack），最后 push 的 App 即为当前前台应用。
*   **核心接口**：
    *   `void startApp(App* app)`: 将应用压栈，并依次调用其 `onCreate` 和 `onResume`。
    *   `void exitApp()`: 将当前前台应用出栈，并依次调用其 `onPause` 和 `onDestroy`。如果栈非空，则调用新栈顶的 `onResume`。
    *   `void notifyEvent(const AppEvent& event)`: 将接收到的事件投递给栈顶的 `App` 的 `onEvent`。

### 2.4 PenguinApp (企鹅主应用)
本项目的默认应用，继承自 `App`。负责将 `SmileAvatar` 和 `Motion` 结合在一起。
*   `onCreate`: 在加锁状态下实例化 `SmileAvatar`。
*   `onEvent`:
    *   **处理 `EVENT_EMOTION`**：解析情绪字符串，转换为 `AvatarEmotion` 并调用 `SmileAvatar::setEmotion`；同时根据情绪类型，调用 `HAL::global_motion` 执行真实的物理动作（如开心时摇头、悲伤时低头）。
    *   **处理 `EVENT_STATUS`**：解析系统状态，控制 UI 反馈，并调整头部物理姿态。
*   `onDestroy`: 销毁 `SmileAvatar` 实例。

## 3. 集成与重构计划

### 3.1 移除全局变量
*   删除 `main.cpp` 和 `my_display.cc` 中的 `extern SmileAvatar* my_avatar`。
*   删除 `my_display.cc` 中硬编码的 `my_avatar = new SmileAvatar(...)`。

### 3.2 MyDisplay 改造
*   在 `SetEmotion` 和 `SetStatus` 方法中，不再解析情绪细节，而是将原始字符串打包为 `AppEvent` 并调用 `AppManager::GetInstance().notifyEvent(...)`。

### 3.3 Main.cpp 启动流程改造
*   在 `HAL::init()` 和 `Board::GetInstance()` 之后，初始化 `AppManager`。
*   创建 `PenguinApp` 实例并调用 `AppManager::GetInstance().startApp(new PenguinApp())`。
*   随后再启动小智大脑 `Application::GetInstance().Start()`。

## 4. 依赖关系
*   `AppManager` 和 `App` 只依赖标准库和 `AppEvent`。
*   `PenguinApp` 依赖 `SmileAvatar` (LVGL) 和 `HAL::global_motion` (Motion 控制器)。
*   `MyDisplay` 仅依赖 `AppManager` 和 `AppEvent`，彻底与具体的 `SmileAvatar` 解耦。
