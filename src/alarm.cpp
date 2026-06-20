#include "alarm.hpp"

void AlarmManager::add(Alarmobserver* obj) {
    alarm[count] = obj;
    count++;
}

void AlarmManager::notify() {
    for(int i = 0; i < count; i++) {
        alarm[i]->update();
    }
    noti_count++;
}