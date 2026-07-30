#pragma once
#include <stdint.h>
#include "alarm_observer.hpp"

/// @brief Alarm categories tracked independently by AlarmManager.
enum class AlarmType : uint8_t {
    kVolume,
    kOcclusion
};

/// @brief Subject (Observer pattern) - tracks alarm states and notifies all observers.
class AlarmManager {
public:
    /// @brief Registers an observer to receive alarm updates.
    void add(AlarmObserver* obj);

    /// @brief Raises the given alarm type and notifies all observers.
    void notify(AlarmType type);

    /// @brief Clears the given alarm type; notifies observers only if all alarms are clear.
    void clear(AlarmType type);

    /// @brief Clears all alarms and notifies observers.
    void clearAll();

    /// @brief Total number of notify() calls so far.
    uint8_t get_notify_count() const { return noti_count; }

private:
    static constexpr uint8_t MAX_OBSERVERS = 4U;
    AlarmObserver* alarm[MAX_OBSERVERS] = {nullptr};
    uint8_t count = 0U;
    uint8_t noti_count = 0;

    bool volume_active = false;
    bool occlusion_active = false;
};