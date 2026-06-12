#include <zephyr/kernel.h>
#include <cmath>
class OcclusionMonitor{
    public:
    float press;
    static constexpr float thrshold = 300.0f;
    bool isocclued(){
        press = sensor_press();
        if (press  > thrshold){
            return false;
        }
        else{
            return true;
        }
    }

};
class VolumeTracker{
    public:
    uint32_t ticks;
    uint64_t initial;
    float elapsed_time;
    float expected;
    float rate;
    float deviation;
    float ml;
    float actual;   
    public:
    VolumeTracker(float setrate, uint64_t start_ms , float ml_step) : rate(setrate),initial(start_ms), ml(ml_step){}
    bool cal(){
        elapsed_time = (k_uptime_get() - initial)/3600000.0f;
        ticks = (uint32_t)atomic_get(&tick_count);
        expected = elapsed_time*rate;
        actual = ticks*ml;
        float deviation = fabsf(expected-actual)/expected*100.0f;
        if(deviation > 5.0f){
            return false;
        }
        else{
            return true;
        }
    }
};
class Alarmobserver{
    public:
    virtual void update() = 0;
};
class led : public Alarmobserver{
    public:
    void update(){
        printf("led");
    }
};
class buz : public Alarmobserver{
    public:
    void update(){
        printf("buzzzer");
    }
};
class AlarmManager{
    public:
    Alarmobserver* alarm[10];
    uint8_t count = 0;
    void add(Alarmobserver* obj){
        alarm[count] = obj;
        count++;
    }
    void notify(){
        for(int i = 0 ; i < count ; i++){
            alarm[i]->update();
        }
    }
};
class InfusionMode{
    public:
    VolumeTracker &volume;
    OcclusionMonitor &occlu;
    AlarmManager &alarm;
    float ml_sec;
    float step_sec;
    bool started_ = false;
    int64_t start_ms_ = 0;

    uint32_t delay;
    public:
        InfusionMode(VolumeTracker &vol,OcclusionMonitor &occ, AlarmManager &ala) : volume(vol) , occlu(occ) , alarm(ala){}
        virtual float computeTargetRate() = 0;  
    void run(){             
        float rate = computeTargetRate();
        applyRate(rate);
        checkAlarm();
    }
        void applyRate(float rate){
                if (!started_) {
                    start_ms_ = k_uptime_get();
                    started_ = true;
                }
            ml_sec = rate/3600.0f;
            step_sec = ml_sec*200;
            delay = 1000000.0f / step_sec;
            set_delay_rate(delay);
        }
        void checkAlarm(){
            if(!volume.cal()){
                alarm.notify();
            }
            if(!occlu.isocclued()){
                alarm.notify();
            }
    }
};
class ConstantRateMode : public InfusionMode{
    public:
    float setrate;
ConstantRateMode(float rate, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
    : InfusionMode(vt, om, am), setrate(rate) {}
float computeTargetRate()override {
    return setrate;
}
};
class LinearRampMode : public InfusionMode{
public:
    float initial;
    float incr;
    float fin;
    float current_lvl;
LinearRampMode(float st, float in, float fi, VolumeTracker &vt, OcclusionMonitor &om, AlarmManager &am)
    : InfusionMode(vt, om, am), initial(st), incr(in), fin(fi), current_lvl(st) {}
    float computeTargetRate() override{
        current_lvl += incr;
        if(current_lvl > fin){
            current_lvl = fin;
        }
        return current_lvl;
}
};  