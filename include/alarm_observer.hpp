#pragma once

/// @brief Base class for alarm observers (Observer pattern).
class AlarmObserver {
public:
    virtual ~AlarmObserver() = default; // LCOV_EXCL_LINE

    /// @brief Reacts to alarm state change.
    /// @param active true if alarm is active.
    virtual void update(bool active) = 0;
};

/// @brief LED alarm indicator.
class led : public AlarmObserver {
public:
    void update(bool active) override;
};

/// @brief Buzzer alarm indicator.
class buz : public AlarmObserver {
public:
    void update(bool active) override;
};

/// @brief UART alarm notifier.
class uart_observer : public AlarmObserver {
public:
    void update(bool active) override;
};