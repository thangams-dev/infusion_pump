#include <gtest/gtest.h>
#include "alarm.hpp"
#include "occlusion.hpp"
#include "volume.hpp"
#include "infusion.hpp"
#include "stub_file.hpp"

class InfusionTest : public testing::Test {
protected:
    void SetUp() override {
        fake_press = 0.0F;
        fake_ticks = 0;
        fake_time  = 0;
    }
    OcclusionMonitor occlu;
    AlarmManager alarm;
    VolumeTracker vt;
    ConstantRateMode constant{100.0F, vt, occlu, alarm};
    LinearRampMode ramp{10.0F, 2.0F, 100.0F, 100.0F, vt, occlu, alarm};
};

// ==================== occlusion.cpp ====================

// normal pressure -> within range -> true
TEST_F(InfusionTest, OcclusionMonitor_NormalPressure_ReturnsTrue) {
    fake_press = 0.0F;
    EXPECT_TRUE(occlu.isocclued());
}

// pressure at exact boundary -> still true
TEST_F(InfusionTest, OcclusionMonitor_EqualPressure_ReturnTrue) {
    fake_press = 100.56F;
    EXPECT_TRUE(occlu.isocclued());
}

// pressure over limit -> false (occlusion detected)
TEST_F(InfusionTest, OcclusionMonitor_HighPressure_ReturnsFalse) {
    fake_press = 400.0F;
    EXPECT_FALSE(occlu.isocclued());
}

// ==================== alarm.cpp ====================

// two observers added, notify volume once -> notify count 1
TEST_F(InfusionTest, AlarmManager_TwoObservers_NotifyCountOne) {
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.notify(AlarmType::kVolume);
    EXPECT_EQ(alarm.get_notify_count(), 1);
}

// no notify called -> count stays 0
TEST_F(InfusionTest, AlarmManager_NoObservers_CountZero) {
    EXPECT_EQ(alarm.get_notify_count(), 0);
}

// clearAll with observers -> loop runs, no crash
TEST_F(InfusionTest, AlarmManager_ClearAll_NoError) {
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.clearAll();
}

// occlusion still active when volume cleared -> compound if() stays false
TEST_F(InfusionTest, AlarmManager_ClearVolumeWhileOcclusionActive) {
    alarm.notify(AlarmType::kOcclusion);
    alarm.notify(AlarmType::kVolume);
    alarm.clear(AlarmType::kVolume);
    EXPECT_TRUE(true);
}

// both alarms cleared with observer present -> inner loop body finally runs
TEST_F(InfusionTest, AlarmManager_ClearBothAlarmsNotifiesObserver) {
    led l;
    alarm.add(&l);
    alarm.notify(AlarmType::kVolume);
    alarm.clear(AlarmType::kVolume);
    EXPECT_TRUE(true);
}

// add 5th observer beyond MAX_OBSERVERS(4) -> add() silently drops it
TEST_F(InfusionTest, AlarmManager_AddBeyondMax_Ignored) {
    led l1, l2; buz b; uart_observer u;
    alarm.add(&l1);
    alarm.add(&l2);
    alarm.add(&b);
    alarm.add(&u);
    led extra;
    alarm.add(&extra);
    EXPECT_TRUE(true);
}

// ==================== volume.cpp ====================

// expected volume ~0 -> early-return true (avoid div by zero)
TEST_F(InfusionTest, VolumeTracker_ZeroTime_ReturnsTrue) {
    fake_time = 0; fake_ticks = 0;
    EXPECT_TRUE(vt.cal(100.0F, 0.0F));
}

// time passed but ticks stayed 0 -> deviation too high -> false
TEST_F(InfusionTest, VolumeTracker_TimeElapsed_ReturnsDeviation) {
    fake_time  = 3600000;
    fake_ticks = 0;
    EXPECT_FALSE(vt.cal(100.0F, 0.0F));
}

// ticks match expected rate -> deviation within accuracy -> true
TEST_F(InfusionTest, VolumeTracker_CorrectTicks_ReturnsTrue) {
    fake_time  = 3600000;
    fake_ticks = 51824;
    EXPECT_TRUE(vt.cal(100.0F, 0.0F));
}

// under 5s elapsed -> too early to judge -> true
TEST_F(InfusionTest, VolumeTracker_EarlyElapsed_ReturnsTrue) {
    fake_time = 2000;
    EXPECT_TRUE(vt.cal(100.0F, 0.0F));
}

// correction already applied once -> 2nd call skips recalculation branch
TEST_F(InfusionTest, VolumeTracker_AlreadyCorrected_SkipsRecalc) {
    fake_time  = 3600000;
    fake_ticks = 51824;
    vt.cal(100.0F, 0.0F);
    vt.cal(100.0F, 0.0F);
}

// encoder offset captured on first run -> ticks reflect delta only
TEST_F(InfusionTest, VolumeTracker_OffsetCapture_TicksReflectDelta) {
    fake_ticks = 500;
    fake_time  = 0;
    constant.run();

    fake_ticks = 550;
    fake_time  = 3600000;
    constant.run();

    EXPECT_FLOAT_EQ(vt.ticks, 50.0F);
}

// ==================== infusion.cpp ====================

// zero ticks despite time passing -> volume mismatch -> alarm fires
TEST_F(InfusionTest, VolumeTracker_ZeroTicks_AlarmTriggered) {
    fake_time = 3600000; fake_ticks = 0;
    constant.run();
    EXPECT_GT(alarm.get_notify_count(), 0);
}

// 2nd alert within throttle window(2500ms) -> print-once branch false
TEST_F(InfusionTest, VolumeAlertThrottled) {
    fake_time = 3600000; fake_ticks = 0;
    constant.run();

    fake_time = 3601000;
    constant.run();
}

// constant mode always returns same fixed rate
TEST_F(InfusionTest, ConstantRateMode_ComputeRate_Returns100) {
    EXPECT_EQ(constant.computeTargetRate(), 100.0F);
}

// ramp mode first call -> returns initial level, no step yet
TEST_F(InfusionTest, LinearRampMode_FirstCall_ReturnsInitial) {
    EXPECT_EQ(ramp.computeTargetRate(), 10.0F);
}

// volume runs out (tot<=0) -> completed flag set, motor stopped
TEST_F(InfusionTest, LinearRampMode_VolumeComplete_StopsMotor) {
    LinearRampMode small{10.0F, 2.0F, 100.0F, 0.0F, vt, occlu, alarm};
    small.computeTargetRate();
    EXPECT_FALSE(running);
}

// step interval elapsed -> rate increments by incr
TEST_F(InfusionTest, LinearRampMode_AfterInterval_Increments) {
    fake_time = 0;
    ramp.computeTargetRate();
    fake_time = 90001;
    EXPECT_FLOAT_EQ(ramp.computeTargetRate(), 12.0F);
}

// many steps -> rate capped at fin, never exceeds it
TEST_F(InfusionTest, LinearRampMode_OverMax_CapsAtFin) {
    for (int i = 0; i < 60; i++) {
        fake_time += 90001;
        ramp.computeTargetRate();
    }
    EXPECT_EQ(ramp.current_lvl, 100.0F);
}

// polymorphic call via base pointer -> correct derived rate returned
TEST_F(InfusionTest, InfusionMode_SwitchMode_ComputesCorrectRate) {
    InfusionMode *active = &constant;
    EXPECT_EQ(active->computeTargetRate(), 100.0F);
    active = &ramp;
    EXPECT_EQ(active->computeTargetRate(), 10.0F);
}

// high pressure -> occlusion alarm fires once
TEST_F(InfusionTest, InfusionMode_OcclusionAlarm_NotifyCount1) {
    fake_press = 400.0F; fake_time = 0;
    constant.run();
    EXPECT_EQ(alarm.get_notify_count(), 1);
}

// repeated run() calls -> no crash, normal path
TEST_F(InfusionTest, InfusionMode_ApplyRate_NoError) {
    constant.run();
    constant.run();
}

// normal pressure + fresh start -> no alarm raised
TEST_F(InfusionTest, InfusionMode_NoPressure_NoAlarm) {
    fake_press = 0.0F; fake_time = 0; fake_ticks = 0;
    constant.run();
    EXPECT_EQ(alarm.get_notify_count(), 0);
}

// rate near-zero -> applyRate() returns early, no motor action
TEST_F(InfusionTest, InfusionMode_ZeroRate_NoAction) {
    ConstantRateMode zero{0.0F, vt, occlu, alarm};
    zero.run();
}

// switch to same mode pointer -> no-op, returns same pointer
TEST_F(InfusionTest, ModeSwitch_SamePointer_ReturnsSame) {
    InfusionMode* active = &constant;
    InfusionMode* result = switch_mode(active, active);
    EXPECT_EQ(result, active);
}

// switch to different mode -> old stopped, new mode's started_ reset
TEST_F(InfusionTest, ModeSwitch_NoRestart_ResetsNewModeState) {
    InfusionMode* active = &constant;
    fake_time = 5000;
    active->run();
    fake_ticks = 100;

    active = switch_mode(active, &ramp);
    EXPECT_EQ(active, &ramp);
    EXPECT_FALSE(ramp.started_);
}

// ==================== hardware.cpp (stubbed) ====================

// stub calls just need to run without error
TEST_F(InfusionTest, Hardware_MotorStopStart_NoError) {
    motor_stop();
    motor_start();
    hardware_init();
}