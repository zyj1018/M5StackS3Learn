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
