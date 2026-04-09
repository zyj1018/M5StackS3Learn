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
