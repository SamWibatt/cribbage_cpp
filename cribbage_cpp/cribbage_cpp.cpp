/**

cribbage_cpp - C++ implementation of cribbage utilities and scoring

Targeting embedded systems, so uses small containers like uint8_t at the risk of loss of speed on larger machines

*/

#include <string.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <stdio.h>
#include "cribbage_cpp.h"

namespace cribbage_cpp {

    //Random number stuff ----------------------------------------------------------------------------------------------

    // for the vpok random
    // VPOK VERSION:
    // ; implements the classic 32-bit linear congruential random number generator (LCRNG) where
    // ; Xn+1 =  (a Xn + c) mod m,     n > 0
    // ; in this case a = 1664525, m = 2^32, c = 1013904223, period = 2^32.
    uint32_t nextvpok = 1;
    void v_srandom(uint32_t n) { nextvpok = n; }
    uint32_t v_random() { nextvpok = (1664525ul * nextvpok) + 1013904223ul; return nextvpok; }


    // OLD VERSION:
    //this is copied from the arduino core random, I think, though maybe I should just write my own
    //was static, dunno if C++11 needs it in a namespace
    uint32_t next = 1;

    int32_t do_random(uint32_t *ctx)
    {
        // * Compute x = (7^5 * x) mod (2^31 - 1)
        // * wihout overflowing 31 bits:
        // *      (2^31 - 1) = 127773 * (7^5) + 2836
        // * From "Random number generators: good ones are hard to find",
        // * Park and Miller, Communications of the ACM, vol. 31, no. 10,
        // * October 1988, p. 1195.
        int32_t hi, lo, x;

        x = *ctx;
        // Can't be initialized with 0, so use another value.
        if (x == 0)
            x = 123459876L;
        hi = x / 127773L;
        lo = x % 127773L;
        x = 16807L * lo - 2836L * hi;
        if (x < 0)
            x += 0x7fffffffL;
        return ((*ctx = x) % ((uint32_t)RANDOM_MAX + 1));
    }

    int32_t my_random(void)
    {
        return do_random(&next);
    }

    void my_srandom(uint32_t seed)
    {
        next = seed;
    }

    // then this is from  @Ryan Reich:
    // at https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
    // Assumes 0 <= max <= RAND_MAX
    // Returns in the closed interval [0, max]
    int32_t random_at_most(int32_t max) {
        uint32_t
            // max <= RAND_MAX < ULONG_MAX, so this is okay.
            num_bins = (uint32_t) max + 1,
            num_rand = (uint32_t) RANDOM_MAX + 1,
            bin_size = num_rand / num_bins,
            defect   = num_rand % num_bins;
        int32_t x;
        do {
            x = my_random();
        }
        // This is carefully written not to overflow
        while (num_rand - defect <= (uint32_t)x);
        // Truncated division is intentional
        return x/bin_size;
    }

    // card related stuff ------------------------------------------------------------------------------------------------

    // these two functions are mostly for debugging and unit testing
    // card is a number from 0-51; card %4 is rank, where 0 = hearts, 1 = diamonds, 2 = clubs, 3 = spades.
    // card / 4 is rank, 0 = ace .. 12 = king
    const std::string rankstr = "A234567890JQK";
    const std::string suitstr = "hdcs";      //lower case to stand out from values better

    //given a card, return the string e.g. 0 => Ah = ace of hearts
    std::string cardstring(uint8_t card) {
        if(card > 51) return std::string("ERROR");
        std::string s(2,rankstr[rank(card)]);       // length 2 string, both chars are rank (dumb, but allocates and takes care of 1st char)
        s[1] = suitstr[suit(card)];
        return s;
    }

    // given a 2-character string, return corresponding card (or error if it doesn't conform to rank/suit possibilities
    uint8_t stringcard(std::string srcstr) {
        uint8_t card = 0;
        uint8_t i;
        for(i = 0; i < rankstr.size(); i++) {
            if (srcstr[0] == rankstr[i]) {
                card += i << 2;
                break;
            }
        }
        //if at this point is strlen(rankstr), the rank wasn't found
        if (i == rankstr.size()) { return ERROR_CARD_VAL; }

        //then account for the suit
        for(i = 0; i < suitstr.size(); i++) {
            if (srcstr[1] == suitstr[i]) {
                card += i;
                break;
            }
        }
        //if at this point is strlen(suitstr), the rank wasn't found
        if (i == suitstr.size()) { return ERROR_CARD_VAL; }
        return card;
    }

    //deck-level functions! =============================================================================================

    //support class for shuffling, see below
    class CardOrder {
        public:
            uint32_t order;         // random number by which deck is sorted, see shuffle below
            uint8_t card;           // card value 0..51
    };

    // shuffle turns the handed-in deck vector into a 52-card deck, randomized by means of constructing an array of CardOrder
    // objects s.t. their "order" member is random and the "card" member is consecutive, then sorting by "order." The resulting
    // deck is the card members copied onto the end of the deck.
    // deck is not assumed to have any particular capacity but for best results, reserve 52.
    void shuffle(std::vector<uint8_t> &deck) {
        std::array<CardOrder, 52> shufdeck;
        deck.clear();
        uint8_t j = 0;
        std::for_each(shufdeck.begin(), shufdeck.end(), [&j](CardOrder &c){ c.order = my_random(); c.card = j++; });
        std::sort(shufdeck.begin(), shufdeck.end(), [](CardOrder &a, CardOrder &b) { return a.order < b.order; });
        std::for_each(shufdeck.begin(), shufdeck.end(), [&j,&deck](CardOrder &c) {deck.push_back(c.card);});
    }


    //deck is a vector, deal off the end of the array for fastness
    uint8_t deal_card(std::vector<uint8_t> &deck) {
        if (deck.empty() || deck.size() > 52) {
            return ERROR_CARD_VAL;
        }
        auto card = deck.back();        // memorize the "rightmost" card
        deck.pop_back();                // and get rid of it, shrinking the deck
        return card;
    }

    //this is a "global" to support the cut function
    std::array<uint8_t,52> tempdeck;

    // kind of weird bc I think of "index" as being from the "left" (beginning of array) and the deck deals from the "right" (end of array)
    void cut(std::vector<uint8_t> &deck, uint8_t index) {
        if (deck.size() < 2 || index < 1 || index > deck.size()-1) {
            printf("Hey illegal deck or index\n");
            return;         //no effect if illegal index or degenerate deck
        }
        // so ok - is this compatible with the idea that we deal from the "end" of the deck for efficiency?
        // I think it is, so all we have to do is go back to using deck.size() - index instead of index
        auto numpop = deck.size() - index;
        std::copy_n(deck.begin(), numpop, tempdeck.begin());
        deck.erase(deck.begin(), deck.begin()+numpop);
        deck.insert(deck.end(),tempdeck.begin(), tempdeck.begin()+numpop);
    }
}
