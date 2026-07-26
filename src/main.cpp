#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "alarm.hpp"
#include "hardware.hpp"
#include "infusion.hpp"
#include "occlusion.hpp"
#include "volume.hpp"

static VolumeTracker volume;
static AlarmManager  alarm;
static OcclusionMonitor occlus;
static led l;
static buz b;

static float initial = 10.0F;   ///< ramp mode starting rate, mL/hr
static float fin     = 190.0F;  ///< ramp mode max rate, mL/hr
static float incr    = 3.0F;    ///< ramp mode rate increase per step

static LinearRampMode ramp(initial, incr, fin, 100.0F, volume, occlus, alarm);
static ConstantRateMode constant(100.0F, volume, occlus, alarm);
static InfusionMode *active_mode = nullptr;  ///< active mode, base-class pointer

static bool running = false;   ///< pump running state
static bool paused  = false;   ///< pump paused state

// Guards running/paused/active_mode and any mode-object fields (ramp/constant/volume)
// touched from both the main thread (UART commands) and th_fn (control loop).
static K_MUTEX_DEFINE(state_mutex);

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

K_THREAD_STACK_DEFINE(thread1, 1024);
static struct k_thread my_thread;

/// @brief Blocking line read over UART, used only during startup mode prompt.
static void read_line(char *buf, uint8_t max_len) {
    uint8_t idx = 0U;
    uint8_t c;
    while (idx < (max_len - 1U)) {
        if (uart_poll_in(uart_dev, &c) == 0) {
            if (c == '\r' || c == '\n') { break; }
            buf[idx++] = static_cast<char>(c);
        }
        k_sleep(K_MSEC(10));
    }
    buf[idx] = '\0';
}

/// @brief Control-loop thread: drives the active infusion mode every 10ms.
void th_fn(void *arg1, void *arg2, void *arg3) {
    while (true) {
        k_mutex_lock(&state_mutex, K_FOREVER);
        if (running && !paused && active_mode != nullptr) {
            active_mode->run();
            if (active_mode->completed_) {
                running = false;
                active_mode->completed_ = false;
            }
        }
        k_mutex_unlock(&state_mutex);
        k_sleep(K_MSEC(10));
    }
}

/// @brief Entry point: initializes hardware, prompts mode, starts control thread, runs UART command loop.
int main() {
    char buf[32]      = {0};
    uint8_t idx       = 0U;
    char mode_buf[10] = {0};

    alarm.add(&l);
    alarm.add(&b);
    hardware_init();

    printk("Mode: CONSTANT or LINEAR?\n> ");
    read_line(mode_buf, sizeof(mode_buf));

    if (strcmp(mode_buf, "LINEAR") == 0) {
        active_mode = static_cast<InfusionMode*>(&ramp);
        printk("Linear Mode\n");
        printk("Default ramp: 10 to 190 ml/hr, +3/min, Volume: 100 ml\n");
        printk("Use SET_RAMP <start> <end> <incr> to customize, or START to use default\n");
    } else {
        active_mode = static_cast<InfusionMode*>(&constant);
        printk("Constant Mode\n");
        printk("Default rate: 100 ml/hr\n");
        printk("Use SET_RATE <value> to customize, or START to use default\n");
    }

    volume.initial = k_uptime_get();
    k_thread_create(&my_thread, thread1, K_THREAD_STACK_SIZEOF(thread1),
                th_fn, nullptr, nullptr, nullptr, 5, 0, K_NO_WAIT);

    printk("\n   HCE Infusion Pump Platform v1.0\nCmds: START|STOP|PAUSE|RESET|MODE|SET_RATE|SET_RAMP\n> ");

    while (true) {
        uint8_t c;
        if (uart_poll_in(uart_dev, &c) == 0) {
            printk("%c", c);
            if (c == '\r' || c == '\n') {
                buf[idx] = '\0';
                printk("\n");

                if (strcmp(buf, "START") == 0) {
                    k_mutex_lock(&state_mutex, K_FOREVER);
                    if (!paused) {
                        active_mode->started_ = false;
                        active_mode->volume.initial = k_uptime_get();
                    }
                    active_mode->volume.last_calc_ms = k_uptime_get();
                    running = true; paused = false;
                    k_mutex_unlock(&state_mutex);
                    printk(">> Started\n");

                } else if (strcmp(buf, "STOP") == 0) {
                    k_mutex_lock(&state_mutex, K_FOREVER);
                    running = false; paused = false;
                    active_mode->volume.expected = 0.0F;
                    active_mode->volume.last_calc_ms = 0;
                    active_mode->started_ = false;
                    k_mutex_unlock(&state_mutex);
                    motor_stop();
                    printk(">> Stopped\n");

                } else if (strcmp(buf, "PAUSE") == 0) {
                    k_mutex_lock(&state_mutex, K_FOREVER);
                    paused = true;
                    active_mode->volume.last_calc_ms = k_uptime_get();
                    k_mutex_unlock(&state_mutex);
                    motor_stop();
                    printk(">> Paused\n");

                } else if (strcmp(buf, "RESET") == 0) {
                    k_mutex_lock(&state_mutex, K_FOREVER);
                    running = false; paused = false;
                    active_mode->started_ = false;
                    if (active_mode == static_cast<InfusionMode*>(&ramp)) {
                        ramp.current_lvl   = ramp.initial;
                        ramp.tot           = ramp.total_volume;
                        ramp.last_step_ms_ = 0;
                    }
                    active_mode->volume.initial = k_uptime_get();
                    active_mode->volume.expected = 0.0F;
                    active_mode->volume.last_calc_ms = 0;
                    k_mutex_unlock(&state_mutex);
                    alarm.clearAll();
                    motor_stop();
                    printk(">> Reset\n");

                } else if (strncmp(buf, "SET_RATE ", 9) == 0) {
                    float new_rate = atof(buf + 9);
                    if (new_rate <= 0.0F || new_rate > 500.0F) {
                        printk(">> Invalid rate (must be 1-500 mL/hr)\n");
                    } else {
                        k_mutex_lock(&state_mutex, K_FOREVER);
                        if (active_mode == static_cast<InfusionMode*>(&constant)) {
                            constant.setrate = new_rate;
                            k_mutex_unlock(&state_mutex);
                            printk(">> Rate set to %d mL/hr\n", (int)new_rate);
                        } else {
                            k_mutex_unlock(&state_mutex);
                            printk(">> SET_RATE only valid in CONSTANT mode\n");
                        }
                    }

                } else if (strncmp(buf, "SET_RAMP ", 9) == 0) {
                    float new_initial, new_fin, new_incr;
                    int parsed = sscanf(buf + 9, "%f %f %f", &new_initial, &new_fin, &new_incr);

                    if (parsed != 3 || new_initial <= 0.0F || new_fin <= new_initial ||
                        new_fin > 500.0F || new_incr <= 0.0F) {
                        printk(">> Invalid. Usage: SET_RAMP <initial> <final> <increment>\n");
                    } else {
                        k_mutex_lock(&state_mutex, K_FOREVER);
                        ramp.initial     = new_initial;
                        ramp.fin         = new_fin;
                        ramp.incr        = new_incr;
                        ramp.current_lvl = new_initial;
                        k_mutex_unlock(&state_mutex);
                        printk(">> Ramp set: %d -> %d mL/hr, +%d/step\n",
                               (int)new_initial, (int)new_fin, (int)new_incr);
                    }

                } else if (strcmp(buf, "MODE") == 0) {
                    k_mutex_lock(&state_mutex, K_FOREVER);
                    InfusionMode *next = (active_mode == static_cast<InfusionMode*>(&constant))
                                        ? static_cast<InfusionMode*>(&ramp)
                                        : static_cast<InfusionMode*>(&constant);
                    active_mode = switch_mode(active_mode, next);
                    k_mutex_unlock(&state_mutex);
                    printk(">> Mode switched\n");

                } else {
                    printk(">> Unknown\n");
                }

                idx = 0U;
                printk("> ");

            } else if (idx < 31U) {
                buf[idx++] = static_cast<char>(c);
            }
        }
        k_sleep(K_MSEC(10));
    }
    return 0;
}