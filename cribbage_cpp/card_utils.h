/*

card_utils.h - class definition for card game utilities - individual cards and decks

*/

#ifndef CARD_UTILS_H_INCLUDED
#define CARD_UTILS_H_INCLUDED

#include <string>
#include <vector>

namespace cardutils {

    class CardUtils {
        public:
            //random number generator -----------------------------------------------------
            // VPOK VERSION:
            // ; implements the classic 32-bit linear congruential random number generator (LCRNG) where
            // ; Xn+1 =  (a Xn + c) mod m,     n > 0
            // ; in this case a = 1664525, m = 2^32, c = 1013904223, period = 2^32.
            //was static, dunno if C++11 needs it in a namespace
            // for the vpok random
            void v_srandom(uint32_t n);
            uint32_t v_random();

            // generating number evenly distributed bt 0 and max
            // limiting max to RANDOM_MAX which, +1, must fit in the container uint32_t
            // we're likely to be using it for tiny numbers but allow for up to 2 billion+, why not
            // returns RANDOM_ERROR on illegal input
            const uint32_t RANDOM_MAX = 0x7FFFFFFF;
            const uint32_t RANDOM_ERROR = 0xFFFFFFFF;
            uint32_t random_at_most(uint32_t max);

            /*
            // from python
            const uint32_t RANDOM_MAX (0x7FFFFFFF);
            int32_t do_random(uint32_t *ctx);
            int32_t my_random(void);
            void my_srandom(uint32_t seed);
            int32_t random_at_most(int32_t max);
            */

            //card utilities --------------------------------------------------------------
            //value to return as a card if there is an error
            const uint8_t ERROR_CARD_VAL = 255;

            // REIMPLEMENT ALL THIS PYTHON!
            //# card handling routines -------------------------------------------------------------------------

            //cards are encoded as (rank * 4) + suit, where rank 0..12 = A234567890JQK and
            //suit 0..3 is heard, diamond, club, spade
            inline uint8_t rank(uint8_t card) { return card >> 2; }

            // card value, s.t. ace = 1, 2 = 2, ... 10 and face cards = 10
            inline uint8_t val(uint8_t card) { return (rank(card)<10) ? rank(card) : 10; }

            inline uint8_t suit(uint8_t card) { return card & 3; }

            // string versions are mostly for debug/test unless there's a text version of the game
            std::string cardstring(uint8_t card);
            uint8_t stringcard(std::string srcstr);

            // deck-level functions
            void shuffle(std::vector<uint8_t> &deck);
            uint8_t deal_card(std::vector<uint8_t> &deck);
            void cut(std::vector<uint8_t> &deck, uint8_t index);
    };
}


#endif // CARD_UTILS_H_INCLUDED