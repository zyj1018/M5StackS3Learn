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
