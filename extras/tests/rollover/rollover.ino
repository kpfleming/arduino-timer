/*
  Test timer rollover handling
*/

#include <util/atomic.h>
#include <arduino-timer-cpp17.hpp>

auto timerset = Timers::create_default();

// https://arduino.stackexchange.com/questions/12587/how-can-i-handle-the-millis-rollover
void set_millis(unsigned long ms)
{
    extern unsigned long timer0_millis;
    ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
        timer0_millis = ms;
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    timerset.every(1000, []()
			 {
			     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
			     return Timers::TimerStatus::repeat;
			 });
}

void loop() {
    // 6-second time loop starting at rollover - 3 seconds
    if (millis() - (-3000) >= 6000)
        set_millis(-3000);
    timerset.tick();
}
