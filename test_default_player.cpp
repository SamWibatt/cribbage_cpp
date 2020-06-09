#include <stdio.h>
#include "cribbage_core.h"
#include "card_utils.h"
#include "cribbage_player.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include "plat_io.h"

using namespace cardutils;
using namespace cribbage_core;

//need some globals in order to use the classes - gross but there it is
extern CardUtils cu;
extern Cribbage cr;

//then a fixture for default player tests
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
    bool build_scorelists = true;      //FOR SPEED TESTING WITH OR WITHOUT BUILDING SCORELISTS

    //special helper function to create a hand - give it string reps of 5 cards
    //and the first 4 become "hand" global and last becomes starter
    void build_hand(std::string h1, std::string h2, std::string h3, std::string h4, std::string st) {
        hand.clear();
        hand.push_back(cu.stringcard(h1));
        hand.push_back(cu.stringcard(h2));
        hand.push_back(cu.stringcard(h3));
        hand.push_back(cu.stringcard(h4));
        starter = cu.stringcard(st);
        //check for illegal cards
        if(hand[0] == cu.ERROR_CARD_VAL) plprintf("WARNING: card 1 is illegal string '%s'\n",h1.c_str());
        if(hand[1] == cu.ERROR_CARD_VAL) plprintf("WARNING: card 2 is illegal string '%s'\n",h2.c_str());
        if(hand[2] == cu.ERROR_CARD_VAL) plprintf("WARNING: card 3 is illegal string '%s'\n",h3.c_str());
        if(hand[3] == cu.ERROR_CARD_VAL) plprintf("WARNING: card 4 is illegal string '%s'\n",h4.c_str());
        if(starter == cu.ERROR_CARD_VAL) plprintf("WARNING: starter is illegal string '%s'\n",st.c_str());
    }


    //special helper function to create a card stack - give it string reps cards
    //they become cardstack "global"
    void build_stack(std::vector<std::string> stackcardsstr) {
        cardstack.clear();
        for (auto j = 0; j < stackcardsstr.size(); j++)
            cardstack.push_back(cu.stringcard(stackcardsstr[j]));
        //check for illegal cards
        for(auto j = 0; j < cardstack.size(); j++)
            if(cardstack[j] == cu.ERROR_CARD_VAL) plprintf("WARNING: stack card %d is illegal string '%s'\n",j,stackcardsstr[j].c_str());
    }

    void render_shew_scorelist() {
        index_t mask;
        plprintf("score list: -----------------\n");
        for(auto j = 0; j < 4; j++) plprintf("%s ",cu.cardstring(hand[j]).c_str());
        plprintf("%s ",cu.cardstring(starter).c_str());
        plprintf(" hand\n");
        index_t totscore = 0;
        for (Cribbage::score_entry se : shew_scorelist) {
            for(auto j = 0,mask = 0x01; j < 5; j++, mask <<= 1)
                if(se.part_cards & mask)
                    if(j<4)
                        plprintf("%s ",cu.cardstring(hand[j]).c_str());
                    else
                        plprintf("%s ",cu.cardstring(starter).c_str());
                else plprintf("-- ");
            totscore += cr.scorePoints[se.score_index];
            plprintf(" %s %d (%d)\n",cr.scoreStrings[se.score_index].c_str(),cr.scorePoints[se.score_index],totscore);
        }
    }

    //cardstack should have the played card on it
    void render_play_scorelist(std::vector<std::string> stackcardsstr, std::string cardstr) {
        index_t mask;
        plprintf("score list: -----------------\n");
        if(stackcardsstr.empty())
            plprintf("empty ");
        else
            for(index_t j = 0; j< stackcardsstr.size(); j++) plprintf("%s ",stackcardsstr[j].c_str());
        //plprintf("%s ",cu.cardstring(starter).c_str());
        plprintf("stack, played %s\n",cardstr.c_str());
        index_t totscore = 0;
        for (Cribbage::score_entry se : play_scorelist) {
            //remember the early cards are rightmost
            for(index_t j = 0,mask = 1 << cardstack.size(); j < cardstack.size(); j++, mask >>= 1)
                if(se.part_cards & mask)
                    plprintf("%s ",cu.cardstring(cardstack[j]).c_str());
                else plprintf("-- ");
            totscore += cr.scorePoints[se.score_index];
            plprintf(" %s %d (%d)\n",cr.scoreStrings[se.score_index].c_str(),cr.scorePoints[se.score_index],totscore);
        }
    }

    // setup initializes data members, runs BEFORE EVERY TEST
    void SetUp() override {
        cu.v_srandom(default_seed);
        //I think the longest possible list of scores is for 29: eight fifteens + 4 of a kind + nobs?
        //VERIFY
        shew_scorelist.reserve(10);
        // there are really only 3 possible scores in play - 15, pairs & variants, and runs, but reserve 10 why not
        play_scorelist.reserve(10);
    }

    // teardown cleans up after data members, runs AFTER EVERY TEST
    void TearDown() override {

    }
};

TEST_F(DefaultPlayerTest,T0_Setup) {
    CribbagePlayer cp;
    EXPECT_EQ(cp.is_dealer(),false);
    EXPECT_EQ(cp.get_name(),"");
    EXPECT_EQ(cp.get_score(),0);
    EXPECT_EQ(cp.get_cards().capacity(),6);
    EXPECT_EQ(cp.get_used_cards().capacity(),4);
    EXPECT_EQ(cp.get_crib().capacity(),4);
    EXPECT_EQ(cp.get_cards().empty(), true);
    EXPECT_EQ(cp.get_used_cards().empty(), true);
    EXPECT_EQ(cp.get_crib().empty(), true);
}
