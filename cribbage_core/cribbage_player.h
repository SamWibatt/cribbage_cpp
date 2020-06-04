#ifndef CRIBBAGE_PLAYER_H_INCLUDED
#define CRIBBAGE_PLAYER_H_INCLUDED

#include "card_utils.h"
#include "cribbage_core.h"

#include <vector>

using namespace cardutils;
using namespace cribbage_core;

class CribbagePlayer {

    //data members =======================================================================================================
    protected:
    //def __init__(self, parent, cards = [], used_cards = [], crib = [], dealer = False, score = 0, name = "Player"):
    //    self.parent = parent
    //    self.cards = cards
    //    self.used_cards = used_cards
    //    self.crib = crib
    //    self.dealer = dealer
    //    self.score = score
    //    self.name = name
    // so - for C++ version need to think over what a player should really have. Most of this is good, I think.
    // try to refactor away whatever "parent" was needed for
    // "name" might also not be necessary in arduino version - but keep it, it's a tiny memory use

    bool dealer;
    std::vector<card_t> cards;
    std::vector<card_t> used_cards;
    std::vector<card_t> crib;
    index_t score;
    std::string name;


    public:
        CribbagePlayer();
        ~CribbagePlayer();
};

#endif // CRIBBAGE_PLAYER_H_INCLUDED
