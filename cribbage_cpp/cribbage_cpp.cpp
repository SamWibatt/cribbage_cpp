/**

cribbage_cpp - C++ implementation of cribbage utilities and scoring

Targeting embedded systems, so uses small containers like uint8_t at the risk of loss of speed on larger machines

*/

#include <string.h>
#include <array>
#include <algorithm>
#include <cstring>
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
    uint8_t tempdeck[52];

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

    // so for C++, what do we want to be able to do? If we're avoiding new and delete, hand in an array
    // presumably initialized to illegal card values, or really it doesn't matter, we're trampling it anyway
    void shuffle(uint8_t *deck) {
        // we need an "order" array of 52 random numbers - could try a C++11 array!
        //https://en.cppreference.com/w/cpp/container/array
        //agog to see if arduino likes it
        //or... could be an array of structures that have the random number and an index, then sort that by the order number,
        //then copy out the indices as the card values
        //can use std::sort https://en.cppreference.com/w/cpp/algorithm/sort
        // template< class RandomIt, class Compare > void sort( RandomIt first, RandomIt last, Compare comp );
        // sort using a lambda expression
        //std::sort(s.begin(), s.end(), [](int a, int b) {
        //  return a > b;
        //});
        //can also use stuff like ranged for loop!
        // need to test soon on ardy, dunno if it supports this fanciness - I guess I could just make a sketch that sorts
        // a bunch of numbers and returns that via serial()
        // Looks like the usual sort implementation for C++ 11 is introsort, which is probably big codewise...? Let's try it
        std::array<CardOrder, 52> tempdeck;
        // std::for_each(nums.begin(), nums.end(), [](int &n){ n++; });
        uint8_t j = 0;
        std::for_each(tempdeck.begin(), tempdeck.end(), [&j](CardOrder &c){ c.order = my_random(); c.card = j++; });
        std::sort(tempdeck.begin(), tempdeck.end(), [](CardOrder &a, CardOrder &b) {
            return a.order < b.order;           //I think this does ascending sort - doesn't really matter tho which we do
        });
        //old fashioned loop until I figure out how to do this better
        for(auto i=0; i < 52; i++) deck[i] = tempdeck[i].card;
    }


    //# deck is just an array now
    //# let's just have the deck be an array and pull cards off of its front
    //def deal_card(self,deck):
    //    if deck is not None and len(deck) > 0:
    //        card = deck[0]
    //        deck = deck[1:]
    //        return (deck,card)
    //    return (None,None)

    //OK so in C++ ... is deck a fixed-length array? Implemented that way, or can be; hand in a pointer and a length
    uint8_t deal_card(uint8_t *deck, uint8_t *pdecklen) {
        if (deck == nullptr || *pdecklen == 0 || *pdecklen > 52) {
            return ERROR_CARD_VAL;
        }
        auto card = deck[0];
        *pdecklen--;
        //std::memmove(deck,&deck[1],*pdecklen);     //swh, bet I can do better with an old fashioned for
        for(auto j=0; j < *pdecklen; j++) deck[j] = deck[j+1];
    }

    //# Cut will take an index into a deck which is assumed not to have any cards removed from it, plus an index.
    //# then it swaps the halves. returns the cut deck.
    //# weirdness is that cut (deck,0) returns deck unchanged. so, disallow 0?
    //# ektully from sec 3.1 of the rules,
    //# When cutting for the first deal of a game, the
    //# first player shall remove no less than four cards
    //# and not more than half the pack. The second
    //# player shall remove no less than four cards and
    //# shall leave at least four cards.
    //# c. When cutting before each deal and for the starter
    //# card, no less than four cards shall be taken from
    //# the top and no less than four left on the bottom.
    //# This will be enforced by the calls to the random number gettors below.
    //def cut(self,deck,index):
    //    if index >= 1 and index < len(deck):
    //        return deck[index:] + deck[:index]
    //    print("Illegal cut index",index,"- not doing cut")
    //    return deck;

    // C++ version will have pointer and length, operate in-place
    void cut(uint8_t *deck, uint8_t decklen, uint8_t index) {
        if (deck == nullptr || decklen < 2 || index < 1 || index > decklen-1) {
            return;         //no effect if illegal index or degenerate deck
        }
        //let's just do a quick C type swappy
        //is that possible? not really, since the sections of deck are typically not the same size
        // fastest is to have a separate array, do copy from index -> end to 0->(decklen-index)ish in dest, then 0..index-1
        // from src to (decklen-index)ish in dest.
        // then copy the whole thing back to deck.
        // cost of 52 bytes, which on anything bigger than a PIC is going to be nothing so don't worry about it
        // speed is also not super critical bc cut is a relatively uncommon op
        // I will make it a "global" to save some time, perhaps, or stack space, or whatever - tempdeck
        // this is a rich field for off-by-ones, so let's think about it
        // deck is A B C D E
        // index is 2
        // we want to end up with 
        // C D E A B
        // verify in python!
        // yup
        // >>> deck = ['A', 'B', 'C', 'D', 'E']
        // >>> index = 2
        // >>> deck[index:] + deck[:index]
        // ['C', 'D', 'E', 'A', 'B']
        // so decklen is 5, index = 2 - copy length of the first memcpy is then decklen-index: 5-2 = 3, CDE
        // second one then has to start at deck[decklen-index], and the number of items is index
        // verify on another:
        // >>> deck = [ 1,2,3,4,5,6 ]
        // >>> index = 2
        // >>> deck[index:] + deck[:index]
        // [3, 4, 5, 6, 1, 2]
        // looks right
        std::memcpy(tempdeck,&deck[index],decklen-index);
        std::memcpy(&tempdeck[decklen-index],deck,index);
        std::memcpy(deck,tempdeck,decklen);
    }
}
