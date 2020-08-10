/*
 * timer_blink_print
 *
 * Blinks the built-in LED every half second, and prints a messages every
 * second using the arduino-timer-cpp17 library.
 *
 */

#include <arduino-timer-cpp17.h>

auto timerset = Timers::create_default(); // create a TimerSet with default settings

Timers::HandlerResult toggle_led() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // toggle the LED
  return Timers::TimerStatus::repeat;
}

Timers::HandlerResult print_message() {
  Serial.print("print_message: Called at: ");
  Serial.println(millis());
  return Timers::TimerStatus::repeat;
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // set LED pin to OUTPUT

  // call the toggle_led function every 500 millis (half second)
  timerset.every(500, toggle_led);

  // call the print_message function every 1000 millis (1 second)
  timerset.every(1000, print_message);
}

void loop() {
  timerset.tick_and_delay(); // tick the timer
}
