/*
 * cribbage_cpp - sketch to test that cribbage_cpp routines work on arduino target hardware
 * Intended for SAMD21/51, might try with ESP32 too
*/

#include <cstdint>
#include "cribbage_cpp.h"

using namespace cribbage_cpp;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // prints title with ending line break
  Serial.println("Hello and welcome to cribbage_cpp");


  // do some card stuff!
  std::vector<uint8_t> deck;
  my_srandom(9999);
  shuffle(deck);

  //then report all the cards out to serial
  Serial.println("Shuffled cards from 9999 seed:");
  for(auto j = 0; j < 52; j++) {
    Serial.print(deck[j]);
    if (((j+1)%13) == 0) Serial.println();
    Serial.print(" ");
  }
  Serial.println();
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
