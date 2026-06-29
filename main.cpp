#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include "alarm.hpp"
#include "occlusion.hpp"
#include "volume.hpp"
#include "infusion.hpp"
#include "hardware.hpp"

static VolumeTracker volume;
static AlarmManager  alarm;
static OcclusionMonitor occlus;
float initial = 10.0F;
float fin = 100.0F;
float incr = (fin - initial) /5.0F; 
static LinearRampMode ramp(initial,incr,fin, 100.0F, volume, occlus, alarm);
static ConstantRateMode constant(100.0F, volume, occlus, alarm);
static InfusionMode     *active_mode = nullptr;
static led l;
static buz b;
bool running = false;
bool paused  = false;

static const struct device *uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

K_THREAD_STACK_DEFINE(thread1, 1024);
static struct k_thread my_thread;

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

void th_fn(void *arg1, void *arg2, void *arg3) {
    InfusionMode *mode = static_cast<InfusionMode *>(arg1);
    mode->volume.initial = k_uptime_get();
    while (true) {
        if (running && !paused) { mode->run(); }
        k_sleep(K_MSEC(10));
    }
}

int main() {
    char buf[8]       = {0};
    uint8_t idx       = 0U;
    char mode_buf[10] = {0};

    alarm.add(&l);
    alarm.add(&b);
    hardware_init();

    printk("Mode: CONSTANT or LINEAR?\n> ");
    read_line(mode_buf, sizeof(mode_buf));

    if (strcmp(mode_buf, "LINEAR") == 0) {
        active_mode = static_cast<InfusionMode*>(&ramp);
        printk("Linear Mode");
    } else {
        printk("Constant Mode");
        active_mode = static_cast<InfusionMode*>(&constant);
    }

    volume.initial = k_uptime_get();
    k_thread_create(&my_thread, thread1, K_THREAD_STACK_SIZEOF(thread1),
                    th_fn, active_mode, NULL, NULL, 5, 0, K_NO_WAIT);

    printk("=== Infusion Pump ===\nCmds: START|STOP|PAUSE|RESET\n> ");

    while (true) {
        uint8_t c;
        if (uart_poll_in(uart_dev, &c) == 0) {
            printk("%c", c);
            if (c == '\r' || c == '\n') {
                buf[idx] = '\0';
                printk("\n");

                if (strcmp(buf, "START") == 0) {
                    running = true; paused = false;
                    active_mode->started_ = false;
                    active_mode->volume.initial = k_uptime_get();
                    atomic_set(&tick_count, 0);  // add this
                    printk(">> Started\n");
                } else if (strcmp(buf, "STOP") == 0) {
                    running = false; paused = false;
                    motor_stop();
                    printk(">> Stopped\n");

                } else if (strcmp(buf, "PAUSE") == 0) {
                    paused = true;
                    motor_stop();
                    printk(">> Paused\n");

                } else if (strcmp(buf, "RESET") == 0) {
                    running = false; paused = false;
                    active_mode->started_ = false;
                    motor_stop();
                    if (active_mode == static_cast<InfusionMode*>(&ramp)) {
                        ramp.current_lvl   = ramp.initial;
                        ramp.tot           = ramp.total_volume;
                        ramp.last_step_ms_ = 0;
                    }
                    active_mode->volume.initial = k_uptime_get();
                    atomic_set(&tick_count, 0);
                    printk(">> Reset\n");

                } else { printk(">> Unknown\n"); }

                idx = 0U;
                printk("> ");

            } else if (idx < 7U) {
                buf[idx++] = static_cast<char>(c);
            }
        }
        k_sleep(K_MSEC(10));
    }
    return 0;
}                                                                                   