#include <stdio.h>
#include <iostream>
#include <string>
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

// then a fixture for play_card test
class PlayCardTest : public ::testing::Test {
 protected:
  // data members
  uint32_t default_seed = 0x1337d00d;
  std::vector<card_t> cardstack;
  card_t card;
  std::vector<Cribbage::score_entry> scorelist;
  bool build_scorelists =
      true;  // FOR SPEED TESTING WITH OR WITHOUT BUILDING SCORELISTS

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
    for (Cribbage::score_entry se : scorelist) {
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
    scorelist.reserve(10);
  }

  // teardown cleans up after data members, runs AFTER EVERY TEST
  void TearDown() override {}
};

// play a card that'll go over 31
TEST_F(PlayCardTest, T0000_Illegal) {
  // order in which they're played so stack will show it same bc it grows left
  // to right
  std::vector<std::string> startstack = {"Qh", "0c", "9s", "Ad"};
  std::string cardstr = "0d";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  EXPECT_EQ(playscore, cr.ERROR_SCORE_VAL);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

// all right! set up a hand that should have no scoring combinations in it and
// score it! Expect a score of 0
TEST_F(PlayCardTest, T000_Nothing) {
  // order in which they're played so stack will show it same bc it grows left
  // to right
  std::vector<std::string> startstack = {"Qh", "8c", "4s", "3d"};
  std::string cardstr = "4d";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  // for(auto j=0;j<10000000;j++)
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T001_Firstcard) {
  // order in which they're played so stack will show it same bc it grows left
  // to right
  std::vector<std::string> startstack = {};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T003_Fifteen) {
  // order in which they're played so stack will show it same bc it grows left
  // to right
  std::vector<std::string> startstack = {"5d"};
  std::string cardstr = "Jh";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T007_Thirtyone) {
  // order in which they're played so stack will show it same bc it grows left
  // to right
  std::vector<std::string> startstack = {"5d", "Jh", "Ac", "5h"};
  std::string cardstr = "Qh";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_THIRTYONE]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T010_Pair) {
  std::vector<std::string> startstack = {"3d"};
  std::string cardstr = "3h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_PAIR]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T020_NotPair) {
  std::vector<std::string> startstack = {"2c"};
  std::string cardstr = "7s";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T030_3OfAKind) {
  std::vector<std::string> startstack = {"4c", "4s"};
  std::string cardstr = "4h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T040_Not3OfAKind) {
  std::vector<std::string> startstack = {"4c", "Qd", "4s"};
  std::string cardstr = "4h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_PAIR]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T050_4OfAKind) {
  std::vector<std::string> startstack = {"Ad", "6c", "6s", "6d"};
  std::string cardstr = "6h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_4KIND]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T060_Not4OfAKind) {
  std::vector<std::string> startstack = {"2c", "2d", "Qd", "2s"};
  std::string cardstr = "2h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_PAIR]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T070_RunOf3) {
  std::vector<std::string> startstack = {"Ac", "2s"};
  std::string cardstr = "3h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN3]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T080_RunOf3OOO) {
  std::vector<std::string> startstack = {"8c", "6s"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN3]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T090_RunOf3intervening) {
  std::vector<std::string> startstack = {"5c", "6s", "Jd"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

//# TODO HEY DO THIS ONE from http://cribbagecorner.com/cribbage-rules-play
//#
//#scoring sequence in play
//#Submitted by Meade (not verified) on Tue, 04/06/2010 - 01:50.
//#
//#How do you score the following sequence made in play? A 7 was played first
//followed by a 9 and then a 8 to make a # sequence of three for 3 points. Here
//is the questions: My playing partner then played a 7. Is this another sequence
//# of three for 3 points and 2 points for 31?
//#
//#re: scoring sequence
//#Submitted by Joan (not verified) on Thu, 06/24/2010 - 17:44.
//#
//#Yes, your parntner gets 3 points for the run of 7, 8, 9, the last 3 cards
//played where the sequence was not broken. # And the 2 points for 31. If 8 or 9
//were played first your partner would be out of luck because the first 7 played
//# would have interrupted the second sequence.

TEST_F(PlayCardTest, T100_RunOf4) {
  std::vector<std::string> startstack = {"Ac", "2s", "3s"};
  std::string cardstr = "4h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN4]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T110_RunOf4OOO) {
  std::vector<std::string> startstack = {"8c", "6s", "5c"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN4]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T120_RunOf4intervening) {
  std::vector<std::string> startstack = {"Ac", "2s", "Jd", "3h"};
  std::string cardstr = "4c";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T130_RunOf5) {
  std::vector<std::string> startstack = {"2c", "3s", "4s", "5d"};
  std::string cardstr = "6h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN5]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T140_RunOf5OOO) {
  std::vector<std::string> startstack = {"8c", "6s", "5c", "4d"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN5]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T150_RunOf5intervening) {
  std::vector<std::string> startstack = {"Ac", "2s", "4c", "Jd", "3h"};
  std::string cardstr = "5c";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T160_RunOf6) {
  std::vector<std::string> startstack = {"2c", "3s", "4s", "5d", "6h"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN6]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T170_RunOf6OOO) {
  std::vector<std::string> startstack = {"3c", "6s", "5c", "4d", "2c"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN6]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T180_RunOf6intervening) {
  std::vector<std::string> startstack = {"Ac", "2s", "4c", "3d", "3h", "5d"};
  std::string cardstr = "6c";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T190_RunOf7) {
  std::vector<std::string> startstack = {"Ad", "Ah,", "2c", "3s",
                                         "4s", "5d",  "6d"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  plprintf("Cardstack len now %lu\n", cardstack.size());

  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN7]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T200_RunOf7OOO) {
  std::vector<std::string> startstack = {"7c", "6s", "5c", "4d", "2d", "3h"};
  std::string cardstr = "Ah";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  // this should be the slowest scoring hand for the play, yes?
  // hm, can't just do it ten million times, I guess - move this earlier? bc
  // cardstack changes so the time is a bit skewed - or maybe just pop that card
  // off cardstack & swh in any case, play scoring is pretty fast, like 3 sec for
  // 10MM in debug plprintf("TEN MILLION!!!\n"); for (auto j = 0; j < 10000000;
  // j++)
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // cardstack.pop_back();
  plprintf("Cardstack len now %lu\n", cardstack.size());
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_RUN7]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T210_RunOf7intervening) {
  std::vector<std::string> startstack = {"3c", "2s", "Ac", "4d", "5h", "6d"};
  std::string cardstr = "Ac";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, 0);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T220_15and3Kind) {
  std::vector<std::string> startstack = {"3d", "4c", "4s"};
  std::string cardstr = "4h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN] +
                           cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T230_31And4Kind) {
  std::vector<std::string> startstack = {"3c", "7s", "7c", "7d"};
  std::string cardstr = "7h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_THIRTYONE] +
                           cr.scorePoints[Cribbage::SCORE_4KIND]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}

TEST_F(PlayCardTest, T220_15andRun5) {
  std::vector<std::string> startstack = {"2d", "Ac", "5s", "3d"};
  std::string cardstr = "4h";
  build_stack(startstack);
  card = cu.stringcard(cardstr);
  index_t playscore;
  playscore = cr.play_card(cardstack, card, &scorelist, build_scorelists);
  // HEY PUT IN DETAILED CHECKS a la
  // self.assertEqual(resultcards,curcards + [newcard])
  // self.assertEqual(curtotal,sum([pyb.val(x) for x in curcards]) +
  // pyb.val(newcard)) self.assertEqual(scorelist,[(pyb.SCORE_RUN4,4)])
  EXPECT_EQ(playscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN] +
                           cr.scorePoints[Cribbage::SCORE_RUN5]);
  if (build_scorelists) render_play_scorelist(startstack, cardstr);
}
