# arduino-timer-cpp17 - library for scheduling function calls

Simple *non-blocking* timer library for calling functions **in / at / every** specified units of time. Supports millis, micros, time rollover, and compile-time configurable number of timers.

This library was inspired by [Michael Contreras' arduino-timer library](https://github.com/contrem/arduino-timer), but has been rewritten to make use of 'modern C++' types and
functionality. As a result this library requires that the Arduino IDE toolchain be manually configured for "gnu++17" mode.

### Use It

Include the library and create a *TimerSet* instance.
```cpp
#include <arduino-timer-cpp17.h>

auto timerset = Timers::create_default();
```

Or using the *TimerSet* constructors for different timer limits / time clocks.
```cpp
Timers::TimerSet<10> timerset; // 10 concurrent Timers, using millisecond clock
Timers::TimerSet<10, Timers::Clock::micros> microtimerset; // 10 concurrent Timers, using microsecond clock
```

Call *timerset*.**tick_and_delay()** in the ```loop``` function to execute handlers for any Timers
which have expired and then delay until the next scheduled Timer expiration.
```cpp
void loop() {
    timerset.tick_and_delay();
}
```

Call *timerset*.**tick()** in the ```loop``` function to execute handlers for any Timers
which have expired, and then return so additional processing can be handled in the loop function.
```cpp
void loop() {
    timerset.tick();
}
```

Make a function to call (without arguments) when a *Timer* expires.
```cpp
Timers::HandlerResult function_to_call() {
    return Timers::TimerStatus::repeat; // to repeat the action - 'completed' to stop
}
```

Make a function to call (with an argument) when a *Timer* expires.
```cpp
Timers::HandlerResult function_to_call_with_arg(int value) {
    return Timers::TimerStatus::completed; // to stop the Timer - 'repeat' to repeat the action
}
```

Make a function to call (without arguments) when a *Timer* expires, which can reschedule itself based
on a digital input. In this example, if digital input 4 is active when the function is called, it will
change its own repeat interval to 5 seconds (5000 millis).
```cpp
Timers::HandlerResult function_to_call_and_reschedule() {
    if (digitalRead(4)) {
        return { Timers::TimerStatus::reschedule, 5000 }; // to change the repeat interval to 5000 ms
    } else {
        return Timers::TimerStatus::repeat; // to repeat the action - 'completed' to stop
    }
}
```

Call *function\_to\_call* **in** *delay* units of time *(unit of time defaults to milliseconds)*.
```cpp
timerset.in(delay, function_to_call);
```

Call *function\_to\_call|_with|_arg* **in** *delay* units of time *(unit of time defaults to milliseconds)*.
```cpp
timerset.in(delay, [](){ return function_to_call_with_arg(42); });
```

Call functions **at** a specific *time*.
```cpp
timerset.at(time, function_to_call);
timerset.at(time, [](){ return function_to_call_with_arg(42); });
```

Call functions **every** *interval* units of time.
```cpp
timerset.every(interval, function_to_call);
timerset.every(interval, [](){ return function_to_call_with_arg(42); });
```

Call functions **now** and **every** *interval* units of time.
```cpp
timerset.now_and_every(interval, function_to_call);
timerset.now_and_every(interval, [](){ return function_to_call_with_arg(42); });
```

To **cancel** a *Timer*
```cpp
auto timer = timerset.in(delay, function_to_call);
timerset.cancel(timer);
```

### API

```cpp
/* Constructors */
/* Create a TimerSet object with default settings:
   millisecond clock, TIMERSET_DEFAULT_TIMERS (=16) Timer slots
*/
Timers::TimerSet<> Timers::create_default() // auto timerset = Timers::create_default();

/* Create a TimerSet with max_timers slots and millisecond clock */
Timers::TimerSet<size_t max_timers = TIMERSET_DEFAULT_TIMERS, typename clock = Clock::millis>()

Timers::TimerSet<> timerset; // Equivalent to: auto timerset = Timers::create_default();
Timers::TimerSet<10> timerset; // TimerSet with 10 Timer slots
Timers::TimerSet<10, Timers::Clock::micros> timerset; // TimerSet with 10 Timer slots and microsecond clock

/* Handler function signature; returns a HandlerResult */
Timers::HandlerResult handler() // declared as Timers::Handler

/* HandlerResult contains a TimerStatus, and optional 'next' Timepoint */
/* (in handler function) */
return Timers::TimerStatus::completed; // remove Timer from TimerSet
return Timers::TimerStatus::repeat; // repeat Timer at previously-set interval
return { Timers::TimerStatus::reschedule, 3000 }; // repeat Timer at new interval of 3000 clock ticks

/* TimerSet Methods */
// Ticks the TimerSet forward, returns the ticks until next event, or 0 if none
Timers::Timepoint tick(); // call this function in loop()

// Ticks the TimerSet forward, and delays until the next event
void tick_and_delay(); // call this function in loop()

/* Calls handler in delay units of time */
Timers::TimerHandle
in(Timers::Timepoint delay, Timers::Handler handler);

/* Calls handler at time */
Timers::TimerHandle
at(Timers::Timepoint time, Timers::Handler handler);

/* Calls handler every interval units of time */
Timers::TimerHandle
every(Timers::Timepoint interval, Timers::Handler handler);

/* Calls handler now and every interval units of time */
Timers::TimerHandle
now_and_every(Timers::Timepoint interval, Timers::Handler handler);

/* Cancel a Timer */
void cancel(Timers::TimerHandle timer);
```

### Installation

Copy **src/arduino-timer-cpp17.h** into your project folder.

### Examples

Found in the [**examples**](examples) folder.

The simplest example, blinking an LED every second *(from examples/blink)*:

```cpp
#include <arduino-timer-cpp17.h>

auto timerset = Timers::create_default(); // create a timerset with default settings

Timers::HandlerResult toggle_led() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggle the LED
  return Timers::TimerStatus::repeat;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT

  // call the toggle_led function every 1000 millis (1 second)
  timerset.every(1000, toggle_led);
}

void loop() {
  timerset.tick_and_delay(); // tick the timer
}
```

### LICENSE

Check the LICENSE file - 3-Clause BSD License

### Notes

Currently only a software timer. Any blocking code delaying *timerset*.**tick()** will prevent the TimerSet from moving forward and calling any functions.

The library does not do any dynamic memory allocation.

The number of concurrent timers is a compile time constant, meaning there is a limit to the number of concurrent timers. The **in / at / every / now_and_every**
functions return a TimerHandle which evaluates to ```false``` if the TimerSet is full.

A *TimerHandle* value is valid only for the TimerSet that created it, and only for the lifetime of that timer.

Change the number of concurrent timers using the *Timer* constructors. Save memory by reducing the number, increase memory use by having more. The default is **TIMERSET_DEFAULT_TIMERS** which is currently 16.
