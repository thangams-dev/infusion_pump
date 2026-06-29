#include "alarm.hpp"

void AlarmManager::add(Alarmobserver* obj) {
    alarm[count] = obj;
    count++;
}
void AlarmManager::clearAll() {
    for(uint8_t i = 0U; i < count; i++) {
        alarm[i]->clear();
    }
}
void AlarmManager::notify() {
    for(uint8_t i = 0; i < count; i++) {
        alarm[i]->update();
    }
    noti_count++;
}
