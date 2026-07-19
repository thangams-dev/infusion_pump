#pragma once
#include <stdint.h>
#include "alarm_observer.hpp"

enum class AlarmType : uint8_t {
    kVolume,
    kOcclusion
};

class AlarmManager {
public:
    void add(AlarmObserver* obj);
    void notify(AlarmType type);
    void clear(AlarmType type);
    void clearAll();
    uint8_t get_notify_count() const { return noti_count; } 

private:
    static constexpr uint8_t MAX_OBSERVERS = 4U;
    AlarmObserver* alarm[MAX_OBSERVERS] = {nullptr};
    uint8_t count = 0U;
    uint8_t noti_count = 0U;

    bool volume_active = false;
    bool occlusion_active = false;
};