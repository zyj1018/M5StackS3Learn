# AppManager Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Implement a lightweight AppManager to manage the SmileAvatar UI and physical motion, decoupling them from the hardware display drivers.

**Architecture:** We will create an `AppEvent` structure, an `App` base class with lifecycle methods, an `AppManager` singleton to route events, and a concrete `PenguinApp` that encapsulates the avatar UI and head motion.

**Tech Stack:** C++, ESP-IDF, LVGL

---

### Task 1: Create AppEvent and App base class

**Files:**
- Create: `main/app/app_event.h`
- Create: `main/app/app.h`

- [ ] **Step 1: Write `app_event.h`**

```cpp
#pragma once
#include <string>

enum class EventType {
    EVENT_EMOTION,
    EVENT_STATUS,
    EVENT_MESSAGE
};

struct AppEvent {
    EventType type;
    std::string data;
};
```

- [ ] **Step 2: Write `app.h`**

```cpp
#pragma once
#include "app_event.h"

class App {
public:
    virtual ~App() = default;

    virtual void onCreate() {}
    virtual void onResume() {}
    virtual void onPause() {}
    virtual void onDestroy() {}
    
    virtual void onEvent(const AppEvent& event) {}
};
```

- [ ] **Step 3: Commit**

```bash
git add main/app/app_event.h main/app/app.h
git commit -m "feat: add AppEvent and App base class"
```

### Task 2: Create AppManager singleton

**Files:**
- Create: `main/app/app_manager.h`
- Create: `main/app/app_manager.cpp`

- [ ] **Step 1: Write `app_manager.h`**

```cpp
#pragma once
#include <vector>
#include "app.h"

class AppManager {
public:
    static AppManager& GetInstance() {
        static AppManager instance;
        return instance;
    }

    void startApp(App* app);
    void exitApp();
    void notifyEvent(const AppEvent& event);

private:
    AppManager() = default;
    ~AppManager() = default;

    std::vector<App*> app_stack_;
};
```

- [ ] **Step 2: Write `app_manager.cpp`**

```cpp
#include "app_manager.h"
#include <esp_log.h>

static const char* TAG = "AppManager";

void AppManager::startApp(App* app) {
    if (!app) return;
    
    if (!app_stack_.empty()) {
        app_stack_.back()->onPause();
    }
    
    app_stack_.push_back(app);
    ESP_LOGI(TAG, "Starting new App");
    app->onCreate();
    app->onResume();
}

void AppManager::exitApp() {
    if (app_stack_.empty()) return;
    
    App* current_app = app_stack_.back();
    current_app->onPause();
    current_app->onDestroy();
    delete current_app;
    app_stack_.pop_back();
    
    if (!app_stack_.empty()) {
        app_stack_.back()->onResume();
    }
}

void AppManager::notifyEvent(const AppEvent& event) {
    if (!app_stack_.empty()) {
        app_stack_.back()->onEvent(event);
    }
}
```

- [ ] **Step 3: Commit**

```bash
git add main/app/app_manager.h main/app/app_manager.cpp
git commit -m "feat: implement AppManager singleton"
```

### Task 3: Create PenguinApp

**Files:**
- Create: `main/app/penguin_app.h`
- Create: `main/app/penguin_app.cpp`

- [ ] **Step 1: Write `penguin_app.h`**

```cpp
#pragma once
#include "app.h"
#include "ui/SmileAvatar.h"

class PenguinApp : public App {
public:
    PenguinApp();
    ~PenguinApp() override;

    void onCreate() override;
    void onDestroy() override;
    void onEvent(const AppEvent& event) override;

private:
    SmileAvatar* my_avatar_ = nullptr;
    
    void handleEmotion(const std::string& emotion);
    void handleStatus(const std::string& status);
};
```

- [ ] **Step 2: Write `penguin_app.cpp`**

```cpp
#include "penguin_app.h"
#include <esp_log.h>
#include "hal/hal.h"
#include "esp_lvgl_port.h"

static const char* TAG = "PenguinApp";

PenguinApp::PenguinApp() {}

PenguinApp::~PenguinApp() {}

void PenguinApp::onCreate() {
    ESP_LOGI(TAG, "PenguinApp created");
    if (lvgl_port_lock(0)) {
        my_avatar_ = new SmileAvatar(lv_scr_act());
        my_avatar_->setEmotion(AvatarEmotion::PLEASE);
        lvgl_port_unlock();
    }
}

void PenguinApp::onDestroy() {
    if (my_avatar_) {
        if (lvgl_port_lock(0)) {
            delete my_avatar_;
            my_avatar_ = nullptr;
            lvgl_port_unlock();
        }
    }
}

void PenguinApp::onEvent(const AppEvent& event) {
    if (event.type == EventType::EVENT_EMOTION) {
        handleEmotion(event.data);
    } else if (event.type == EventType::EVENT_STATUS) {
        handleStatus(event.data);
    }
}

void PenguinApp::handleEmotion(const std::string& emo) {
    if (!my_avatar_) return;
    
    ESP_LOGI(TAG, "LLM Emotion: %s", emo.c_str());
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;
    
    if (emo == "neutral" || emo == "thinking" || emo == "relaxed") {
        target_emo = AvatarEmotion::NEUTRAL;
        if (HAL::global_motion) HAL::global_motion->goHome();
    } else if (emo == "happy" || emo == "glad" || emo == "confident") {
        target_emo = AvatarEmotion::HAPPY;
        if (HAL::global_motion) HAL::global_motion->lookAtNormalized(0, 0.5);
    } else if (emo == "laughing" || emo == "cheer" || emo == "excited") {
        target_emo = AvatarEmotion::CHEER;
        if (HAL::global_motion) HAL::global_motion->lookAtNormalized(0, 0.8);
    } else if (emo == "angry" || emo == "mad") {
        target_emo = AvatarEmotion::ANGRY;
    } else if (emo == "sad" || emo == "sorrow" || emo == "crying" || emo == "embarrassed") {
        target_emo = AvatarEmotion::SAD;
        if (HAL::global_motion) HAL::global_motion->lookAtNormalized(0, -0.8);
    } else if (emo == "fear" || emo == "scared") {
        target_emo = AvatarEmotion::FEAR;
    } else if (emo == "surprised" || emo == "shocked" || emo == "confused") {
        target_emo = AvatarEmotion::SURPRISE;
    } else if (emo == "cute" || emo == "kawaii" || emo == "loving" || emo == "kissy" || emo == "delicious") {
        target_emo = AvatarEmotion::CUTE;
    } else if (emo == "naughty" || emo == "playful" || emo == "funny" || emo == "silly" || emo == "winking") {
        target_emo = AvatarEmotion::NAUGHTY;
    } else if (emo == "please" || emo == "pleading") {
        target_emo = AvatarEmotion::PLEASE;
    } else if (emo == "mock" || emo == "sarcastic" || emo == "cool") {
        target_emo = AvatarEmotion::MOCK;
    } else if (emo == "disdain" || emo == "disgust") {
        target_emo = AvatarEmotion::DISDAIN;
    } else if (emo == "microchip_ai" || emo == "boring" || emo == "sleepy") {
        target_emo = AvatarEmotion::ROLL_EYES;
    } else {
        target_emo = AvatarEmotion::NEUTRAL;
    }
    
    if (lvgl_port_lock(0)) {
        my_avatar_->setEmotion(target_emo);
        lvgl_port_unlock();
    }
}

void PenguinApp::handleStatus(const std::string& stat) {
    if (!my_avatar_) return;
    
    ESP_LOGI(TAG, "LLM Status: %s", stat.c_str());
    AvatarEmotion target_emo = AvatarEmotion::NEUTRAL;
    bool need_change = false;
    
    if (stat == "聆听中" || stat == "Listening") {
        target_emo = AvatarEmotion::PLEASE;
        need_change = true;
    } else if (stat == "连接中" || stat == "Connecting" || stat == "检查新版本...") {
        target_emo = AvatarEmotion::ROLL_EYES;
        need_change = true;
    }
    
    if (need_change) {
        if (lvgl_port_lock(0)) {
            my_avatar_->setEmotion(target_emo);
            lvgl_port_unlock();
        }
    }
}
```

- [ ] **Step 3: Commit**

```bash
git add main/app/penguin_app.h main/app/penguin_app.cpp
git commit -m "feat: implement PenguinApp with UI and motion integration"
```

### Task 4: Refactor MyDisplay and Main

**Files:**
- Modify: `main/board/my_display.cc`
- Modify: `main/main.cpp`

- [ ] **Step 1: Update `my_display.cc`**

Open `main/board/my_display.cc` and:
1. Replace `#include "ui/SmileAvatar.h"` with `#include "app/app_manager.h"`
2. Remove `extern SmileAvatar* my_avatar;`
3. In `MyDisplay::MyDisplay` constructor, delete the block `if (lvgl_port_lock(0)) { my_avatar = new SmileAvatar... }`
4. Rewrite `SetEmotion`:
```cpp
void MyDisplay::SetEmotion(const char* emotion) {
    if (!emotion) return;
    ESP_LOGI(TAG, "大模型下发情绪标签: %s", emotion);
    AppManager::GetInstance().notifyEvent({EventType::EVENT_EMOTION, emotion});
}
```
5. Rewrite `SetStatus`:
```cpp
void MyDisplay::SetStatus(const char* status) {
    if (!status) return;
    ESP_LOGE(TAG, "SetStatus: %s", status);
    AppManager::GetInstance().notifyEvent({EventType::EVENT_STATUS, status});
}
```

- [ ] **Step 2: Update `main.cpp`**

Open `main/main.cpp` and:
1. Remove `#include "ui/SmileAvatar.h"`
2. Include `#include "app/app_manager.h"` and `#include "app/penguin_app.h"`
3. Remove `SmileAvatar* my_avatar = nullptr;`
4. Before `Application::GetInstance().Start();`, add:
```cpp
    AppManager::GetInstance().startApp(new PenguinApp());
```

- [ ] **Step 3: Commit**

```bash
git add main/board/my_display.cc main/main.cpp
git commit -m "refactor: integrate AppManager into MyDisplay and main"
```

### Task 5: Update CMakeLists.txt

**Files:**
- Modify: `main/CMakeLists.txt`

- [ ] **Step 1: Update CMakeLists.txt**

Add `app/app_manager.cpp` and `app/penguin_app.cpp` to `PENGUIN_SOURCES`.
Add `"app"` to `MY_INCLUDE_DIRS`.

- [ ] **Step 2: Verify Build**

Run `idf.py build` (or similar build command if applicable) to ensure the project compiles without errors.

- [ ] **Step 3: Commit**

```bash
git add main/CMakeLists.txt
git commit -m "build: add app manager to CMake configuration"
```
