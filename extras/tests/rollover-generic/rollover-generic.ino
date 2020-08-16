/*
  Test timer rollover handling
*/

#include <arduino-timer-cpp17.hpp>

Timers::Timepoint wrapping_millis();

Timers::TimerSet<1, Timers::Clock::custom<wrapping_millis>> timerset; // this timer will wrap
auto _timerset = Timers::create_default(); // to count milliseconds

Timers::Timepoint _millis = 0L;
Timers::Timepoint wrapping_millis()
{
    // uses _millis controled by _timer
    // 6-second time loop starting at rollover - 3 seconds
    if (_millis - (-3000) >= 6000)
        _millis = -3000;
    return _millis;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    _timerset.every(1, []()
		       {
			   ++_millis; // increase _millis every millisecond
			   return Timers::TimerStatus::repeat;
		       });

    // should blink the LED every second, regardless of wrapping
    timerset.every(1000, []()
			 {
			     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
			     return Timers::TimerStatus::repeat;
			 });
}

void loop() {
    _timerset.tick();
    timerset.tick();
}
