#include "alarm.hpp"

void AlarmManager::add(AlarmObserver* obj) {
    if (count < MAX_OBSERVERS) {
        alarm[count] = obj;
        count++;
    }
}

void AlarmManager::notify(AlarmType type) {
    if (type == AlarmType::kVolume)    { volume_active = true; }
    if (type == AlarmType::kOcclusion) { occlusion_active = true; }

    for (uint8_t i = 0U; i < count; i++) {
        alarm[i]->update(true);
    }
    noti_count++;
}

void AlarmManager::clear(AlarmType type) {
    if (type == AlarmType::kVolume)    { volume_active = false; }
    if (type == AlarmType::kOcclusion) { occlusion_active = false; }

    if (!volume_active && !occlusion_active) {
        for (uint8_t i = 0U; i < count; i++) {
            alarm[i]->update(false);
        }
    }
}

void AlarmManager::clearAll() {
    volume_active = false;
    occlusion_active = false;
    for (uint8_t i = 0U; i < count; i++) {
        alarm[i]->update(false);
    }
}