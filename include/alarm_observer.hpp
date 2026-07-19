#pragma once

/// @brief Abstract base class for alarm observers.
class AlarmObserver {
public:
    virtual void update(bool active) = 0;
};

/// @brief LED alarm observer.
class led : public AlarmObserver {
public:
    void update(bool active) override;
};

/// @brief Buzzer alarm observer.
class buz : public AlarmObserver {
public:
    void update(bool active) override;
};

/// @brief UART alarm observer - sends alarm notification over UART.
class uart_observer : public AlarmObserver {
public:
    void update(bool active) override;
};