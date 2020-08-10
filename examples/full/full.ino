/*
 * timer_full
 *
 * Full example using the arduino-timer-cpp17 library.
 * Shows:
 *  - Setting a different number of tasks with microsecond resolution
 *  - disabling a repeated function
 *  - running a function after a delay
 *  - cancelling a task
 *
 */

#include <arduino-timer-cpp17.h>

auto timerset = Timers::create_default(); // create a TimerSet with default settings
Timers::TimerSet<> default_timerset; // same as above

// create a TimerSet that can hold 1 concurrent task, with microsecond resolution
Timers::TimerSet<1, micros, delayMicroseconds> microtimerset;

// create a TimerSet that holds 16 tasks, with millisecond resolution
Timers::TimerSet<16, millis> t_timerset;

Timers::HandlerResult toggle_led() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggle the LED
  return Timers::TimerStatus::repeat;
}

Timers::HandlerResult print_message(const char *message) {
  Serial.print("print_message: ");
  Serial.println(message);
  return Timers::TimerStatus::repeat;
}

size_t repeat_count = 1;
Timers::HandlerResult repeat_x_times(size_t limit) {
  Serial.print("repeat_x_times: ");
  Serial.print(repeat_count);
  Serial.print("/");
  Serial.println(limit);

  // remove this task after limit reached
  return ++repeat_count <= limit ? Timers::TimerStatus::repeat : Timers::TimerStatus::completed;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT

  // call the toggle_led function every 500 millis (half second)
  timerset.every(500, toggle_led);

  // call the repeat_x_times function every 1000 millis (1 second)
  timerset.every(1000, [](){ return repeat_x_times(10); });

  // call the print_message function every 1000 millis (1 second),
  // passing it an argument string
  t_timerset.every(1000, [](){ return print_message("called every second"); });

  // call the print_message function in five seconds
  t_timerset.in(5000, [](){ return print_message("delayed five seconds"); });

  // call the print_message function at time + 10 seconds
  t_timerset.at(millis() + 10000, [](){ return print_message("call at millis() + 10 seconds"); });

  // call the toggle_led function every 500 millis (half-second)
  auto timer = timerset.every(500, toggle_led);
  timerset.cancel(timer); // this task is now cancelled, and will not run

  // call print_message in 2 seconds, but with microsecond resolution
  microtimerset.in(2000000, [](){ return print_message("delayed two seconds using microseconds"); });

  if (!microtimerset.in(5000, [](){ return print_message("never printed"); })) {
    /* this fails because we created microtimerset with only 1 concurrent timer slot */
    Serial.println("Failed to add microsecond event - timer full");
  }
}

void loop() {
  timerset.tick();
  t_timerset.tick();
  microtimerset.tick();
}
