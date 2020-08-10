# arduino-timer-cpp17 - library for scheduling function calls

Simple *non-blocking* timer library for calling functions **in / at / every** specified units of time. Supports millis, micros, time rollover, and compile-time configurable number of timers.

This library was inspired by [Michael Contreras' arduino-timer library](https://github.com/contrem/arduino-timer), but has been rewritten to make use of 'modern C++' types and
functionality. As a result this library requires that the Arduino IDE toolchain be manually configured for "gnu++17" mode.

### Use It

Include the library and create a *TimerSet* instance.
```cpp
#include <arduino-timer.h>

auto timerset = Timers::create_default();
```

Or using the *TimerSet* constructors for different timer limits / time resolution.
```cpp
Timers::TimerSet<10> timerset; // 10 concurrent timers, using millis as resolution
Timers::TimerSet<10, micros, delayMicroseconds> microtimerset; // 10 concurrent timers, using micros as resolution
```

Call *timerset*.**tick_and_delay()** in the ```loop``` function to execute handlers for any timers
which have expired and then delay until the next scheduled timer expiration.
```cpp
void loop() {
    timerset.tick_and_delay();
}
```

Call *timerset*.**tick()** in the ```loop``` function to execute handlers for any timers
which have expired, and then return so additional processing can be handled in the loop function.
```cpp
void loop() {
    timerset.tick();
}
```

Make a function to call (without arguments) when a *Timer* expires.
```cpp
Timers::HandlerResult function_to_call() {
    return { Timers::TimerStatus::repeat, 0 }; // to repeat the action - 'completed' to stop
}
```

Make a function to call (with an argument) when a *Timer* expires.
```cpp
Timers::HandlerResult function_to_call_with_arg(int value) {
    return { Timers::TimerStatus::completed, 0 }; // to stop the timer - 'repeat' to repeat the action
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
        return { Timers::TimerStatus::repeat, 0 }; // to repeat the action - 'completed' to stop
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
/* Create a timer object with default settings:
   millis resolution, TIMER_MAX_TASKS (=16) task slots, T = void *
*/
Timer<> timer_create_default(); // auto timer = timer_create_default();

/* Create a timer with max_tasks slots and time_func resolution */
Timer<size_t max_tasks = TIMER_MAX_TASKS, unsigned long (*time_func)(void) = millis, typename T = void *> timer;
Timer<> timer; // Equivalent to: auto timer = timer_create_default()
Timer<10> timer; // Timer with 10 task slots
Timer<10, micros> timer; // timer with 10 task slots and microsecond resolution
Timer<10, micros, int> timer; // timer with 10 task slots, microsecond resolution, and handler argument type int

/* Signature for handler functions - T = void * by default */
bool handler(T argument);

/* Timer Methods */
/* Ticks the timer forward, returns the ticks until next event, or 0 if none */
unsigned long tick(); // call this function in loop()

/* Calls handler with opaque as argument in delay units of time */
Timer<>::Task
in(unsigned long delay, handler_t handler, T opaque = T());

/* Calls handler with opaque as argument at time */
Timer<>::Task
at(unsigned long time, handler_t handler, T opaque = T());

/* Calls handler with opaque as argument every interval units of time */
Timer<>::Task
every(unsigned long interval, handler_t handler, T opaque = T());

/* Cancel a timer task */
void cancel(Timer<>::Task &task);
```

### Installation

[Check out the instructions](https://www.arduino.cc/en/Guide/Libraries) from Arduino.

**OR** copy **src/arduino-timer.h** into your project folder *(you won't get managed updates this way)*.

### Examples

Found in the [**examples**](examples) folder.

The simplest example, blinking an LED every second *(from examples/blink)*:

```cpp
#include <arduino-timer.h>

auto timerset = Timers::create_default(); // create a timerset with default settings

Timers::HandlerResult toggle_led() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggle the LED
  return { Timers::TimerStatus::repeat, 0 }; // keep timer active? true
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

If you find this project useful, [consider becoming a sponsor.](https://github.com/sponsors/contrem)
