#include <zephyr/kernel.h>
#include "alarm.hpp"
#include "occlusion.hpp"
#include "volume.hpp"
#include "infusion.hpp"
#include "hardware.hpp"

static VolumeTracker volume(100.0f, 0, 0.005f);
static AlarmManager alarm;
static OcclusionMonitor occlus;
static ConstantRateMode constant(100.0f, volume, occlus, alarm);
static LinearRampMode ramp(10.0f, 2.0f, 100.0f, volume, occlus, alarm);
static InfusionMode *active_mode = &constant;
static led l;
static buz b;

K_THREAD_STACK_DEFINE(thread1, 1024);
static struct k_thread my_thread;

void th_fn(void *arg1, void *arg2, void *arg3) {
    InfusionMode *mode = static_cast<InfusionMode*>(arg1);
    while(1) {
        mode->run();
        k_sleep(K_MSEC(10));
    }
}

int main() {
    alarm.add(&l);
    alarm.add(&b);
    hardware_init();
    active_mode = get_pump_mode() == PumpMode::Ramp_mode ? static_cast<InfusionMode*>(&ramp) : static_cast<InfusionMode*>(&constant);
    k_thread_create(&my_thread, thread1, K_THREAD_STACK_SIZEOF(thread1), th_fn, active_mode, NULL, NULL, 5, 0, K_NO_WAIT);
    while(1) {
        k_sleep(K_MSEC(100));
    }
    return 0;
}