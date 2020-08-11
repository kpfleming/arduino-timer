/*
 * timer_blink
 *
 * Blinks the built-in LED every second using the arduino-timer-cpp17 library.
 *
 */

#include <arduino-timer-cpp17.h>

auto timerset = Timers::create_default(); // create a TimerSet with default settings

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
