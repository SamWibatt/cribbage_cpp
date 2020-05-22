/**

cribbage_cpp - C++ implementation of cribbage utilities and scoring

Targeting embedded systems, so uses small containers like uint8_t at the risk of loss of speed on larger machines

*/

#include <string.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include "cribbage_cpp.h"

namespace cribbage_cpp {

    //Random number stuff ----------------------------------------------------------------------------------------------

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
    //this is a "global" to support the cut function
    std::array<uint8_t,52> tempdeck;

    // inlines moved to header

    //# shuffle returns a data structure containing parallel lists of card value (rank/suit combo 0..51) and flag
    //# for whether it's been dealt.
    //# this representation lets you walk through the deck sequentially (deal) but also fish cards out random-access (cut).
    //# only... that's not really what cut is, is it? it takes a position in the deck and swaps the 'halves' of the deck!
    //# that being the case, do we need the dealt-flag? the cribbage cut and turn is just swap halves, then deal a card.
    //# shuffle implemented by generating 52 32-bit random numbers and scanning it to determine their order.
    //# the LOL here is that this takes 208 bytes and I couldn't use it for vpok bc that's more RAM than a PIC 16F628 has.
    //# on ardy it can be discarded once the ordering is redone.
    //# Uno R3 / atmega328 has 2K ram, yes? Some taken by the arduino core but not lots
    //def shuffle(self):
    //    newdeck = {'order':[self.random() for i in range(0,52)],
    //            'value':[-1 for i in range(0,52)]}
    //    curmin = min(newdeck['order'])
    //    # the arduino version will look quite different, searching instead of listbuilding
    //    for val in range(0,52):
    //        card = newdeck['order'].index(curmin)
    //        newdeck['value'][card] = val
    //        gtmin = list(filter(lambda x:x>curmin,newdeck['order']))
    //        if len(gtmin) > 0:
    //            curmin = min(gtmin)
    //    return newdeck['value']
    //
    //# COULD ALSO TRY THE SHUFFLE WAY WHERE YOU JUST PICK TWO CARDS TO SWAP AND DO THAT A BUNCH OF TIMES.
    //# THAT'S MORE THE TINY861 VERSION - how many times is enough, etc.
    //# worry re later

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
        std::array<CardOrder, 52> tempdeck;
        deck.clear();
        uint8_t j = 0;
        std::for_each(tempdeck.begin(), tempdeck.end(), [&j](CardOrder &c){ c.order = my_random(); c.card = j++; });
        std::sort(tempdeck.begin(), tempdeck.end(), [](CardOrder &a, CardOrder &b) { return a.order < b.order; });
        std::for_each(tempdeck.begin(), tempdeck.end(), [&j,&deck](CardOrder &c) {deck.push_back(c.card);});
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
