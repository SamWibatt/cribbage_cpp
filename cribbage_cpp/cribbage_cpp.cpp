/**

cribbage_cpp - C++ implementation of cribbage utilities and scoring

Targeting embedded systems, so uses small containers like uint8_t at the risk of loss of speed on larger machines

*/

#include <string.h>
#include <array>
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
    // REIMPLEMENT ALL THIS PYTHON!
    //# card handling routines -------------------------------------------------------------------------
    //
    // So for greatest speed, inline the tiny stuff? This is the sort of thing I'd think could be done with a macro
    // in the old days. Indeed, this seems to be the preferred way; also sounds like class member functions are implicitly
    // inline.
    //def rank(self,card):
    //    return card // 4
    inline uint8_t rank(uint8_t card) {
        return card >> 2;
    }

    //def val(self,card):
    //    '''value, s.t. ace = 1, 2 = 2, ... 10 and face cards = 10'''
    //    return self.rank(card)+1 if self.rank(card) < 10 else 10
    inline uint8_t val(uint8_t card) {
        return (rank(card)<10) ? rank(card) : 10;
    }

    //def suit(self,card):
    //    return card % 4
    inline uint8_t suit(uint8_t card) {
        return card & 3;            //bc bitwise operators are more rad and might work better on tiny chips
    }

    //def cardstring(self,card):
    //    '''card is a number from 0-51; card %4 is rank, where 0 = hearts, 1 = diamonds, 2 = clubs, 3 = spades.
    //    card // 4 is rank, 0 = ace .. 12 = king
    //    '''
    //    if card not in range(0,52):
    //        print("Illegal card value",card)
    //        return None
    //    return 'A234567890JQK'[self.rank(card)] + '♥♦♣♠'[self.suit(card)]
    // CHANGING TO JUST USE HDCS instead of card suit characters bc unicode in C++ is still a PIA
    //quick noodle with unicode string
    //auto suitstr = u8"\u2660\u2665\u2663\u2666";
    //printing as string works, char doesn't
    //printf("Suits: %s, or %c %c %c %c\n",suitstr,suitstr[0],suitstr[1],suitstr[2],suitstr[3]);
    //so... cardstr and strcard don't need to be super high performance, yes? They're for human
    //convenience. So let's have them compile out if ... hm.
    //they're important for unit testing
    //ARDUINO CAN'T DO NEW OR DELETE!
    //Could have it so that it takes a char * in and plunks down the characters at a specified index
    //let's go with that, and see how it works
    const char rankstr[] = "A234567890JQK";
    const char suitstr[] = "hdcs";      //lower case to stand out from values better

    inline void cardstring(uint8_t card, char *deststr, uint8_t index) {
        deststr[index] = rankstr[rank(card)];
        deststr[index+1] = suitstr[suit(card)];
    }

    //# this is mostly for debugging and unit testing
    //def stringcard(self,strc):
    //    '''strc is a 2 character code for a card. first character = rank, A234567890JQK, 2nd = suit, hdcs for heart diamond
    //    club spade, case-insensitive, can also be ♥♦♣♠ '''
    //    if strc is None:
    //        return None
    //    ranks = 'A234567890JQK'
    //    suits = 'HDCS'
    //    suits2 = '♥♦♣♠'
    //    if len(strc) != 2:
    //        print("ERROR: stringcard input must be 2 characters")
    //        return None
    //
    //    stru = str.upper(strc)
    //    if stru[0] not in ranks:
    //        print("ERROR: rank",stru[0],"is not a legal rank from",ranks)
    //        return None
    //    if stru[1] not in suits and stru[1] not in suits2:
    //        print("ERROR: suit",stru[1],"is not a legal suit from",suits,"or",suits2)
    //        return None
    //
    //    return (ranks.index(stru[0]) * 4) + (suits.index(stru[1]) if stru[1] in suits else suits2.index(stru[1]))
    //
    // FOR C++ version let's assume ranks and suits are in the appropriate case and suits are lowercase letters as in
    // cardstring - or no, that's annoying
    // and do it the same way that it picks two characters out of a provided string
    inline uint8_t stringcard(char *srcstr, uint8_t index) {
        uint8_t card = 0;
        for(uint8_t i = 0; i < strlen(rankstr); i++) {
            if (srcstr[index] == rankstr[i) {
                card += i << 2; 
                break;
            }
        }
        //if at this point is strlen(rankstr), the rank wasn't found
        if (i == strlen(rankstr)) {
            //how are errors handled? Let's say return illegal card value, 0xFF is a good choice
            return 0xFF;
        }
        //then account for the suit
        for(uint8_t i = 0; i < strlen(suitstr); i++) {
            if (srcstr[index] == suitstr[i) {
                card += i; 
                break;
            }
        }
        //if at this point is strlen(suitstr), the rank wasn't found
        if (i == strlen(suitstr)) {
            //how are errors handled? Let's say return illegal card value, 0xFF is a good choice
            return 0xFF;
        }
        return card;
    }


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
        uint32_t order;         // random number by which deck is sorted, see shuffle below
        uint8_t card;           // card value 0..51
    };
                                         
    // so for C++, what do we want to be able to do? If we're avoiding new and delete, hand in an array
    // presumably initialized to illegal card values, or really it doesn't matter, we're trampling it anyway
    void shuffle(char *deck) {
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
    //
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

}
