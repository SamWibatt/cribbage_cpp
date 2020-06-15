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

// now for actual cribbage tests! Here, scoring shew.
// let's make a fixture, bc we will want a Cribbage for each as well as a hand
// and a starter and a score list
class ScoreShowTest : public ::testing::Test {
 protected:
  // data members
  uint32_t default_seed = 0x1337d00d;
  std::vector<card_t> hand;
  card_t starter;
  std::vector<Cribbage::score_entry> scorelist;
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

  void render_shew_scorelist() {
    index_t mask;
    plprintf("score list: -----------------\n");
    for (auto j = 0; j < 4; j++)
      plprintf("%s ", cu.cardstring(hand[j]).c_str());
    plprintf("%s ", cu.cardstring(starter).c_str());
    plprintf(" hand\n");
    index_t totscore = 0;
    for (Cribbage::score_entry se : scorelist) {
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

// all right! set up a hand that should have no scoring combinations in it and
// score it! Expect a score of 0
TEST_F(ScoreShowTest, T000_Nothing) {
  build_hand("Qh", "0c", "9s", "3d", "4d");
  // this should be the longest-running scoring case - let's try 10 million of
  // them 20963 ms debug, 1115 ms release - not bad!
  index_t handscore;
  // for(auto j=0;j<10000000;j++)
  handscore = cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, 0);
  if (build_scorelists) render_shew_scorelist();
}

// all right! set up a hand that should have two two-card fifteens in it and
// score it! Expect a score of 4 LATER will check for detailed results
TEST_F(ScoreShowTest, T010_TwoCardFifteen) {
  build_hand("Qh", "0c", "9s", "3d", "5d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 2);
  if (build_scorelists) render_shew_scorelist();
}

// should have two 3-card fifteens! Expect a score of 4
// LATER will check for detailed results
TEST_F(ScoreShowTest, T020_ThreeCardFifteen) {
  build_hand("6h", "3c", "7s", "0d", "2d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 2);
  if (build_scorelists) render_shew_scorelist();
}

// should have one 4-card fifteen in it! Expect a score of 2
// LATER will check for detailed results
TEST_F(ScoreShowTest, T030_FourCardFifteen) {
  build_hand("Ah", "4c", "3s", "7d", "6d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN]);
  if (build_scorelists) render_shew_scorelist();
}

// should have one 5-card fifteen and a run of 5 in it! Expect a score of 7
// LATER will check for detailed results
TEST_F(ScoreShowTest, T040_FiveCardFifteen) {
  build_hand("Ah", "4c", "3s", "5d", "2d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_FIFTEEN] +
                           cr.scorePoints[Cribbage::SCORE_RUN5]);
  if (build_scorelists) render_shew_scorelist();
}

// should have one pair in it! Expect a score of 2
// LATER will check for detailed results
TEST_F(ScoreShowTest, T050_OnePair) {
  build_hand("Ah", "2c", "6s", "0d", "Ad");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_PAIR]);
  if (build_scorelists) render_shew_scorelist();
}

// should have two pairs in it! Expect a score of 4
// LATER will check for detailed results
TEST_F(ScoreShowTest, T060_TwoPair) {
  build_hand("Ah", "0c", "6s", "0d", "Ad");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_TWOPAIR]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a 3 of a kind in it! Expect a score of 6
// LATER will check for detailed results
// ... for how these are spotted, rank shouldn't matter, but still
// 3 of a kind is the lowest rank
// Instead do tests re: where they are in the hand
TEST_F(ScoreShowTest, T070_ThreeOfAKindLow) {
  build_hand("Ah", "Ac", "6s", "0d", "Ad");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
  if (build_scorelists) render_shew_scorelist();
}

// 3 of a kind is the middle rank
TEST_F(ScoreShowTest, T073_ThreeOfAKindMid) {
  build_hand("6s", "7c", "0h", "7h", "7d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
  if (build_scorelists) render_shew_scorelist();
}

// 3 of a kind is the highest rank
TEST_F(ScoreShowTest, T077_ThreeOfAKindHigh) {
  build_hand("0d", "6s", "Kc", "Kh", "Kd");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a 3 of a kind in it plus another pair! Expect a score of 8
// LATER will check for detailed results
TEST_F(ScoreShowTest, T080_ThreeOfAKindAndPairLow) {
  build_hand("Ah", "Ac", "4s", "4d", "Ad");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_PAIRROYAL] +
                           cr.scorePoints[Cribbage::SCORE_PAIR]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T085_ThreeOfAKindAndPairHigh) {
  build_hand("Kh", "Kc", "4s", "4d", "Kd");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_PAIRROYAL] +
                           cr.scorePoints[Cribbage::SCORE_PAIR]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a 4 of a kind in it! Expect a score of 12
// LATER will check for detailed results
// this one tests if the 4 of a kind is the higher rank compared to the odd card
// out because of how the 4s of a kind are spotted, with patterns, this needs to
// be tested as well as if the 4 of a kind is the lower rank (see next test.)
TEST_F(ScoreShowTest, T090_FourOfAKindHigh) {
  build_hand("4c", "Ac", "4s", "4h", "4d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_4KIND]);
  if (build_scorelists) render_shew_scorelist();
}

// lower rank compared to the odd card out
TEST_F(ScoreShowTest, T095_FourOfAKindLow) {
  build_hand("4c", "Jc", "4s", "4h", "4d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_4KIND]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a run of 3 in it! Expect a score of 3
// LATER will check for detailed results
// this one tests if the run of 3 is the first 3 cards
// because of how the runs are spotted, with patterns, this needs to be tested
// as well as if the run of 3 is the second or third triplet
TEST_F(ScoreShowTest, T100_RunOfThreeLow) {
  build_hand("8c", "9c", "0s", "Qh", "Kd");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_RUN3]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T103_RunOfThreeMid) {
  build_hand("Ac", "9c", "0s", "Jh", "Kd");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_RUN3]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T107_RunOfThreeHigh) {
  build_hand("2c", "4c", "Js", "Qh", "Kd");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_RUN3]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a run of 4 in it! Expect a score of 8 bc there are a couple of
// 15s in there too LATER will check for detailed results this one tests if the
// run of 4 is the first 4 cards because of how the runs are spotted, with
// patterns, this needs to be tested as well as if the run of 4 is the second set
// of 4
TEST_F(ScoreShowTest, T110_RunOfFourLow) {
  build_hand("Ac", "3c", "4s", "Qh", "2d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_RUN4] +
                           (2 * cr.scorePoints[Cribbage::SCORE_FIFTEEN]));
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T120_RunOfFourHigh) {
  build_hand("0c", "Jc", "Ks", "Qh", "2d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_RUN4]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a run of 5 in it! Expect a score of 5
// LATER will check for detailed results
TEST_F(ScoreShowTest, T130_RunOfFive) {
  build_hand("9c", "Kc", "Js", "Qh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_RUN5]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a double run of 3 in it! Expect a score of 8
// this one checks for double run of 3 where the pair is the lowest rank
// LATER will check for detailed results
TEST_F(ScoreShowTest, T140_DblRunThreeLow) {
  build_hand("8c", "8c", "9s", "Qh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN3]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T143_DblRunThreeMid) {
  build_hand("8c", "9c", "9s", "Qh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN3]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T147_DblRunThreeHigh) {
  build_hand("8c", "0c", "9s", "Qh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN3]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a double run of 3 in it! Expect a score of 10
// this one checks for double run of 4 where the pair is the lowest rank
// LATER will check for detailed results
TEST_F(ScoreShowTest, T150_DblRunFourLowest) {
  build_hand("8c", "8s", "9s", "Jh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN4]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T153_DblRunFourLowmid) {
  build_hand("8c", "9c", "9s", "Jh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN4]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T155_DblRunFourHighmid) {
  build_hand("8c", "0c", "9s", "Jh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN4]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T157_DblRunFourHigh) {
  build_hand("8c", "Jc", "9s", "Jh", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLRUN4]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a double double run of 3 in it! Expect a score of 16
// this one checks for double double run of 3 where the pairs are the lowest
// rank and highest LATER will check for detailed results
TEST_F(ScoreShowTest, T160_DblDblRunLowHigh) {
  build_hand("8c", "8s", "0s", "0h", "9d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLDBLRUN]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T163_DblDblRunLowMid) {
  build_hand("8c", "9d", "9s", "0h", "8d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLDBLRUN]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T160_DblDblRunMidHigh) {
  build_hand("8c", "9h", "9s", "0h", "0d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_DBLDBLRUN]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a triple run of 3 in it! Expect a score of 15
// this one checks for triple run of 3 where the 3 of a kind is lowest rank
// LATER will check for detailed results
TEST_F(ScoreShowTest, T170_TripleRunLow) {
  build_hand("9h", "9c", "0s", "Js", "9d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_TRIPLERUN]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T173_TripleRunMid) {
  build_hand("0h", "0c", "9s", "0d", "Jc");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_TRIPLERUN]);
  if (build_scorelists) render_shew_scorelist();
}

TEST_F(ScoreShowTest, T177_TripleRunHigh) {
  build_hand("6h", "7h", "5s", "7d", "7c");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_TRIPLERUN]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a 4 card flush in it! Expect a score of 4
// LATER will check for detailed results
TEST_F(ScoreShowTest, T180_FourCardFlush) {
  build_hand("4c", "3c", "Jc", "7c", "9h");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_FLUSH]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a 5 card flush in it! Expect a score of 5
// LATER will check for detailed results
TEST_F(ScoreShowTest, T190_FiveCardFlush) {
  build_hand("4c", "3c", "Qc", "7c", "9c");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_FLUSH5]);
  if (build_scorelists) render_shew_scorelist();
}

// should NOT have a 4 card flush in it! bc the 4 cards include starter Expect a
// score of 0 LATER will check for detailed results
TEST_F(ScoreShowTest, T200_NotFourCardFlush) {
  build_hand("4c", "3c", "Kc", "7h", "9c");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, 0);
  if (build_scorelists) render_shew_scorelist();
}

// should have nobs in it! Expect a score of 1
// LATER will check for detailed results
TEST_F(ScoreShowTest, T210_Nobs) {
  build_hand("Qh", "Jd", "9s", "3d", "4d");
  index_t handscore =
      cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, cr.scorePoints[Cribbage::SCORE_NOBS]);
  if (build_scorelists) render_shew_scorelist();
}

// should have a 29 hand in it! Expect a score of 29
// LATER will check for detailed results
TEST_F(ScoreShowTest, T220_Twentynine) {
  build_hand("5c", "5d", "Jh", "5s", "5h");
  index_t handscore;
  // I think this might be the slowest case for scoring, so let's try 10 million
  // of it actually no 4 of a kind is a short-circuit, try one with 5 5 5 j j
  // incl nobs plprintf("TEN MILLION!!!\n"); for (auto j = 0; j < 10000000; j++)
  handscore = cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, (cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 8) +
                           cr.scorePoints[Cribbage::SCORE_4KIND] +
                           cr.scorePoints[Cribbage::SCORE_NOBS]);
  if (build_scorelists) render_shew_scorelist();
}

//for coverage - do a test with build list forced false
// should have a 29 hand in it! Expect a score of 29
// LATER will check for detailed results
TEST_F(ScoreShowTest, T223_TwentynineNoList) {
  build_hand("5c", "5d", "Jh", "5s", "5h");
  index_t handscore;
  // I think this might be the slowest case for scoring, so let's try 10 million
  // of it actually no 4 of a kind is a short-circuit, try one with 5 5 5 j j
  // incl nobs plprintf("TEN MILLION!!!\n"); for (auto j = 0; j < 10000000; j++)
  handscore = cr.score_shew(hand, starter, &scorelist, false);
  EXPECT_EQ(handscore, (cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 8) +
                           cr.scorePoints[Cribbage::SCORE_4KIND] +
                           cr.scorePoints[Cribbage::SCORE_NOBS]);
}

//for coverage - do a test with build list forced true
// should have a 29 hand in it! Expect a score of 29
// LATER will check for detailed results
TEST_F(ScoreShowTest, T227_TwentynineWithList) {
  build_hand("5c", "5d", "Jh", "5s", "5h");
  index_t handscore;
  // I think this might be the slowest case for scoring, so let's try 10 million
  // of it actually no 4 of a kind is a short-circuit, try one with 5 5 5 j j
  // incl nobs plprintf("TEN MILLION!!!\n"); for (auto j = 0; j < 10000000; j++)
  handscore = cr.score_shew(hand, starter, &scorelist, true);
  EXPECT_EQ(handscore, (cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 8) +
                           cr.scorePoints[Cribbage::SCORE_4KIND] +
                           cr.scorePoints[Cribbage::SCORE_NOBS]);
  render_shew_scorelist();
}

// Here trying to find the worst case hand for scoring time. Current guess, 5 5
// 5 j j with a nobs Expect a score of 15-14 + 3 of a kind + pair + nobs = 23
// wow, 20.1 seconds debug, but only 900 ms release!
// if clock speed were the only diffie bt takkun and samd21, ~52 times slower =
// a long time in debug and ~47 sec release, not horrible considering LATER will
// check for detailed results
TEST_F(ScoreShowTest, T230_SlowestScore) {
  build_hand("5c", "5d", "Jh", "Js", "5h");
  index_t handscore;
  // I think this might be the slowest case for scoring, so let's try 10 million
  // of it actually no 4 of a kind is a short-circuit, try one with 5 5 5 j j
  // incl nobs 
  //plprintf("TEN MILLION!!!\n"); 
  //for (auto j = 0; j < 10000000; j++)
  handscore = cr.score_shew(hand, starter, &scorelist, build_scorelists);
  EXPECT_EQ(handscore, (cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 7) +
                           cr.scorePoints[Cribbage::SCORE_PAIRROYAL] +
                           cr.scorePoints[Cribbage::SCORE_PAIR] +
                           cr.scorePoints[Cribbage::SCORE_NOBS]);
  if (build_scorelists) render_shew_scorelist();
}
