#include <stdio.h>
#include <iostream>
#include <string>
#include <utility>
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "gtest/gtest.h"
#include "plat_io.h"

using namespace cardutils;
using namespace cribbage_core;

// need some globals in order to use the classes - gross but there it is
extern CardUtils cu;
extern Cribbage cr;

// then a fixture for default player tests
class DefaultPlayerTest : public ::testing::Test {
 protected:
  // data members
  uint32_t default_seed = 0x1337d00d;
  std::vector<card_t> cardstack;
  std::vector<card_t> hand;
  card_t starter;
  card_t card;
  std::vector<Cribbage::score_entry> play_scorelist;
  std::vector<Cribbage::score_entry> shew_scorelist;
  bool build_scorelists =
      true;  // FOR SPEED TESTING WITH OR WITHOUT BUILDING SCORELISTS

  // special helper function to create a hand - give it string reps of 5 cards
  // and the first 4 become "hand" global and last becomes starter
  void build_hand(std::string h1, std::string h2, std::string h3,
                  std::string h4, std::string st) {
    hand.clear();
    hand.push_back(cu.stringcard(h1));
    hand.push_back(cu.stringcard(h2));
    hand.push_back(cu.stringcard(h3));
    hand.push_back(cu.stringcard(h4));
    starter = cu.stringcard(st);
    // check for illegal cards
    if (hand[0] == cu.ERROR_CARD_VAL)
      plprintf("WARNING: card 1 is illegal string '%s'\n", h1.c_str());
    if (hand[1] == cu.ERROR_CARD_VAL)
      plprintf("WARNING: card 2 is illegal string '%s'\n", h2.c_str());
    if (hand[2] == cu.ERROR_CARD_VAL)
      plprintf("WARNING: card 3 is illegal string '%s'\n", h3.c_str());
    if (hand[3] == cu.ERROR_CARD_VAL)
      plprintf("WARNING: card 4 is illegal string '%s'\n", h4.c_str());
    if (starter == cu.ERROR_CARD_VAL)
      plprintf("WARNING: starter is illegal string '%s'\n", st.c_str());
  }

  // special helper function to create a card stack - give it string reps cards
  // they become cardstack "global"
  void build_stack(std::vector<std::string> stackcardsstr) {
    cardstack.clear();
    for (auto j = 0; j < stackcardsstr.size(); j++)
      cardstack.push_back(cu.stringcard(stackcardsstr[j]));
    // check for illegal cards
    for (auto j = 0; j < cardstack.size(); j++)
      if (cardstack[j] == cu.ERROR_CARD_VAL)
        plprintf("WARNING: stack card %d is illegal string '%s'\n", j,
                 stackcardsstr[j].c_str());
  }

  void render_shew_scorelist() {
    index_t mask;
    plprintf("score list: -----------------\n");
    for (auto j = 0; j < 4; j++)
      plprintf("%s ", cu.cardstring(hand[j]).c_str());
    plprintf("%s ", cu.cardstring(starter).c_str());
    plprintf(" hand\n");
    index_t totscore = 0;
    for (Cribbage::score_entry se : shew_scorelist) {
      for (auto j = 0, mask = 0x01; j < 5; j++, mask <<= 1)
        if (se.part_cards & mask)
          if (j < 4)
            plprintf("%s ", cu.cardstring(hand[j]).c_str());
          else
            plprintf("%s ", cu.cardstring(starter).c_str());
        else
          plprintf("-- ");
      totscore += cr.scorePoints[se.score_index];
      plprintf(" %s %d (%d)\n", cr.scoreStrings[se.score_index].c_str(),
               cr.scorePoints[se.score_index], totscore);
    }
  }

  // cardstack should have the played card on it
  void render_play_scorelist(std::vector<std::string> stackcardsstr,
                             std::string cardstr) {
    index_t mask;
    plprintf("score list: -----------------\n");
    if (stackcardsstr.empty())
      plprintf("empty ");
    else
      for (index_t j = 0; j < stackcardsstr.size(); j++)
        plprintf("%s ", stackcardsstr[j].c_str());
    // plprintf("%s ",cu.cardstring(starter).c_str());
    plprintf("stack, played %s\n", cardstr.c_str());
    index_t totscore = 0;
    for (Cribbage::score_entry se : play_scorelist) {
      // remember the early cards are rightmost
      for (index_t j = 0, mask = 1 << cardstack.size(); j < cardstack.size();
           j++, mask >>= 1)
        if (se.part_cards & mask)
          plprintf("%s ", cu.cardstring(cardstack[j]).c_str());
        else
          plprintf("-- ");
      totscore += cr.scorePoints[se.score_index];
      plprintf(" %s %d (%d)\n", cr.scoreStrings[se.score_index].c_str(),
               cr.scorePoints[se.score_index], totscore);
    }
  }

  // setup initializes data members, runs BEFORE EVERY TEST
  void SetUp() override {
    cu.v_srandom(default_seed);
    // I think the longest possible list of scores is for 29: eight fifteens + 4
    // of a kind + nobs? VERIFY
    shew_scorelist.reserve(10);
    // there are really only 3 possible scores in play - 15, pairs & variants,
    // and runs, but reserve 10 why not
    play_scorelist.reserve(10);
  }

  // teardown cleans up after data members, runs AFTER EVERY TEST
  void TearDown() override {}
};

TEST_F(DefaultPlayerTest, T000_Setup) {
  CribbagePlayer cp;
  EXPECT_EQ(cp.is_dealer(), false);
  EXPECT_EQ(cp.get_name(), "");
  EXPECT_EQ(cp.get_score(), 0);
  EXPECT_EQ(cp.get_cards().capacity(), 6);
  EXPECT_EQ(cp.get_used_cards().capacity(), 4);
  EXPECT_EQ(cp.get_crib().capacity(), 4);
  EXPECT_EQ(cp.get_cards().empty(), true);
  EXPECT_EQ(cp.get_used_cards().empty(), true);
  EXPECT_EQ(cp.get_crib().empty(), true);
}

TEST_F(DefaultPlayerTest, T010_SetupWNameNDealer) {
  CribbagePlayer cp("Player1",true);
  EXPECT_EQ(cp.is_dealer(), true);
  EXPECT_EQ(cp.get_name(), "Player1");
  EXPECT_EQ(cp.get_score(), 0);
  EXPECT_EQ(cp.get_cards().capacity(), 6);
  EXPECT_EQ(cp.get_used_cards().capacity(), 4);
  EXPECT_EQ(cp.get_crib().capacity(), 4);
  EXPECT_EQ(cp.get_cards().empty(), true);
  EXPECT_EQ(cp.get_used_cards().empty(), true);
  EXPECT_EQ(cp.get_crib().empty(), true);
}

// For accessor tests I'm going to trust the setup values given tests T000 and T010 are ok.
// inline index_t get_score() { return score; }
// inline void set_score(index_t nuscore) { score = nuscore; }
TEST_F(DefaultPlayerTest, T020_SetGetScore) {
  CribbagePlayer cp("Player1",true);
  cp.set_score(99);  //must be a score that fits in index_t
  EXPECT_EQ(cp.get_score(), 99);
}

// inline bool is_dealer() { return dealer; }
// inline void set_dealer(bool nudeal) { dealer = nudeal; }
TEST_F(DefaultPlayerTest, T030_SetGetDealer) {
  CribbagePlayer cp("Player1",true);
  cp.set_dealer(false);
  EXPECT_EQ(cp.is_dealer(), false);
}

// inline std::vector<card_t> &get_cards() { return cards; }
// inline void set_cards(std::vector<card_t> nucards) { cards = nucards; }
TEST_F(DefaultPlayerTest, T040_SetGetCards) {
  CribbagePlayer cp("Player1",true);
  build_hand("Qh", "0c", "9s", "3d", "5d");
  cp.set_cards(hand);
  EXPECT_EQ(cp.get_cards(), hand);
}

// inline void add_card(card_t nucard) { cards.push_back(nucard); }
TEST_F(DefaultPlayerTest, T050_AddCard) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d", "5d"});
  cp.set_cards(cardstack);
  card_t nucard = cu.stringcard("Jh");
  cp.add_card(nucard);
  cardstack.push_back(nucard);
  EXPECT_EQ(cp.get_cards(), cardstack);
}

TEST_F(DefaultPlayerTest, T060_AddCardTooMany) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d", "5d", "Qd"});
  cp.set_cards(cardstack);
  card_t nucard = cu.stringcard("Jh");
  cp.add_card(nucard);  //should have no effect
  EXPECT_EQ(cp.get_cards(), cardstack);
}

// inline std::vector<card_t> &get_crib() { return crib; }
// inline void set_crib(std::vector<card_t> nucrib) { crib = nucrib; }
TEST_F(DefaultPlayerTest, T070_SetGetCrib) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "5d", "Jh"});
  cp.set_crib(cardstack);
  EXPECT_EQ(cp.get_crib(), cardstack);
}

// inline void add_crib(card_t nucard) { crib.push_back(nucard); }
TEST_F(DefaultPlayerTest, T080_AddCrib) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s"});
  cp.set_crib(cardstack);
  card_t nucard = cu.stringcard("Jh");
  cp.add_crib(nucard);
  cardstack.push_back(nucard);
  EXPECT_EQ(cp.get_crib(), cardstack);
}

TEST_F(DefaultPlayerTest, T090_AddCribTooMany) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d"});
  cp.set_crib(cardstack);
  card_t nucard = cu.stringcard("Jh");
  cp.add_crib(nucard);  //should have no effect
  EXPECT_EQ(cp.get_crib(), cardstack);
}

// inline std::vector<card_t> &get_used_cards() { return used_cards; }
// inline void set_used_cards(std::vector<card_t> nuused) {
//   used_cards = nuused;
// }
TEST_F(DefaultPlayerTest, T100_SetGetUsed) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "5d", "Jh"});
  cp.set_used_cards(cardstack);
  EXPECT_EQ(cp.get_used_cards(), cardstack);
}

// inline void add_used_cards(card_t nucard) { used_cards.push_back(nucard); }
TEST_F(DefaultPlayerTest, T110_AddUsed) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s"});
  cp.set_used_cards(cardstack);
  card_t nucard = cu.stringcard("Jh");
  cp.add_used_cards(nucard);
  cardstack.push_back(nucard);
  EXPECT_EQ(cp.get_used_cards(), cardstack);
}

TEST_F(DefaultPlayerTest, T120_AddUsedTooMany) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d"});
  cp.set_used_cards(cardstack);
  card_t nucard = cu.stringcard("Jh");
  cp.add_used_cards(nucard);  //should have no effect
  EXPECT_EQ(cp.get_used_cards(), cardstack);
}

// inline std::string get_name() { return name; }
// inline void set_name(std::string nuname) { name = nuname; }
TEST_F(DefaultPlayerTest, T130_SetGetName) {
  CribbagePlayer cp("Player1",true);
  cp.set_name("Rollo");
  EXPECT_EQ(cp.get_name(), "Rollo");
}

// non-accessor functions!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Kind of a dumb test, but ok
TEST_F(DefaultPlayerTest, T140_CutIndex) {
  CribbagePlayer cp("Player1",true);
  cu.v_srandom(9999);
  EXPECT_EQ(cp.get_cut_index(52),8);
}

// do a million cuts of a 52 card deck and ensure
// that it's always not from the first or last 4 cards bc them's the rules
TEST_F(DefaultPlayerTest, T150_1MCutIndex) {
  CribbagePlayer cp("Player1",true);
  cu.v_srandom(9999);
  for(auto j = 0; j < 1000000; j++) {
    auto c = cp.get_cut_index(52);
    EXPECT_LE(c,47);
    EXPECT_GE(c,4);
  }
}

// make sure 9 card deck returns 4
TEST_F(DefaultPlayerTest, T160_CutIndex9) {
  CribbagePlayer cp("Player1",true);
  cu.v_srandom(9999);
  for(auto j = 0; j< 100; j++)  //every time!
    EXPECT_EQ(cp.get_cut_index(9),4);
}

TEST_F(DefaultPlayerTest, T170_CutIndexError) {
  CribbagePlayer cp("Player1",true);
  cu.v_srandom(9999);
  EXPECT_EQ(cp.get_cut_index(8),cr.ERROR_SCORE_VAL);
}

//default discards = last two cards in the provided hand
TEST_F(DefaultPlayerTest, T180_GetDiscards) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d", "Jh", "Ks"});
  auto res = std::pair<card_t,card_t>(cu.stringcard("Ks"),cu.stringcard("Jh"));
  EXPECT_EQ(cp.get_discards(cardstack), res);
}

//default discards needs hand to be 6 cards
TEST_F(DefaultPlayerTest, T190_GetDiscardsError) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d", "Jh"});
  auto res = std::pair<card_t,card_t>(cu.ERROR_CARD_VAL,cu.ERROR_CARD_VAL);
  EXPECT_EQ(cp.get_discards(cardstack), res);
}

/*
// choose a card to play given a current stack and a current hand
// default implementation is just grab the first legal one
// if no playable card is in the hand, return cu.ERROR_CARD_VAL;
card_t CribbagePlayer::get_play_card(std::vector<card_t> &cardvec,
                                     std::vector<card_t> &cardstack) {
  // index_t play_card(std::vector<card_t> &stack, card_t card,
  // std::vector<score_entry> *scores, bool build_list); the only illegal play is
  // to exceed 31, which returns cr.ERROR_SCORE_VAL IT DOES CHANGE CARDSTACK THO
  // SO FIGURE OUT IF I NEED TO CHANGE THAT

  // if there aren't any cards to play, return an error. May not be a real
  // error, that's caller's call
  if (cardvec.empty()) return cu.ERROR_CARD_VAL;

  // find the first card that is legal to play and take it out of cardvec and
  // return it
  for (auto j = 0; j < cardvec.size(); j++) {
    card_t c = *(cardvec.begin() + j);
    //so: play_card changes cardstack, yes? if this is being called hypothetically, the stack will
    //stack up.
    //options? Add a parameter to play_card to not do that,
    //pop it back off here.
    //I lean toward the former bc this is not the only place this will be called.
    //so that's done, it's that last false there
    index_t s = cr.play_card(cardstack, c, nullptr, false, false);
    if (s != cr.ERROR_SCORE_VAL) {
      cardvec.erase(cardvec.begin() + j);
      return c;
    }
  }

  // there was no playable card
  return cu.ERROR_CARD_VAL;
}
*/
//empty hand means there's no card to play
TEST_F(DefaultPlayerTest, T200_PlayCardError) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s", "3d", "Jh"});
  hand.clear();
  card_t c = cp.get_play_card(hand,cardstack,true);
  EXPECT_EQ(c, cu.ERROR_CARD_VAL);
}

//first legal card should be the 2 in this case, play that, even though the ace is also legal
TEST_F(DefaultPlayerTest, T210_PlayCard) {
  CribbagePlayer cp("Player1",true);
  build_stack({"Qh", "0c", "9s"});
  //recall build_hand takes starter as fifth arg so hand is really only 4h 9d 2d Ah
  build_hand("4h","9d","2d","Ah","Jh");
  //true means the card gets removed from hand
  card_t c = cp.get_play_card(hand,cardstack, true);
  EXPECT_EQ(c, cu.stringcard("2d"));
  build_stack({"4h","9d","Ah"});
  EXPECT_EQ(hand,cardstack);
}
