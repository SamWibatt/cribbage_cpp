// let's try this with google test
// includes and such are in /usr/

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <stdio.h>
#include "cribbage_cpp.h"
#include "card_utils.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>

using namespace cardutils;
using namespace cribbage_cpp;

namespace {

    // Step 2. Use the TEST macro to define your tests.
    //
    // TEST has two parameters: the test case name and the test name.
    // After using the macro, you should define your test logic between a
    // pair of braces.  You can use a bunch of macros to indicate the
    // success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
    // examples of such macros.  For a complete list, see gtest.h.

    // TODO: init an array of these uint32s, srand with 9999, expect this sequence from my_random
    //
    // testing random first 10! seeded with 9999 ================================================================

    TEST(CardUtilsTest,VpokRandomFirst10From1337d00d) {
        //vpok random! First 10 are
        uint32_t vrandfirst[10] = { 0xC46EB208, 0xB3C52DC7, 0x661E907A, 0xB576E591, 0x3E1961BC,
            0x0C05D1EB, 0xAA513E4E, 0x57003155, 0x8C6652B0, 0x35C3464F };
        //which agree with my ancient PIC version!
        CardUtils c;
        c.v_srandom(0x1337d00d);
        for(auto j=0;j<10;j++) {
            EXPECT_EQ(c.v_random(),vrandfirst[j]);
        }
    }

    //let's do ten million random_at_mosts and see if any wander out of bounds
    //NOT CONCLUSIVE, just checks if the min and max are within the expected min-max range
    //very suggestive, though, if ten million >> expected max
    TEST(CardUtilsTest,RandomAtMost3333TenMReps) {
        uint32_t maxy = 0, minny = 0xFFFFFFFF, expmin = 0, expmax = 3333;
        CardUtils c;
        for (auto j = 0; j < 10000000; j++) {
            auto x = c.random_at_most(expmax);
            if (x < minny) minny = x;
            if (x > maxy) maxy = x;
        }
        //printf("Minny = %d, maxy = %d\n",minny, maxy);
        EXPECT_LE(maxy,expmax);
        EXPECT_GE(minny,expmin);
    }

    // card basics tests ========================================================================================

    //test that card -> string -> card works for all cards
    TEST(CardUtilsTest,StrcardCardstrAllLegit) {
        std::string cardstr;
        uint8_t outcard;
        CardUtils c;
        for(uint8_t card = 0; card < 52; card++) {
            cardstr = c.cardstring(card);
            outcard = c.stringcard(cardstr);
            //printf("%2d => %s => %2d\n",card,cardstr.c_str(),outcard);
            EXPECT_EQ(card,outcard);
        }
    }

    // shuffle test for sameness with python implementation
    TEST(CardUtilsTest,ShuffleTestFrom9999) {
        //generated by python shuffle after random seeded with 9999
        uint8_t cardorder[52] = { 15, 9, 49, 14, 22, 31, 16, 13, 0, 50, 8, 47, 28, 11, 35, 2, 1, 12, 45, 7, 21, 23, 6, 17, 34, 37,
            4, 41, 36, 40, 32, 38, 51, 19, 39, 46, 27, 10, 29, 26, 48, 33, 20, 30, 3, 44, 24, 42, 18, 25, 43, 5 };
        std::vector<uint8_t> deck(52);
        CardUtils c;
        c.v_srandom(9999);
        c.shuffle(deck);
        for(auto j = 0; j < 52; j++) EXPECT_EQ(deck[j],cardorder[j]);
        //however, we expect the DEAL to be from the right, so it'd go 9, 17, 7, 42, ...
        //in fact let's unit test that
    }

    TEST(CardUtilsTest,Deal10FromShuf9999) {
        uint8_t dealorder[10] = { 5, 43, 25, 18, 42, 24, 44, 3, 30, 20 };
        CardUtils c;
        std::vector<uint8_t> deck(52);
        c.v_srandom(9999);
        c.shuffle(deck);
        for(auto j = 0; j < 10; j++) {
            EXPECT_EQ(c.deal_card(deck),dealorder[j]);
            EXPECT_EQ(deck.size(),52-(j+1));            //clunky but sort of readable, deck shrinks by 1 per card dealt
        }
    }

    //test cut: 10 card deck from 10..19, cut at index 6
    // but... now we count from the right? so index 6 is really 6 from the end
    // python semantics are now consistent
    TEST(CardUtilsTest,DeckCutTest) {
        uint8_t postcut_deck[10] = {14, 15, 16, 17, 18, 19, 10, 11, 12, 13 };
        CardUtils c;
        std::vector<uint8_t> deck;
        deck.reserve(52);
        for(auto j=0;j<10;j++) deck.push_back(10+j);
        c.cut(deck,6);
        for(auto j=0;j<10;j++) EXPECT_EQ(deck[j],postcut_deck[j]);
   }



   // now for actual cribbage tests!
   // let's make a fixture, bc we will want a Cribbage for each as well as a hand and a starter and a score list

    class CribbageTest : public ::testing::Test {
        protected:

        // data members
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

        // setup initializes data members, runs BEFORE EVERY TEST
        void SetUp() override {
            cr.getCardUtils().v_srandom(default_seed);
        }

        // teardown cleans up after data members, runs AFTER EVERY TEST
        void TearDown() override {

        }

    };


    //all right! set up a hand that should have no scoring combinations in it and score it! Expect a score of 0
    TEST_F(CribbageTest,T000_Nothing) {
        build_hand("Qh", "0c", "9s", "3d","4d");
        //this should be the longest-running scoring case - let's try 10 million of them
        //20963 ms debug, 1115 ms release - not bad!
        uint8_t handscore;
        //for(auto j=0;j<10000000;j++)
            handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,0);
    }

    //all right! set up a hand that should have two two-card fifteens in it and score it! Expect a score of 4
    //LATER will check for detailed results
    TEST_F(CribbageTest,T010_TwoCardFifteen) {
        build_hand("Qh", "0c", "9s", "3d","5d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_FIFTEEN]*2);
    }

    //should have two 3-card fifteens! Expect a score of 4
    //LATER will check for detailed results
    TEST_F(CribbageTest,T020_ThreeCardFifteen) {
        build_hand("6h", "3c", "7s", "0d","2d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_FIFTEEN]*2);
    }

    //should have one 4-card fifteen in it! Expect a score of 2
    //LATER will check for detailed results
    TEST_F(CribbageTest,T030_FourCardFifteen) {
        build_hand("Ah", "4c", "3s", "7d","6d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_FIFTEEN]);
    }

    //should have one 5-card fifteen and a run of 5 in it! Expect a score of 7
    //LATER will check for detailed results
    TEST_F(CribbageTest,T040_FiveCardFifteen) {
        build_hand("Ah", "4c", "3s", "5d","2d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_FIFTEEN] + cr.scorePoints[Cribbage::SCORE_RUN5]);
    }

    //should have one pair in it! Expect a score of 2
    //LATER will check for detailed results
    TEST_F(CribbageTest,T050_OnePair) {
        build_hand("Ah", "2c", "6s", "0d","Ad");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_PAIR]);
    }

    //should have two pairs in it! Expect a score of 4
    //LATER will check for detailed results
    TEST_F(CribbageTest,T060_TwoPair) {
        build_hand("Ah", "0c", "6s", "0d","Ad");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_TWOPAIR]);
    }

    //should have a 3 of a kind in it! Expect a score of 6
    //LATER will check for detailed results
    // ... for how these are spotted, rank shouldn't matter, but still
    //3 of a kind is the lowest rank
    //Instead do tests re: where they are in the hand
    TEST_F(CribbageTest,T070_ThreeOfAKindLow) {
        build_hand("Ah", "Ac", "6s", "0d","Ad");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
    }

    //3 of a kind is the middle rank
    TEST_F(CribbageTest,T073_ThreeOfAKindMid) {
        build_hand("6s", "7c", "0h", "7h","7d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
    }

    //3 of a kind is the highest rank
    TEST_F(CribbageTest,T077_ThreeOfAKindHigh) {
        build_hand("0d", "6s", "Kc", "Kh","Kd");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_PAIRROYAL]);
    }


    //should have a 3 of a kind in it plus another pair! Expect a score of 8
    //LATER will check for detailed results
    TEST_F(CribbageTest,T080_ThreeOfAKindAndPairLow) {
        build_hand("Ah", "Ac", "4s", "4d","Ad");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_PAIRROYAL]+cr.scorePoints[Cribbage::SCORE_PAIR]);
    }

    //should have a 4 of a kind in it! Expect a score of 12
    //LATER will check for detailed results
    //this one tests if the 4 of a kind is the higher rank compared to the odd card out
    //because of how the 4s of a kind are spotted, with patterns, this needs to be tested as well as if
    //the 4 of a kind is the lower rank (see next test.)
    TEST_F(CribbageTest,T090_FourOfAKindHigh) {
        build_hand("4c", "Ac", "4s", "4h","4d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_4KIND]);
    }

    //lower rank compared to the odd card out
    TEST_F(CribbageTest,T095_FourOfAKindLow) {
        build_hand("4c", "Jc", "4s", "4h","4d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_4KIND]);
    }

    //should have a run of 3 in it! Expect a score of 3
    //LATER will check for detailed results
    //this one tests if the run of 3 is the first 3 cards
    //because of how the runs are spotted, with patterns, this needs to be tested as well as if
    //the run of 3 is the second or third triplet
    TEST_F(CribbageTest,T100_RunOfThreeLow) {
        build_hand("8c", "9c", "0s", "Qh","Kd");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_RUN3]);
    }

    TEST_F(CribbageTest,T103_RunOfThreeMid) {
        build_hand("Ac", "9c", "0s", "Jh","Kd");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_RUN3]);
    }

    TEST_F(CribbageTest,T107_RunOfThreeHigh) {
        build_hand("2c", "4c", "Js", "Qh","Kd");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_RUN3]);
    }

    //should have a run of 4 in it! Expect a score of 8 bc there are a couple of 15s in there too
    //LATER will check for detailed results
    //this one tests if the run of 4 is the first 4 cards
    //because of how the runs are spotted, with patterns, this needs to be tested as well as if
    //the run of 4 is the second set of 4
    TEST_F(CribbageTest,T110_RunOfFourLow) {
        build_hand("Ac", "3c", "4s", "Qh","2d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_RUN4] + (2* cr.scorePoints[Cribbage::SCORE_FIFTEEN]));
    }

    TEST_F(CribbageTest,T120_RunOfFourHigh) {
        build_hand("0c", "Jc", "Ks", "Qh","2d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_RUN4]);
    }

    //should have a run of 5 in it! Expect a score of 5
    //LATER will check for detailed results
    TEST_F(CribbageTest,T130_RunOfFive) {
        build_hand("9c", "Kc", "Js", "Qh","0d");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,cr.scorePoints[Cribbage::SCORE_RUN5]);
    }


    //# double run (3)
    //class aShowTest140_RunOf3Dbl(unittest.TestCase):
    //    def test_shewtest_run3dbl(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand and starter
    //        print("Show double run of 3 ---------------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['8c', '0c', '9s', 'Qh']]
    //        starter = pyb.stringcard('0d')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,8)
    //
    //# double run (4)
    //class aShowTest150_RunOf4Dbl(unittest.TestCase):
    //    def test_shewtest_run4dbl(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand and starter
    //        print("Show double run of 4 ---------------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['8c', '0c', '9s', 'jh']]
    //        starter = pyb.stringcard('0d')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,10)
    //
    //# double double run
    //class aShowTest160_RunOf3DblDbl(unittest.TestCase):
    //    def test_shewtest_run3dbldbl(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand and starter
    //        print("Show double double run of 3 --------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['8c', '0c', '9s', '9h']]
    //        starter = pyb.stringcard('0d')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,16)
    //
    //# triple run
    //class aShowTest170_RunOf3Triple(unittest.TestCase):
    //    def test_shewtest_run3triple(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand and starter
    //        print("Show triple run of 3 ---------------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['6h', '6c', '5s', '6d']]
    //        starter = pyb.stringcard('7c')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,15)
    //
    //# 4 card flush
    //class aShowTest180_4CardFlush(unittest.TestCase):
    //    def test_shewtest_4cardflush(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand, keep starter same
    //        print("Show 4 card flush ------------------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['4c', '3c', 'jc', '7c']]
    //        starter = pyb.stringcard('9h')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,4)
    //
    //# 5 card flush
    //class aShowTest190_5CardFlush(unittest.TestCase):
    //    def test_shewtest_5cardflush(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand and starter
    //        print("Show 5 card flush ------------------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['4c', '3c', 'qc', '7c']]
    //        starter = pyb.stringcard('9c')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,5)
    //
    //# not 4 card flush bc the 4 incl starter
    //class aShowTest200_Not4CardFlush(unittest.TestCase):
    //    def test_shewtest_not4cardflush(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand, keep starter same
    //        print("Show Not 4 card flush bc 4 cards incl starter --------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['4c', '3c', 'kc', '7h']]
    //        starter = pyb.stringcard('9c')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,0)
    //
    //# nobs
    //class aShowTest210_Nobs(unittest.TestCase):
    //    def test_shewtest_nobs(self):
    //        pyb = pybbage.Pybbage()
    //        # todo should work in all orderings of hand, jack must be in hand, starter w suit of j
    //        print("Show nobs --------------------------------------------------------------------------------------------")
    //        hand = [pyb.stringcard(x) for x in ['Qh', 'Jd', '9s', '3d']]
    //        starter = pyb.stringcard('4d')
    //        print("hand",[pyb.cardstring(x) for x in hand],"starter",pyb.cardstring(starter))
    //        (score,subsets) = pyb.score_shew(hand,starter)
    //        pyb.render_score_subsets(hand, starter, subsets)        # DEBUG
    //        self.assertEqual(score,1)
    //

    //should have a 29 hand in it! Expect a score of 29
    //LATER will check for detailed results
    TEST_F(CribbageTest,T220_Twentynine) {
        build_hand("5c", "5d", "Jh", "5s","5h");
        uint8_t handscore = cr.score_shew(hand,starter,&scorelist,false);
        EXPECT_EQ(handscore,(cr.scorePoints[Cribbage::SCORE_FIFTEEN] * 8) +
            cr.scorePoints[Cribbage::SCORE_4KIND] + cr.scorePoints[Cribbage::SCORE_NOBS]);
    }

}

int main(int argc, char *argv[]) {
    //silly noodles
    std::vector<uint8_t> hand;
    hand.push_back(31);
    hand.push_back(17);
    hand.push_back(41);
    hand.push_back(51);
    uint8_t starter = 44;
    printf("Hand: ");
    std::for_each(hand.begin(),hand.end(),[](uint8_t c){printf("%d ",c);});
    printf("Starter: %d\n",starter);
    std::array<uint8_t,5> whole_hand;
    std::array<uint8_t,5> whole_vals;
    std::array<uint8_t,5> sorthand_nranks;
    std::array<uint8_t,5> whole_suits;
    Cribbage c;
    //try a bunch of reps to see how long it takes - v. roughly 3 sec for ten million iterations, not great
    //BUT in the context of a giant AI search or whatever, not terrible; minimax is unlikely to be doing that many
    //at least if I limit the deepening
    //oh wait that was on debug, how about release? pretty much insty
    //printf("TEN MILLION!!!!\n");
    //for(auto j = 0; j < 10000000; j++)
        c.prep_score_hand(hand, starter, whole_hand, whole_vals, sorthand_nranks, whole_suits );
    printf("Whole Hand: ");
    std::for_each(whole_hand.begin(),whole_hand.end(),[](uint8_t c){printf("%d ",c);});
    printf("\nWhole Vals: ");
    std::for_each(whole_vals.begin(),whole_vals.end(),[](uint8_t c){printf("%d ",c);});
    printf("\nWhole Hand Suits: ");
    std::for_each(whole_suits.begin(),whole_suits.end(),[](uint8_t c){printf("%d ",c);});
    printf("\nSortHand NormRanks: ");
    std::for_each(sorthand_nranks.begin(),sorthand_nranks.end(),[](uint8_t c){printf("%d ",c);});
    printf("\n");

    //4-carders
    std::array<uint8_t,4> first4;
    std::array<uint8_t,4> last4;
    uint8_t firstlastrank = sorthand_nranks[1];     //first rank of last 4 cards
    std::copy_n(sorthand_nranks.begin(),4,first4.begin());
    std::transform(sorthand_nranks.begin() + 1,sorthand_nranks.end(),last4.begin(),
        [firstlastrank](uint8_t rank) -> uint8_t { return rank - firstlastrank; });
    printf("First 4 nranks: ");
    std::for_each(first4.begin(),first4.end(),[](uint8_t c){printf("%d ",c);});
    printf("\nLast 4 nranks, firstlastrank %d: ",firstlastrank);
    std::for_each(last4.begin(),last4.end(),[](uint8_t c){printf("%d ",c);});
    printf("\n");

    // real testing main starts here
    ::testing::InitGoogleTest();
    RUN_ALL_TESTS();
}
