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
        fake_time = 0;
    }
    OcclusionMonitor occlu;
    AlarmManager alarm;
    VolumeTracker vt{100.0F, 0, 0.005F};
    ConstantRateMode constant{100.0F, vt, occlu, alarm};
    LinearRampMode ramp{10.0F, 2.0F, 100.0F, vt, occlu, alarm};
};
TEST_F(InfusionTest, OcclusionMonitor_NormalPressure_ReturnsTrue){
        printf("fake_press = %f\n", fake_press);
    EXPECT_EQ(occlu.isocclued(), true);
}
TEST_F(InfusionTest, AlarmManager_TwoObservers_NotifyCountOne){
    led l; buz b;
    alarm.add(&l); alarm.add(&b);
    alarm.notify();
    EXPECT_EQ(alarm.noti_count, 1);
}
TEST_F(InfusionTest, VolumeTracker_TimeElapsed_ReturnsDeviation){
    fake_time = 2000;
    EXPECT_EQ(vt.cal(), false);
}
TEST_F(InfusionTest, ConstantRateMode_ComputeRate_Returns100){
    EXPECT_EQ(constant.computeTargetRate(), 100.0F);
}
TEST_F(InfusionTest, LinearRampMode_FirstCall_Returns12){
    EXPECT_EQ(ramp.computeTargetRate(), 12.0F);
}
TEST_F(InfusionTest, LinearRampMode_FourCalls_Returns18){
    ramp.computeTargetRate();
    ramp.computeTargetRate();
    ramp.computeTargetRate();
    EXPECT_EQ(ramp.computeTargetRate(), 18.0F);
}
TEST_F(InfusionTest, OcclusionMonitor_HighPressure_ReturnsFalse){
    fake_press = 400.0F;
    EXPECT_EQ(occlu.isocclued(), false);
}
TEST_F(InfusionTest, VolumeTracker_CorrectTicks_ReturnsTrue){
    fake_time = 3600000;
    fake_ticks = 20000;
    EXPECT_EQ(vt.cal(), true);
}
TEST_F(InfusionTest, InfusionMode_SwitchMode_ComputesCorrectRate){
    InfusionMode *active = &constant;
    EXPECT_EQ(active->computeTargetRate(), 100.0F);
    active = &ramp;
    EXPECT_EQ(active->computeTargetRate(), 12.0F);
}
TEST_F(InfusionTest, LinearRampMode_OverMax_CapsAtFin){
    for(int i = 0; i < 50; i++) ramp.computeTargetRate();
    EXPECT_EQ(ramp.computeTargetRate(), 100.0F);
}
TEST_F(InfusionTest, AlarmManager_NoObservers_CountZero){
    EXPECT_EQ(alarm.noti_count, 0);
}
TEST_F(InfusionTest, VolumeTracker_ZeroTime_ReturnsTrue){
    fake_time = 0; fake_ticks = 0;
    EXPECT_EQ(vt.cal(), true);
}
TEST_F(InfusionTest, InfusionMode_OcclusionAlarm_NotifyCount1){
    fake_press = 400.0F; fake_time = 0;
    constant.run();
    EXPECT_EQ(alarm.noti_count, 1);
}
TEST_F(InfusionTest, InfusionMode_ApplyRate_NoError){
    constant.run();
    constant.run();
}
TEST_F(InfusionTest, InfusionMode_NoPressure_NoAlarm){
    fake_press = 0.0F; fake_time = 0; fake_ticks = 0;
    constant.run();
    EXPECT_EQ(alarm.noti_count, 0);
}
TEST_F(InfusionTest, VolumeTracker_ZeroTicks_AlarmTriggered){
    fake_time = 3600000; fake_ticks = 0;
    constant.run();
    EXPECT_GT(alarm.noti_count, 0);
}
TEST_F(InfusionTest, Hardware_GetPumpMode_ReturnsConstant){
    EXPECT_EQ(get_pump_mode(), PumpMode::Constant_mode);
}