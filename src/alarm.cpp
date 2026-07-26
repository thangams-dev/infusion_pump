#include "alarm.hpp"

/// @brief Registers an observer, up to MAX_OBSERVERS.
void AlarmManager::add(AlarmObserver* obj) {
    if (count < MAX_OBSERVERS) {
        alarm[count] = obj;
        count++;
    }
}

/// @brief Raises given alarm type and notifies all observers.
void AlarmManager::notify(AlarmType type) {
    if (type == AlarmType::kVolume)    { volume_active = true; }
    if (type == AlarmType::kOcclusion) { occlusion_active = true; }

    for (uint8_t i = 0U; i < count; i++) {
        alarm[i]->update(true);
    }
    noti_count++;
}

/// @brief Clears given alarm type; notifies observers only once all alarms are clear.
void AlarmManager::clear(AlarmType type) {
    if (type == AlarmType::kVolume)    { volume_active = false; }
    if (type == AlarmType::kOcclusion) { occlusion_active = false; }

    if (!volume_active && !occlusion_active) {
        for (uint8_t i = 0U; i < count; i++) {
            alarm[i]->update(false);
        }
    }
}

/// @brief Clears all alarms and notifies observers.
void AlarmManager::clearAll() {
    volume_active = false;
    occlusion_active = false;
    for (uint8_t i = 0U; i < count; i++) {
        alarm[i]->update(false);
    }
}