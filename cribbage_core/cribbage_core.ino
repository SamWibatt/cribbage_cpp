/*
 * cribbage_cpp - sketch to test that cribbage_cpp routines work on arduino target hardware
 * Intended for SAMD21/51, might try with ESP32 too
 * Might as well just make it equivalent to unit tests in main.cpp - could even write macros to fake the gtest stuff and use
 * the code verbatim?
*/

#include <cstdint>
#include "card_utils.h"
#include "cribbage_core.h"



using namespace cribbage_core;

// GLOBALS ================================================================================================================================================

//globals equivalent to cribbagetest data members in unit testing
Cribbage cr;
uint32_t default_seed = 0x1337d00d;
std::vector<card_t> hand;
card_t starter;
std::vector<card_t> cardstack;
std::vector<Cribbage::score_entry> scorelist;
card_t card;
bool build_scorelists = true;

/* doesn't seem to work
//function to emulate printf to the serial console
void sprintf(spbuf,char *fmt, ...) {
  va_list argp;
  char fmtbuf[256];
  
  va_start(argp, fmt);

  sprintf(fmtbuf,fmt, argp);
  Serial.print(fmtbuf);
}
 */

//what DOES work is this:
//replace "printf(" with "sprintf(spbuf," (sans quotes) globally
//after each of these sprintf statements, put a " Serial.print(spbuf)" 
//global sprintf buffer
char spbuf[512];

//for timing performance things like 10,000,000 scorings of a hand
unsigned long starttime;
unsigned long endtime;

//flag if any tests had errors in a given unit test
// NEEDS TO BE RESET AT THE BEGINNING OF EVERY UNIT TEST
bool test_had_errors;


// CODE =========================================================================================================



// UNIT TEST FUNCTIONS =========================================================================================================================

// some starts to copying over gtest unit tests

// TEST and TEST_F
// NEED TO RESET GLOBAL test_had_errors boolean at beginning
// NEED TO PRINT SUCCESS OR FAIL AT THE END based on it

// equivalent to EXPECT_EQ macro - let's use functions bc they're so much better behaved...? well, there's the whole type thing.
#define EXPECT_EQ(a,b) { if(a != b) { test_had_errors = true; sprintf(spbuf,"EXPECT_EQ failed line %d of file %s\n",__LINE__, __FILE__); Serial.print(spbuf); } }


// HELPER FUNCTIONS ============================================================================================================================

//special helper function to create a hand - give it string reps of 5 cards
//and the first 4 become "hand" global and last becomes starter
void build_hand(std::string h1, std::string h2, std::string h3, std::string h4, std::string st) {
    hand.clear();
    hand.push_back(cr.getCardUtils().stringcard(h1));
    hand.push_back(cr.getCardUtils().stringcard(h2));
    hand.push_back(cr.getCardUtils().stringcard(h3));
    hand.push_back(cr.getCardUtils().stringcard(h4));
    starter = cr.getCardUtils().stringcard(st);
    //check for illegal cards
    if(hand[0] == cr.getCardUtils().ERROR_CARD_VAL) { sprintf(spbuf,"WARNING: card 1 is illegal string '%s'\n",h1.c_str()); Serial.print(spbuf);}
    if(hand[1] == cr.getCardUtils().ERROR_CARD_VAL) { sprintf(spbuf,"WARNING: card 2 is illegal string '%s'\n",h2.c_str()); Serial.print(spbuf);}
    if(hand[2] == cr.getCardUtils().ERROR_CARD_VAL) { sprintf(spbuf,"WARNING: card 3 is illegal string '%s'\n",h3.c_str()); Serial.print(spbuf);}
    if(hand[3] == cr.getCardUtils().ERROR_CARD_VAL) { sprintf(spbuf,"WARNING: card 4 is illegal string '%s'\n",h4.c_str()); Serial.print(spbuf);}
    if(starter == cr.getCardUtils().ERROR_CARD_VAL) { sprintf(spbuf,"WARNING: starter is illegal string '%s'\n",st.c_str()); Serial.print(spbuf); }
}


//for play
//they become cardstack "global"
void build_stack(std::vector<std::string> stackcardsstr) {
    cardstack.clear();
    for (auto j = 0; j < stackcardsstr.size(); j++)
        cardstack.push_back(cr.getCardUtils().stringcard(stackcardsstr[j]));
    //check for illegal cards
    for(auto j = 0; j < cardstack.size(); j++)
        if(cardstack[j] == cr.getCardUtils().ERROR_CARD_VAL) {
          sprintf(spbuf,"WARNING: stack card %d is illegal string '%s'\n",j,stackcardsstr[j].c_str()); Serial.print(spbuf);
        }
}

//cardstack should have the played card on it
void render_play_scorelist(std::vector<std::string> stackcardsstr, std::string cardstr) {
    index_t mask;
    sprintf(spbuf,"score list: -----------------\n");  Serial.print(spbuf);
    if(stackcardsstr.empty()) { sprintf(spbuf,"empty ");  Serial.print(spbuf); }
    else
        for(index_t j = 0; j< stackcardsstr.size(); j++) { sprintf(spbuf,"%s ",stackcardsstr[j].c_str());  Serial.print(spbuf); }
    //sprintf(spbuf,"%s ",cr.getCardUtils().cardstring(starter).c_str());
    sprintf(spbuf,"stack, played %s\n",cardstr.c_str()); Serial.print(spbuf);
    index_t totscore = 0;
    for (Cribbage::score_entry se : scorelist) {
        //remember the early cards are rightmost
        for(index_t j = 0,mask = 1 << cardstack.size(); j < cardstack.size(); j++, mask >>= 1)
            if(se.part_cards & mask) { sprintf(spbuf,"%s ",cr.getCardUtils().cardstring(cardstack[j]).c_str());  Serial.print(spbuf); }
            else { sprintf(spbuf,"-- ");  Serial.print(spbuf); }
        totscore += cr.scorePoints[se.score_index];
        sprintf(spbuf," %s %d (%d)\n",cr.scoreStrings[se.score_index].c_str(),cr.scorePoints[se.score_index],totscore);  Serial.print(spbuf);
    }
}


// ARDUINO SETUP AND LOOP ===========================================================================================================

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
  //card dealing unit test
  Serial.print("Card dealing unit test\n");
  test_had_errors = false;
  
  card_t cardorder[52] = { 15, 9, 49, 14, 22, 31, 16, 13, 0, 50, 8, 47, 28, 11, 35, 2, 1, 12, 45, 7, 21, 23, 6, 17, 34, 37,
            4, 41, 36, 40, 32, 38, 51, 19, 39, 46, 27, 10, 29, 26, 48, 33, 20, 30, 3, 44, 24, 42, 18, 25, 43, 5}; //last should be 5 };
  std::vector<card_t> deck(52);
  CardUtils c;
  c.v_srandom(9999);
  c.shuffle(deck);
  for(auto j = 0; j < 52; j++) EXPECT_EQ(deck[j],cardorder[j]);
  if(test_had_errors) Serial.println("FAILED!"); else Serial.println("SUCCESS");
  

  //should have a 29 hand in it! Expect a score of 29
  //LATER will check for detailed results
  Serial.print("Longest hand score unit test\n");
  test_had_errors = false;
  
  build_hand("5c", "5d", "Jh", "Js","5h");
  index_t handscore;
  
  //Serial.println("ONE MILLION!");
  starttime = millis();
  //for(auto j=0 ; j < 1000000; j++)
  handscore = cr.score_shew(hand,starter,&scorelist,build_scorelists);
  endtime = millis();
  sprintf(spbuf,"Timing: %lu millis\n",(endtime-starttime)); Serial.print(spbuf);
  EXPECT_EQ(handscore,(cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 7) +
      cr.scorePoints[Cribbage::SCORE_PAIRROYAL] + cr.scorePoints[Cribbage::SCORE_PAIR] + cr.scorePoints[Cribbage::SCORE_NOBS]);
  if(test_had_errors) Serial.println("FAILED!"); else Serial.println("SUCCESS");

  Serial.print("Play_card 4 of a kind and 31 unit test\n");
  test_had_errors = false;
  std::vector<std::string> startstack = {"3c","7s","7c","7d"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cr.getCardUtils().stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack,card,&scorelist,build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) + pyb.val(newcard))
  // self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore,cr.scorePoints[Cribbage::SCORE_THIRTYONE] + cr.scorePoints[Cribbage::SCORE_4KIND]);
  if(build_scorelists) render_play_scorelist(startstack,cardstr);
  if(test_had_errors) Serial.println("FAILED!"); else Serial.println("SUCCESS");

  
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
