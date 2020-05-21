#ifndef CRIBBAGE_CPP_H_INCLUDED
#define CRIBBAGE_CPP_H_INCLUDED

#include <cstdint>
#include "string.h"

namespace cribbage_cpp {

    //random number generator -----------------------------------------------------
    const uint32_t RANDOM_MAX (0x7FFFFFFF);
    int32_t do_random(uint32_t *ctx);
    int32_t my_random(void);
    void my_srandom(uint32_t seed);
    int32_t random_at_most(int32_t max);

    //card utilities --------------------------------------------------------------
    //value to return as a card if there is an error
    const uint8_t ERROR_CARD_VAL = 255;

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
        uint8_t i;
        for(i = 0; i < strlen(rankstr); i++) {
            if (srcstr[index] == rankstr[i]) {
                card += i << 2;
                break;
            }
        }
        //if at this point is strlen(rankstr), the rank wasn't found
        if (i == strlen(rankstr)) {
            //how are errors handled? Let's say return illegal card value, 0xFF is a good choice
            return ERROR_CARD_VAL;
        }
        //then account for the suit
        for(i = 0; i < strlen(suitstr); i++) {
            if (srcstr[index] == suitstr[i]) {
                card += i;
                break;
            }
        }
        //if at this point is strlen(suitstr), the rank wasn't found
        if (i == strlen(suitstr)) {
            //how are errors handled? Let's say return illegal card value, 0xFF is a good choice
            return ERROR_CARD_VAL;
        }
        return card;
    }

    void shuffle(uint8_t *deck);
    uint8_t deal_card(uint8_t **deck, uint8_t *pdecklen);
    void cut(uint8_t *deck, uint8_t decklen, uint8_t index);

}


#endif // CRIBBAGE_CPP_H_INCLUDED
