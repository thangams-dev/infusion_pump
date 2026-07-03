#include <gtest/gtest.h>
#include "alarm.hpp"
#include "occlusion.hpp"
#include "volume.hpp"
#include "infusion.hpp"
#include "mock_hardware.hpp"

class InfusionTest : public testing::Test {
protected:
    void SetUp() override {
        fake_press = 0.0F;
        fake_ticks = 0;
        fake_time  = 0;
        tick_count = 0;
    }
    OcclusionMonitor occlu;
    AlarmManager alarm;
    VolumeTracker vt;
    ConstantRateMode constant{100.0F, vt, occlu, alarm};
    LinearRampMode ramp{10.0F, 2.0F, 100.0F, 100.0F, vt, occlu, alarm};
};

// ─── occlusion.cpp ──────────────────

TEST_F(InfusionTest, OcclusionMonitor_NormalPressure_ReturnsTrue) {
    fake_press = 0.0F;
    EXPECT_EQ(occlu.isocclued(), true);
}

TEST_F(InfusionTest, OcclusionMonitor_HighPressure_ReturnsFalse) {
    fake_press = 400.0F;
    EXPECT_EQ(occlu.isocclued(), false);
}

// ─── alarm.cpp ──────────────────────

TEST_F(InfusionTest, AlarmManager_TwoObservers_NotifyCountOne) {
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.notify();
    EXPECT_EQ(alarm.noti_count, 1);
}

TEST_F(InfusionTest, AlarmManager_NoObservers_CountZero) {
    EXPECT_EQ(alarm.noti_count, 0);
}

TEST_F(InfusionTest, AlarmManager_ClearAll_NoError) {
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.clearAll();
}

// ─── volume.cpp ─────────────────────

TEST_F(InfusionTest, VolumeTracker_ZeroTime_ReturnsTrue) {
    fake_time = 0; fake_ticks = 0;
    EXPECT_EQ(vt.cal(100.0F, 0.0F), true);
}

TEST_F(InfusionTest, VolumeTracker_TimeElapsed_ReturnsDeviation) {
    fake_time  = 3600000;
    fake_ticks = 0;
    EXPECT_EQ(vt.cal(100.0F, 0.0F), false);
}

TEST_F(InfusionTest, VolumeTracker_CorrectTicks_ReturnsTrue) {
    fake_time  = 3600000;
    fake_ticks = 51824;
    EXPECT_EQ(vt.cal(100.0F, 0.0F), true);
}

// ─── infusion.cpp ───────────────────

TEST_F(InfusionTest, VolumeTracker_ZeroTicks_AlarmTriggered) {
    fake_time = 3600000; fake_ticks = 0;
    constant.run();
    EXPECT_GT(alarm.noti_count, 0);
}

TEST_F(InfusionTest, ConstantRateMode_ComputeRate_Returns100) {
    EXPECT_EQ(constant.computeTargetRate(), 100.0F);
}

TEST_F(InfusionTest, LinearRampMode_FirstCall_ReturnsInitial) {
    EXPECT_EQ(ramp.computeTargetRate(), 10.0F);
}
TEST_F(InfusionTest, LinearRampMode_VolumeComplete_StopsMotor) {
    LinearRampMode small{10.0F, 2.0F, 100.0F, 0.0F, vt, occlu, alarm};
    small.computeTargetRate();
    EXPECT_FALSE(running);
}
TEST_F(InfusionTest, LinearRampMode_AfterInterval_Increments) {
    fake_time = 0;
    ramp.computeTargetRate();
    fake_time = 90001;
    float rate = ramp.computeTargetRate();
    EXPECT_FLOAT_EQ(rate, 12.0F);
}

TEST_F(InfusionTest, LinearRampMode_OverMax_CapsAtFin) {
    for (int i = 0; i < 60; i++) {
        fake_time += 90001;
        ramp.computeTargetRate();
    }
    EXPECT_EQ(ramp.current_lvl, 100.0F);
}

TEST_F(InfusionTest, InfusionMode_SwitchMode_ComputesCorrectRate) {
    InfusionMode *active = &constant;
    EXPECT_EQ(active->computeTargetRate(), 100.0F);
    active = &ramp;
    EXPECT_EQ(active->computeTargetRate(), 10.0F);
}

TEST_F(InfusionTest, InfusionMode_OcclusionAlarm_NotifyCount1) {
    fake_press = 400.0F; fake_time = 0;
    constant.run();
    EXPECT_EQ(alarm.noti_count, 1);
}

TEST_F(InfusionTest, InfusionMode_ApplyRate_NoError) {
    constant.run();
    constant.run();
}

TEST_F(InfusionTest, InfusionMode_NoPressure_NoAlarm) {
    fake_press = 0.0F; fake_time = 0; fake_ticks = 0;
    constant.run();
    EXPECT_EQ(alarm.noti_count, 0);
}

TEST_F(InfusionTest, InfusionMode_ZeroRate_NoAction) {
    ConstantRateMode zero{0.0F, vt, occlu, alarm};
    zero.run();
}

// ─── mock_hardware.cpp ──────────────

TEST_F(InfusionTest, Hardware_GetPumpMode_ReturnsConstant) {
    EXPECT_EQ(get_pump_mode(), PumpMode::Constant_mode);
}

TEST_F(InfusionTest, Hardware_MotorStopEnable_NoError) {
    motor_stop();
    motor_enable();
    hardware_init();
    atomic_inc(&tick_count);
}

// ─── infusion.cpp: switch_mode() ────

TEST_F(InfusionTest, ModeSwitch_NoRestart_ResetsNewModeState) {
    InfusionMode* active = &constant;
    fake_time = 5000;
    active->run();
    fake_ticks = 100;

    active = switch_mode(active, &ramp);
    EXPECT_EQ(active, &ramp);
    EXPECT_FALSE(ramp.started_);
}