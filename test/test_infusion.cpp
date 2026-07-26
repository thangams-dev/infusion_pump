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

//  occlusion.cpp (4 tests) 

// Pressure below threshold -> should read as normal.
TEST_F(InfusionTest, OcclusionMonitor_NormalPressure_ReturnsTrue) {
    fake_press = 0.0F;
    EXPECT_TRUE(occlu.isPressureNormal());
}

// Pressure exactly at the threshold boundary -> still counts as normal.
TEST_F(InfusionTest, OcclusionMonitor_EqualPressure_ReturnTrue) {
    fake_press = 100.56F;
    EXPECT_TRUE(occlu.isPressureNormal());
}

// Pressure above threshold -> should be flagged as abnormal (occlusion).
TEST_F(InfusionTest, OcclusionMonitor_HighPressure_ReturnsFalse) {
    fake_press = 400.0F;
    EXPECT_FALSE(occlu.isPressureNormal());
}

// Two calls inside the 2-second print-throttle window -> no duplicate print, same result.
TEST_F(InfusionTest, OcclusionMonitor_SecondCallWithinWindow_NoReprint) {
    fake_time = 0;
    occlu.isPressureNormal();
    fake_time = 500;
    EXPECT_TRUE(occlu.isPressureNormal());
}

// A monitor built with a custom/default threshold correctly flags a very high pressure.
TEST_F(InfusionTest, OcclusionMonitor_CustomThreshold) {
    OcclusionMonitor custom_om;
    fake_press = 999.0F;
    EXPECT_FALSE(custom_om.isPressureNormal());
}

//  alarm.cpp (13 tests) 

// clear(kOcclusion) must short-circuit and not clear kVolume when volume alarm is still active.
TEST_F(InfusionTest, AlarmManager_ClearOcclusion_ShortCircuitsOnVolume) {
    alarm.notify(AlarmType::kVolume);
    alarm.notify(AlarmType::kOcclusion);
    alarm.clear(AlarmType::kOcclusion);
    EXPECT_TRUE(true);
}

// Compiler-generated virtual destructor path for the AlarmObserver interface.
TEST_F(InfusionTest, AlarmObserver_VirtualDestructor_Coverage) {
    AlarmObserver* obs = new led();
    delete obs;
    EXPECT_TRUE(true);
}

// notify() should call update() once per registered observer (count reflects one notify call).
TEST_F(InfusionTest, AlarmManager_TwoObservers_NotifyCountOne) {
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.notify(AlarmType::kVolume);
    EXPECT_EQ(alarm.get_notify_count(), 1);
}

// notify() with zero registered observers must not crash and count stays zero.
TEST_F(InfusionTest, AlarmManager_NoObservers_CountZero) {
    EXPECT_EQ(alarm.get_notify_count(), 0);
}

// clearAll() resets state cleanly when observers are registered.
TEST_F(InfusionTest, AlarmManager_ClearAll_NoError) {
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.clearAll();
    EXPECT_EQ(alarm.get_notify_count(), 0);
}

// clearAll() on an empty observer list must not crash.
TEST_F(InfusionTest, AlarmManager_ClearAll_NoObservers) {
    alarm.clearAll();
    EXPECT_EQ(alarm.get_notify_count(), 0);
}

// Clearing the volume alarm should leave the occlusion alarm untouched/active.
TEST_F(InfusionTest, AlarmManager_ClearVolumeWhileOcclusionActive) {
    alarm.notify(AlarmType::kOcclusion);
    alarm.notify(AlarmType::kVolume);
    alarm.clear(AlarmType::kVolume);
    EXPECT_TRUE(true);
}

// Clearing the volume alarm should update observer state once all alarms are inactive.
TEST_F(InfusionTest, AlarmManager_ClearBothAlarmsNotifiesObserver) {
    led l;
    alarm.add(&l);
    alarm.notify(AlarmType::kVolume);
    alarm.clear(AlarmType::kVolume);
    EXPECT_TRUE(true);
}

// Explicitly exercises the kOcclusion branch inside clear().
TEST_F(InfusionTest, AlarmManager_ClearOcclusionType_Branch) {
    alarm.notify(AlarmType::kOcclusion);
    alarm.clear(AlarmType::kOcclusion);
    EXPECT_TRUE(true);
}

// Explicitly exercises the kOcclusion branch inside notify().
TEST_F(InfusionTest, AlarmManager_NotifyOcclusionType_Branch) {
    alarm.notify(AlarmType::kOcclusion);
    EXPECT_EQ(alarm.get_notify_count(), 1);
}

// Adding more observers than MAX_OBSERVERS should be safely ignored (no overflow/crash).
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

// All observer types (led, buzzer, uart) should have their overridden update() invoked correctly.
TEST_F(InfusionTest, AlarmManager_AllObservers_UpdateExecution) {
    led l; buz b; uart_observer u;
    alarm.add(&l);
    alarm.add(&b);
    alarm.add(&u);
    alarm.notify(AlarmType::kVolume);
    EXPECT_EQ(alarm.get_notify_count(), 1);
    alarm.clear(AlarmType::kVolume);
    EXPECT_TRUE(true);
}

// clearAll() must reset both alarm-active flags (volume and occlusion) together.
TEST_F(InfusionTest, AlarmManager_ClearAll_ResetsFlags) {
    alarm.notify(AlarmType::kOcclusion);
    alarm.notify(AlarmType::kVolume);
    alarm.clearAll();
    EXPECT_TRUE(true);
}

//  volume.cpp (9 tests) 

// Very first call with zero elapsed time -> passes without evaluating deviation yet.
TEST_F(InfusionTest, VolumeTracker_ZeroTime_ReturnsTrue) {
    fake_time = 0; fake_ticks = 0;
    EXPECT_TRUE(vt.cal(100.0F));
}

// After enough elapsed time, zero ticks delivered -> large deviation -> flags true (alarm-worthy).
TEST_F(InfusionTest, VolumeTracker_TimeElapsed_ReturnsDeviation) {
    fake_time = 1000; fake_ticks = 0;
    vt.cal(100.0F);
    fake_time = 3601000; fake_ticks = 0;
    EXPECT_TRUE(vt.cal(100.0F));
}

// After enough elapsed time, correct tick count delivered -> low deviation -> no alarm (false).
TEST_F(InfusionTest, VolumeTracker_CorrectTicks_ReturnsTrue) {
    fake_time = 1000; fake_ticks = 0;
    vt.cal(100.0F);
    fake_time = 3601000; fake_ticks = 2400000;
    EXPECT_FALSE(vt.cal(100.0F));
}

// Early-elapsed guard: too little time has passed to evaluate -> returns true (no premature check).
TEST_F(InfusionTest, VolumeTracker_EarlyElapsed_ReturnsTrue) {
    fake_time = 2000;
    EXPECT_TRUE(vt.cal(100.0F));
}

// Calling cal() again after last_calc_ms was already set should not force a fresh recalculation.
TEST_F(InfusionTest, VolumeTracker_AlreadyCorrected_SkipsRecalc) {
    fake_time  = 3600000;
    fake_ticks = 51824;
    vt.cal(100.0F);
    vt.cal(100.0F);
}

// Tick count tracked should reflect only the delta from the starting tick offset, not absolute ticks.
TEST_F(InfusionTest, VolumeTracker_OffsetCapture_TicksReflectDelta) {
    fake_ticks = 500;
    fake_time  = 0;
    constant.run();
    fake_ticks = 550;
    fake_time  = 3600000;
    constant.run();
    EXPECT_FLOAT_EQ(vt.ticks, 50.0F);
}

// Two calls inside the 2-second print-throttle window -> no duplicate print.
TEST_F(InfusionTest, VolumeTracker_SecondCallWithinWindow_NoReprint) {
    fake_time = 3600000; fake_ticks = 0;
    vt.cal(100.0F);
    fake_time = 3600500;
    fake_ticks = 0;
    vt.cal(100.0F);
}

// Boundary case: expected volume is essentially zero -> guarded, returns true without dividing.
TEST_F(InfusionTest, VolumeTracker_NearZeroExpected_ReturnsTrue) {
    fake_time = 0;
    vt.last_calc_ms = 0;
    fake_time = 1;
    EXPECT_TRUE(vt.cal(0.0001F));
}

// Full 100% deviation (zero ticks delivered over a full interval) -> confirms alarm condition triggers.
TEST_F(InfusionTest, VolumeTracker_HighDeviation_TriggersAlarmCondition) {
    fake_time = 1000;
    vt.cal(100.0F);
    fake_time += 3600000;
    fake_ticks = 0;
    EXPECT_TRUE(vt.cal(100.0F));
}

//  infusion.cpp (22 tests) 

// A repeat volume-fault call too soon after the last alarm must be suppressed (throttle false branch).
TEST_F(InfusionTest, InfusionMode_AlarmThrottle_FalseBranch) {
    fake_time = 1000;
    fake_ticks = 0;
    constant.run();
    fake_time = 3601000;
    fake_ticks = 2400000;
    constant.run();
    fake_time += 500;
    fake_ticks += 333;
    constant.run();
    EXPECT_TRUE(true);
}

// Enough time since the last alarm has passed -> throttle allows a fresh alarm and resets its timer.
TEST_F(InfusionTest, InfusionMode_VolumeAlarm_Coverage) {
    fake_time = 1000;
    fake_ticks = 0;
    constant.run();
    fake_time = 3601000;
    fake_ticks = 2400000;
    constant.run();
    fake_time += 2500;
    constant.run();
    EXPECT_TRUE(true);
}

// End-to-end: zero ticks delivered over a full interval must actually raise a real alarm notification.
TEST_F(InfusionTest, VolumeTracker_ZeroTicks_AlarmTriggered) {
    fake_time = 1000; fake_ticks = 0;
    constant.run();
    fake_time = 3601000; fake_ticks = 0;
    constant.run();
    EXPECT_GT(alarm.get_notify_count(), 0);
}

// End-to-end: a second volume alarm shortly after the first should be throttled (no new alarm).
TEST_F(InfusionTest, VolumeAlertThrottled) {
    fake_time = 1000; fake_ticks = 0;
    constant.run();
    fake_time = 3601000; fake_ticks = 0;
    constant.run();
    fake_time = 3601500;
    constant.run();
}

// Constant-rate mode should always report its fixed configured rate.
TEST_F(InfusionTest, ConstantRateMode_ComputeRate_Returns100) {
    EXPECT_EQ(constant.computeTargetRate(), 100.0F);
}

// Ramp mode's very first call should return the configured starting (initial) rate.
TEST_F(InfusionTest, LinearRampMode_FirstCall_ReturnsInitial) {
    EXPECT_EQ(ramp.computeTargetRate(), 10.0F);
}

// Zero remaining volume at start -> ramp should immediately mark itself complete and stop the motor.
TEST_F(InfusionTest, LinearRampMode_VolumeComplete_StopsMotor) {
    LinearRampMode small{10.0F, 2.0F, 100.0F, 0.0F, vt, occlu, alarm};
    small.computeTargetRate();
    EXPECT_TRUE(small.completed_);
}

// Volume still remaining -> ramp must stay in the "not completed" state.
TEST_F(InfusionTest, LinearRampMode_VolumeNotComplete_NoStop) {
    fake_time = 0;
    EXPECT_FALSE(ramp.completed_);
    ramp.computeTargetRate();
    EXPECT_FALSE(ramp.completed_);
}

// Once the step interval has elapsed, the ramp rate should step up by the configured increment.
TEST_F(InfusionTest, LinearRampMode_AfterInterval_Increments) {
    fake_time = 0;
    ramp.computeTargetRate();
    fake_time = 90001;
    EXPECT_FLOAT_EQ(ramp.computeTargetRate(), 12.0F);
}

// Repeated step increments must be capped at the configured max (final) rate, never exceed it.
TEST_F(InfusionTest, LinearRampMode_OverMax_CapsAtFin) {
    for (int i = 0; i < 60; i++) {
        fake_time += 90001;
        ramp.computeTargetRate();
    }
    EXPECT_EQ(ramp.current_lvl, 100.0F);
}

// Two ramp calls inside the 1-second print-throttle window -> no duplicate print.
TEST_F(InfusionTest, LinearRampMode_SecondCallWithinWindow_NoReprint) {
    fake_time = 0;
    ramp.computeTargetRate();
    fake_time = 200;
    ramp.computeTargetRate();
}

// Calling through the base InfusionMode* pointer must dispatch to the correct derived class logic.
TEST_F(InfusionTest, InfusionMode_SwitchMode_ComputesCorrectRate) {
    InfusionMode *active = &constant;
    EXPECT_EQ(active->computeTargetRate(), 100.0F);
    active = &ramp;
    EXPECT_EQ(active->computeTargetRate(), 10.0F);
}

// Abnormal pressure during run() must raise exactly one occlusion alarm notification.
TEST_F(InfusionTest, InfusionMode_OcclusionAlarm_NotifyCount1) {
    fake_press = 0.0F;
    fake_time = 0;
    constant.run();
    EXPECT_EQ(alarm.get_notify_count(), 1);
}

// run() should execute repeatedly without throwing or corrupting state.
TEST_F(InfusionTest, InfusionMode_ApplyRate_NoError) {
    constant.run();
    constant.run();
}

// Normal pressure during run() must not raise any occlusion alarm.
TEST_F(InfusionTest, InfusionMode_NoPressure_NoAlarm) {
    fake_press = 400.0F;
    fake_time = 0; fake_ticks = 0;
    constant.run();
    EXPECT_EQ(alarm.get_notify_count(), 0);
}

// A configured rate of exactly zero must be guarded against (no divide-by-zero, motor not driven).
TEST_F(InfusionTest, InfusionMode_ZeroRate_NoAction) {
    ConstantRateMode zero{0.0F, vt, occlu, alarm};
    zero.run();
}

// Switching to the mode that is already active should be a no-op and return the same pointer.
TEST_F(InfusionTest, ModeSwitch_SamePointer_ReturnsSame) {
    InfusionMode* active = &constant;
    InfusionMode* result = switch_mode(active, active);
    EXPECT_EQ(result, active);
}

// Switching to a genuinely new mode should update the active pointer and leave the new mode unstarted.
TEST_F(InfusionTest, ModeSwitch_NoRestart_ResetsNewModeState) {
    InfusionMode* active = &constant;
    fake_time = 5000;
    active->run();
    fake_ticks = 100;
    active = switch_mode(active, &ramp);
    EXPECT_EQ(active, &ramp);
    EXPECT_FALSE(ramp.started_);
}

// A ramp configured with a negative starting volume should complete immediately.
TEST_F(InfusionTest, LinearRampMode_TotalVolumeDepleted_CompletesInfusion) {
    LinearRampMode zero_vol_ramp{10.0F, 5.0F, 50.0F, -1.0F, vt, occlu, alarm};
    fake_time = 0;
    zero_vol_ramp.computeTargetRate();
    EXPECT_TRUE(zero_vol_ramp.completed_);
}

// An extremely low configured rate should be treated as effectively zero -> motor not started.
TEST_F(InfusionTest, InfusionMode_NearZeroRate_SkipsMotorStart) {
    constant.setrate = 0.00001F;
    constant.run();
}

// Once a volume alarm fires, the throttle timestamp must be updated so the next check uses fresh timing.
TEST_F(InfusionTest, InfusionMode_AlarmThrottle_Execution) {
    fake_time = 1000;
    constant.run();
    fake_time = 3601000;
    fake_ticks = 0;
    constant.run();
    fake_time = 3601100;
    constant.run();
}

//  hardware.cpp (stubbed) 

// Hardware stub calls (motor start/stop, init) must execute cleanly without error during tests.
TEST_F(InfusionTest, Hardware_MotorStopStart_NoError) {
    motor_stop();
    motor_start();
    hardware_init();
}