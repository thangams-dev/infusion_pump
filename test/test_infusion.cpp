#include <gtest/gtest.h>
#include "alarm.hpp"
#include "occlusion.hpp"
#include "volume.hpp"
#include "infusion.hpp"
#include "mock_hardware.hpp"

extern float fake_press;
extern uint32_t fake_ticks;
extern int64_t fake_time;
class infusion_test :public testing::Test{
    protected:
    OcclusionMonitor occlu;
    AlarmManager alarm;
    VolumeTracker vt{100.0f, 0, 0.005f};
    ConstantRateMode constant{100.0f,vt,occlu,alarm};
    LinearRampMode ramp{10.0f,2.0f,100.0f,vt,occlu,alarm};
};
TEST_F(infusion_test,occlu_cal){
    EXPECT_EQ(occlu.isocclued(),true);
}   
TEST_F(infusion_test,notify_alarm){
    led l;
    buz b;
    alarm.add(&l);
    alarm.add(&b);
    alarm.notify();
    EXPECT_EQ(alarm.noti_count,1);
}
TEST_F(infusion_test,vol_cal){
    fake_time = 2000;
    EXPECT_EQ(vt.cal(),false);
}
TEST_F(infusion_test, const_t){
    EXPECT_EQ(constant.computeTargetRate(),100.0f);
}
TEST_F(infusion_test, ramp_t){
    EXPECT_EQ(ramp.computeTargetRate(),12.0f);
}
TEST_F(infusion_test, multi_ramp_t){
    ramp.computeTargetRate();
    ramp.computeTargetRate();
    ramp.computeTargetRate();
    EXPECT_EQ(ramp.computeTargetRate(),18.0f);
}
TEST_F(infusion_test, occlu_cal_alram){
    fake_press = 400.0f;
    EXPECT_EQ(occlu.isocclued(),false);
}
TEST_F(infusion_test, vt_alarm){
    fake_time = 3600000;
    fake_ticks = 20000;
    EXPECT_EQ(vt.cal(),true);
}
TEST_F(infusion_test, switch_mode){
    InfusionMode *active = &constant;

    EXPECT_EQ(active->computeTargetRate(),100.0f);

    active = &ramp;
    EXPECT_EQ(active->computeTargetRate(),12.0f);
}
TEST_F(infusion_test, ramp_cap){
    for(int i=0; i<50; i++) ramp.computeTargetRate();
    EXPECT_EQ(ramp.computeTargetRate(), 100.0f);  // hits fin cap
}

TEST_F(infusion_test, alarm_no_observer){
 // count=0, loop doesn't run
    EXPECT_EQ(alarm.noti_count, 0);
}

TEST_F(infusion_test, vt_zero_expected){
    fake_time = 0;
    fake_ticks = 0;
    EXPECT_EQ(vt.cal(), true);  // no deviation
}
TEST_F(infusion_test, check_alarm){
    fake_press = 400.0f;
    fake_time = 0;
    constant.run();
    EXPECT_EQ(alarm.noti_count, 1);  // alarm triggered
}
TEST_F(infusion_test, apply_rate){
    constant.run();
    constant.run();
}
TEST_F(infusion_test, vol_ok_no_alarm){
    fake_press = 0.0f;  // no occlusion
    fake_time = 0;
    fake_ticks = 0;
    constant.run();
    EXPECT_EQ(alarm.noti_count, 0);
}
TEST_F(infusion_test, vol_deviation_alarm){
    fake_time = 3600000;
    fake_ticks = 0;  // expected high, actual zero → deviation >5%
    constant.run();
    EXPECT_GT(alarm.noti_count, 0);
}
TEST_F(infusion_test, get_mode){
    EXPECT_EQ(get_pump_mode(), PumpMode::Constant_mode);
}