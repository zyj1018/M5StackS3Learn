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
