# M5StackS3Learn / M5StackS3Learn

基于 ESP-IDF 的 ESP32-S3（M5Stack CoreS3）工程：通过 LVGL 渲染可动表情头像，并将 “xiaozhi-esp32” 的音频管线、网络协议与状态机集成到本工程中，实现设备端语音/联网/协议/交互的一体化运行。

An ESP-IDF project targeting ESP32-S3 (M5Stack CoreS3): renders an animated avatar via LVGL and integrates the “xiaozhi-esp32” audio pipeline, network protocols, and state machine into this repository for an end-to-end device runtime.

<a id="toc"></a>
## 目录 / Table of Contents
- [项目概述 / Overview](#overview)
- [代码目录结构 / Repository Layout](#layout)
- [版本历史 / Version History](#history)
- [系统环境要求 / System Requirements](#requirements)
- [开发环境配置 / Dev Environment Setup](#dev-setup)
- [安装与启动 / Install & Run](#install-run)
- [核心功能示例 / Core Usage Examples](#examples)
- [API 文档链接 / API Docs Links](#api-docs)
- [测试运行方法 / Testing](#testing)
- [部署指南 / Deployment](#deployment)
- [常见问题 / FAQ](#faq)
- [贡献者规范 / Contributing](#contributing)
- [许可证 / License](#license)
- [联系方式 / Contact](#contact)

<a id="overview"></a>
## 项目概述 / Overview

### 功能概览 / Features
- 表情头像 UI（LVGL）：头像表情切换、气泡文本、UI 线程安全（LVGL port lock）
- CoreS3 板级适配：屏幕/背光/触摸/音频外设初始化与透传
- 集成 xiaozhi-esp32：音频处理、协议栈（WebSocket/MQTT/MCP 等）、资源（多语言提示音/字体等）、OTA/设置等模块（按本工程 CMake 选择性编译）

- Avatar UI (LVGL): emotion switching, bubble text, UI thread-safety via LVGL port lock
- CoreS3 board adaptation: display/backlight/touch/audio initialization and handle passthrough
- xiaozhi-esp32 integration: audio pipeline, protocol stack (WebSocket/MQTT/MCP), assets (multi-language sounds/fonts), OTA/settings (selectively compiled by this project's CMake)

<a id="layout"></a>
## 代码目录结构 / Repository Layout

### 顶层结构（概要）/ Top-level Tree (Summary)
```text
M5StackS3Learn/
├─ .devcontainer/                 # 容器化开发环境（可选）
├─ .vscode/                       # VS Code 任务/设置（可选）
├─ main/                          # 本工程主组件（ESP-IDF main component）
│  ├─ main.cpp                    # app_main：初始化板级、LVGL、创建头像、启动 Application
│  ├─ CMakeLists.txt              # 将本工程源与 xiaozhi-esp32/main 源合并注册为单一组件
│  ├─ idf_component.yml           # ESP-IDF Component Manager 依赖清单（含版本约束）
│  ├─ Kconfig.projbuild           # menuconfig 入口（继承/扩展选项）
│  ├─ ui/
│  │  └─ SmileAvatar.hpp          # LVGL 头像 UI（头文件实现）
│  └─ board/
│     ├─ m5stack_core_s3.cc       # CoreS3 板级适配/接管与透传（屏幕句柄等）
│     ├─ cores3_audio_codec.*     # 音频 codec 适配（本工程侧封装）
│     ├─ my_display.*             # Display 代理：将“小智情绪/状态”映射到头像表情
│     ├─ config.h / config.json   # 板级配置（显示尺寸等）
│     └─ README.md                # 板级说明
├─ xiaozhi-esp32/                 # 外部工程源码与文档（作为源码目录直接编译部分文件）
├─ CMakeLists.txt                 # 工程入口（含 PROJECT_VER）
├─ sdkconfig / sdkconfig.defaults # 本地配置与默认配置
├─ partitions.csv                 # 分区表
├─ repos.json / fetch_repos.py    # 第三方仓库清单与拉取脚本（写入 components/）
├─ dependencies.lock              # 依赖锁文件（脚本侧使用）
└─ CodeArchitecture.md            # 架构说明（可能与当前目录略有出入，以源码为准）
```

### 关键模块说明 / Key Modules
- 本工程 UI：`main/ui/SmileAvatar.hpp`
- “小智”到 UI 的桥接：`main/board/my_display.cc`（字符串情绪 → 枚举 AvatarEmotion）
- 集成入口：`main/CMakeLists.txt`（将 `xiaozhi-esp32/main/*` 的源文件列表转换为绝对路径并编译）

The integration boundary is `main/CMakeLists.txt`: it directly compiles selected sources under `xiaozhi-esp32/main/` into this project's main component.

<a id="history"></a>
## 版本历史 / Version History

| 版本 / Version | 日期 / Date | 变更 / Changes |
|---|---:|---|
| 2.0.4 | - | 工程版本号来自根目录 [CMakeLists.txt](CMakeLists.txt) 的 `PROJECT_VER`。仓库未维护更细粒度的版本变更日志。 |

<a id="requirements"></a>
## 系统环境要求 / System Requirements

### 操作系统 / OS
- Windows 10/11（推荐使用 Espressif 官方工具安装器与 “ESP-IDF PowerShell”）
- Linux / macOS（使用官方脚本安装 ESP-IDF 工具链）

### 运行时环境 / Runtime
- Python：随 ESP-IDF Tools 安装（版本范围由所选 ESP-IDF 决定）
- CMake：最低 3.16（见根目录 [CMakeLists.txt](CMakeLists.txt)）
- Git（用于拉取仓库与依赖）

Python/CMake/Git are required to configure and build with ESP-IDF.

### 关键依赖版本（Component Manager）/ Key Dependencies (Component Manager)
依赖以 [main/idf_component.yml](main/idf_component.yml) 为准，核心项如下（示例）：
- ESP-IDF：本仓库 `sdkconfig` 显示 `CONFIG_IDF_INIT_VERSION="5.5.4"`
- LVGL：`lvgl/lvgl: ~9.3.0`
- esp_lvgl_port：`~2.6.0`
- esp_codec_dev：`~1.5`
- 78/esp-wifi-connect：`~2.6.1`
- 78/xiaozhi-fonts：`~1.5.4`

<a id="dev-setup"></a>
## 开发环境配置 / Dev Environment Setup

### Windows（ESP-IDF 工具链）/ Windows (ESP-IDF Toolchain)
1. 安装 Espressif 官方 “ESP-IDF Tools Installer”（选择 ESP-IDF 5.5.x）
2. 从开始菜单启动 “ESP-IDF PowerShell”
3. 在该终端进入仓库根目录，执行后续 `idf.py` 命令

Install ESP-IDF via the official installer, then run all commands inside the ESP-IDF PowerShell.

### VS Code（可选）/ VS Code (Optional)
1. 安装 Espressif IDF 插件
2. 在插件中选择已安装的 ESP-IDF（与步骤一致）

### Dev Container（可选）/ Dev Container (Optional)
仓库包含 [.devcontainer/devcontainer.json](.devcontainer/devcontainer.json)，用于在容器中准备 ESP-IDF 开发环境（需要 Docker）。

<a id="install-run"></a>
## 项目安装与启动流程 / Install & Run

### 1) 获取代码 / Clone
```bash
git clone <repo-url>
cd M5StackS3Learn
```

### 2) 拉取第三方依赖（写入 components/）/ Fetch 3rd-party deps (into components/)
```bash
python fetch_repos.py
```

### 3) 拉取 Component Manager 依赖（写入 managed_components/）/ Fetch managed components
```bash
idf.py reconfigure
```

### 4) 编译 / Build
```bash
idf.py set-target esp32s3
idf.py build
```

### 5) 烧录并监视 / Flash & Monitor
```bash
idf.py -p COMx flash monitor
```

退出串口监视 / Exit monitor：`Ctrl+]`

### 6) 首次联网配置（按小智网络流程）/ First-time Wi-Fi Provisioning
本工程启动后由 “xiaozhi-esp32” 的板级网络流程接管 Wi-Fi 配置：当设备未保存 Wi-Fi 列表时，会进入配网模式（启动配置 AP 并提示访问地址）。相关实现见 `xiaozhi-esp32/main/boards/common/wifi_board.cc`。

Wi-Fi provisioning is handled by the “xiaozhi-esp32” board network flow. If no SSID is stored, the device enters provisioning mode (starts a config AP and prints the access URL).

<a id="examples"></a>
## 核心功能使用示例 / Core Usage Examples

### 1) 从字符串情绪驱动头像 / Drive avatar emotion from string
文件：`main/board/my_display.cc`
```cpp
void MyDisplay::SetEmotion(const char* emotion) {
    if (!my_avatar || !emotion) return;

    std::string emo(emotion);
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;

    if (emo == "happy" || emo == "glad") target_emo = AvatarEmotion::HAPPY;
    else if (emo == "angry" || emo == "mad") target_emo = AvatarEmotion::ANGRY;
    else if (emo == "microchip_ai" || emo == "boring") target_emo = AvatarEmotion::ROLL_EYES;

    if (lvgl_port_lock(0)) {
        my_avatar->setEmotion(target_emo);
        lvgl_port_unlock();
    }
}
```

### 2) 启动流程（板级 → LVGL → 头像 → 小智应用）/ Boot flow
文件：`main/main.cpp`
```cpp
extern "C" void app_main(void) {
    nvs_flash_init();
    Board& board = Board::GetInstance();

    lvgl_port_init(&ESP_LVGL_PORT_INIT_CONFIG());
    lvgl_port_add_disp(&disp_cfg);

    lvgl_port_lock(0);
    my_avatar = new SmileAvatar(lv_scr_act());
    lvgl_port_unlock();

    Application::GetInstance().Start();
}
```

<a id="api-docs"></a>
## API 接口文档链接 / API Docs Links

协议与对接文档集中在 `xiaozhi-esp32/docs/`：
- WebSocket：[websocket.md](xiaozhi-esp32/docs/websocket.md)
- MQTT/UDP：[mqtt-udp.md](xiaozhi-esp32/docs/mqtt-udp.md)
- MCP 协议：[mcp-protocol.md](xiaozhi-esp32/docs/mcp-protocol.md)
- MCP 使用：[mcp-usage.md](xiaozhi-esp32/docs/mcp-usage.md)
- 自定义板卡：[custom-board.md](xiaozhi-esp32/docs/custom-board.md)

<a id="testing"></a>
## 测试运行方法 / Testing

本仓库未提供独立的自动化测试用例目录。当前验证方式以固件构建与板端集成验证为主：

No standalone automated test suite is provided. Validation is based on firmware build + on-device integration check:
```bash
idf.py build
idf.py -p COMx flash monitor
```

<a id="deployment"></a>
## 部署指南 / Deployment

### 生成可分发固件 / Generate distributable binaries
```bash
idf.py build
idf.py merge-bin
```

### 量产/离线烧录（示例）/ Mass flashing (example)
使用 `merge-bin` 产物配合 `esptool.py` 或量产工具进行烧录；分区布局见 [partitions.csv](partitions.csv)。

Use the merged binary set for offline flashing; the partition layout is defined in `partitions.csv`.

<a id="faq"></a>
## 常见问题解答 / FAQ

### 1) 链接错误：`undefined reference to app_main`
入口函数位于 C++ 文件时，必须导出 C 符号：
```cpp
extern "C" void app_main(void);
```

### 2) 依赖目录为什么是空的？
`components/` 与 `managed_components/` 默认被忽略提交，按需通过脚本与 `idf.py` 拉取：
```bash
python fetch_repos.py
idf.py reconfigure
```

### 3) 为什么 README/架构文档里的目录与当前仓库不一致？
以实际源码树为准，优先参考本 README 的 “代码目录结构” 与 `main/CMakeLists.txt` 的源文件清单。

<a id="contributing"></a>
## 贡献者规范 / Contributing

### 提交规范 / Change Guidelines
- 保持与现有工程一致的 ESP-IDF 组件组织方式（`main/` 为唯一 main 组件，依赖使用 `idf_component.yml` 管理）
- 修改跨工程集成行为时，同步检查 `main/CMakeLists.txt` 中的 xiaozhi 源文件列表与 include 路径
- 严禁在仓库中提交明文凭据（Wi-Fi、Token、Key 等）

- Keep the ESP-IDF component layout (single `main/` component; dependencies via `idf_component.yml`)
- When changing integration behavior, update the xiaozhi source list/include paths in `main/CMakeLists.txt`
- Never commit plaintext secrets (Wi-Fi, tokens, keys)

### 代码风格 / Code Style
- 遵循现有文件的格式与命名习惯（本工程以 C++ 为主，构建系统为 CMake/ESP-IDF）

<a id="license"></a>
## 许可证信息 / License

本仓库根目录未包含独立 LICENSE 文件；`xiaozhi-esp32/` 子目录包含其自身许可证文件：`xiaozhi-esp32/LICENSE`。整体仓库的许可证归属以项目维护者声明为准。

<a id="contact"></a>
## 联系方式 / Contact

- 维护者 / Maintainer：<填写维护者信息>
- 邮箱 / Email：<填写邮箱>
- Issue/反馈 / Issues：<填写仓库 Issue 地址>
