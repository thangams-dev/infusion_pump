    #include "infusion.hpp"
    #include <stdlib.h>
    #include <cmath>
    #ifdef UNIT_TEST
    #include "stub_file.hpp"
    #include <cstdio>
    #define printk printf
    #else
    #include "hardware.hpp"
    #include <zephyr/sys/printk.h>
    #endif

    // Motor/timing conversion constants
    static constexpr float sec_per_hr          = 3600.0F;   // seconds in an hour, for mL/hr -> mL/s
    static constexpr float steps_per_ml        = 32000.0F;   // stepper steps needed per mL delivered
    static constexpr float usec_per_sec        = 1000000.0F; // microseconds in a second, for step delay calc
    static constexpr float ms_per_tick         = 10.0F;      // duration of one control-loop tick, in ms
    static constexpr float ms_per_hr           = 3600000.0F; // milliseconds in an hour, for elapsed-time calc
    static constexpr int64_t alarm_throttle_ms = 2000;        // min gap between repeated volume alerts
    static int64_t last_ramp_print = 0;

    /// @brief Template method: It follow the same structure here,
    void InfusionMode::run() {
        float rate = computeTargetRate();  
        applyRate(rate);                   
        checkAlarm(rate);                  
    }

    /// @brief Converts target rate (mL/hr) into stepper delay and starts/refreshes motor output
    void InfusionMode::applyRate(float rate) {
        if (!started_) { // pass only first time
            volume.encoder_offset = static_cast<float>(get_encoder_position());
            start_ms_ = k_uptime_get();     // mark infusion start time, once
            started_  = true;
        }
        if (rate < 0.001F) { return; }      // guard against divide-by-zero on near-zero rate

        ml_per_sec    = rate / sec_per_hr;         // we calculate ml per seconds

        steps_per_sec = steps_per_ml * ml_per_sec; // we calculate steps per sec
        motor_start();
        
        delay = static_cast<uint32_t>(usec_per_sec / steps_per_sec);        // converst steps per second into microseconds
        set_delay_rate(delay);
    }

    /// @brief Checks volume-tracking mismatch and occlusion pressure,
    ///        raises/clears each alarm type independently — no cross-wipe
    void InfusionMode::checkAlarm(float rate) {

        // Volume Check
        if (!volume.cal(rate)) {
            int64_t now = k_uptime_get();
            if (now - last_alert_ms_ >= alarm_throttle_ms) {
                printk("[ALARM] Volume deviation exceeded 5%%\n"); 
                last_alert_ms_ = now;
            }
            alarm.notify(AlarmType::kVolume);
        } else {
            alarm.clear(AlarmType::kVolume);
        }

        // Occlusion check: pressure out of range -> alert, else clear only occlusion
        if (!occlu.isPressureNormal()) {
            printk("[ALARM] Occlusion detected - pressure out of range\n");
            alarm.notify(AlarmType::kOcclusion);
        } else {
            alarm.clear(AlarmType::kOcclusion);
        }
    }

    /// @brief Constant mode: rate never changes
    auto ConstantRateMode::computeTargetRate() -> float {
        return setrate;
    }

    /// @brief Ramp mode: increases rate step-wise every step_interval_ms_, until fin or volume runs out
        auto LinearRampMode::computeTargetRate() -> float {
        int64_t now = k_uptime_get();

        ///@brief if condition to increase current rate per minuite
        if ((now - last_step_ms_) >= step_interval_ms_) {
            current_lvl += incr;
            if (current_lvl > fin) { current_lvl = fin; }
            last_step_ms_ = now;
        }

        float infused = current_lvl * (ms_per_tick / ms_per_hr); //ticks per 10ms with current rate
        tot -= infused;

        /// @brief just to print Datas for every 1 sec
        if (now - last_ramp_print >= 1000) {   // print once per second
            printk("[RAMP] Rate:%d mL/hr | Remaining:%d.%02d mL\n",
               (int)current_lvl,
                (int)tot, (int)(fabsf(tot - (int)tot) * 100));

            printk("[DATA] time_ms:%d rate:%d\n", (int)now, (int)current_lvl);
            last_ramp_print = now;
        }

        if (tot <= 0.0F) {
            completed_ = true;
            motor_stop();
            printk(">> Infusion completed\n");
        }
        return current_lvl;
    }

    /// @brief Switches active infusion mode, resetting timing/volume state on both old and new mode
    InfusionMode* switch_mode(InfusionMode* current, InfusionMode* next) {
        if (current == next) { return current; }  // no-op if same mode selected

        motor_stop();
        current->started_ = false;                // Set the current started state off
        next->started_ = false;                   //Set the next started state off
        next->completed_ = false;                 // clear any stale completion flag
        next->volume.initial = k_uptime_get();    //Before Startinh another mode, we reset the volume level as initial, to calculate present mode data only.
        next->volume.expected = 0.0F;  
        next->volume.last_calc_ms = 0;
        return next;
    }