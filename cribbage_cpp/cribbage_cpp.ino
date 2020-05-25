/*
 * cribbage_cpp - sketch to test that cribbage_cpp routines work on arduino target hardware
 * Intended for SAMD21/51, might try with ESP32 too
 * Might as well just make it equivalent to unit tests in main.cpp - could even write macros to fake the gtest stuff and use
 * the code verbatim?
*/

#include <cstdint>
#include "cribbage_cpp.h"

using namespace cribbage_cpp;

//globals equivalent to cribbagetest data members in unit testing
Cribbage cr;
uint32_t default_seed = 0x1337d00d;
std::vector<uint8_t> hand;
uint8_t starter;
std::vector<Cribbage::score_entry> scorelist;


//special helper function to create a hand - give it string reps of 5 cards
//and the first 4 become "hand" global and last becomes starter
void build_hand(std::string h1, std::string h2, std::string h3, std::string h4, std::string st) {
    hand.clear();
    hand.push_back(cr.getCardUtils().stringcard(h1));
    hand.push_back(cr.getCardUtils().stringcard(h2));
    hand.push_back(cr.getCardUtils().stringcard(h3));
    hand.push_back(cr.getCardUtils().stringcard(h4));
    starter = cr.getCardUtils().stringcard(st);
}


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
  CardUtils cu;
  std::vector<uint8_t> deck;
  cu.v_srandom(9999);
  cu.shuffle(deck);

  //then report all the cards out to serial
  Serial.println("Shuffled cards from 9999 seed:");
  Serial.print(" ");
  for(auto j = 0; j < 52; j++) {
    if (deck[j]<10) Serial.print(" ");    //gross way to do %2d print
    Serial.print(deck[j]); Serial.print("-"); Serial.print(cu.cardstring(deck[j]).c_str()); Serial.print(" ");
    if (((j+1)%13) == 0) Serial.println();
    Serial.print(" ");
  }
  Serial.println();

  //should have a 29 hand in it! Expect a score of 29
  //LATER will check for detailed results
  
  build_hand("5c", "5d", "Jh", "5s","5h");
  uint8_t handscore;
  handscore = cr.score_shew(hand,starter,&scorelist,false);
  Serial.print(handscore);
  Serial.print(" should be ");
  Serial.println((cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 8) +
      cr.scorePoints[Cribbage::SCORE_4KIND] + cr.scorePoints[Cribbage::SCORE_NOBS]);
  
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
