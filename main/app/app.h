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
