#include "plat_play_main.h"
#include "Arduino.h"

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  // SEAN NOTES THIS IS NEEDED FOR SERIAL TO WORK ON CPX BUT MAKES IT SO IT DOESN'T START BLINKING UNTIL
  // THE MONITOR IS CONNECTED!
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // call play_main's run()
  Serial.println("About to call run!!!");
  run();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN,
               HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(333);          // wait for a third of a second
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
  delay(333);                      // wait for a third of a second
}
