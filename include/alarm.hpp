#pragma once
#include <cstdint>
/// @brief Abstract observer for alarm events
class Alarmobserver {
public:
    /// @brief Called when alarm is triggered
    virtual void update() = 0;
    virtual void clear() = 0;
};

/// @brief Buzzer alarm observer
class buz : public Alarmobserver {
public:
    void update();
    void clear();
};

/// @brief LED alarm observer
class led : public Alarmobserver {
public:
    void update();
    void clear();
};

/// @brief Manages and notifies alarm observers
class AlarmManager {
public:
    static constexpr uint8_t MAX_OBSERVERS = 10U;
    Alarmobserver* alarm[10] = {nullptr};
    uint16_t noti_count = 0;
    uint8_t count = 0;

    /// @brief Add observer @param obj pointer to observer
    void add(Alarmobserver* obj);
    /// @brief To clear all indication
    void clearAll();
    /// @brief Notify all observers
    void notify();
};