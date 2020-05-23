/**

cribbage_cpp - C++ implementation of cribbage utilities and scoring

Targeting embedded systems, so uses small containers like uint8_t at the risk of loss of speed on larger machines

*/


#include "cribbage_cpp.h"

namespace cribbage_cpp {

    Cribbage::Cribbage() {
        //ctor stuff here
    }

    Cribbage::~Cribbage() {
        //dtor stuff here
    }

    void Cribbage::prep_score_hand(std::vector<uint8_t> &hand, uint8_t starter,
        std::array<uint8_t,5> &whole_hand, std::array<uint8_t,5> &whole_vals,
        std::array<uint8_t,5> &sorthand_ranks, std::array<uint8_t,5> &sorthand_suits ) {

        // get the whole_hand array going - copy hand over, then drop starter at end
        std::copy_n(hand.begin(), 4, whole_hand.begin());
        whole_hand[4] = starter;

        // then construct the vals array from that
        std::transform(whole_hand.begin(),whole_hand.end(),whole_vals.begin(),
            [this](uint8_t card) -> uint8_t { return this->cu.val(card); });

        // then construct the sorted hand in sorthand_ranks - will boil it down to ranks after extracting suits
        std::copy(whole_hand.begin(),whole_hand.end(),sorthand_ranks.begin());
        std::sort(sorthand_ranks.begin(),sorthand_ranks.end());

        //extract suits
        std::transform(sorthand_ranks.begin(),sorthand_ranks.end(),sorthand_suits.begin(),
            [this](uint8_t card) -> uint8_t { return this->cu.suit(card); });

        //finally, boil sorted_ranks down to its ranks
        std::transform(sorthand_ranks.begin(),sorthand_ranks.end(),sorthand_ranks.begin(),
            [this](uint8_t card) -> uint8_t { return this->cu.rank(card); });
    }


    uint8_t Cribbage::score_shew(std::vector<uint8_t> hand, uint8_t starter, std::vector<score_entry> &scores) {
        return 0;       //STUB!!!!!!!
    }

    // also scores hand + starter, but optimized for speed and doesn't worry about how scores would be read out
    uint8_t Cribbage::score_shew_fast(std::vector<uint8_t> hand, uint8_t starter) {
        return 0;       //STUB!!!!!!!
    }

}
