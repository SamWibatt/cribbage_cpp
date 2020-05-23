#ifndef CRIBBAGE_CPP_H_INCLUDED
#define CRIBBAGE_CPP_H_INCLUDED

#include "card_utils.h"
#include <cstdint>
#include <utility>
#include <vector>
#include <numeric>
#include <algorithm>


using namespace cardutils;

namespace cribbage_cpp {

    class Cribbage {

        // data members =======================================================================================
        public:
            //score
            // global score list
            enum score_index : uint8_t {
                SCORE_NOBS,          //  0. nobs - 1
                SCORE_GO,            //  1. go - 1
                SCORE_FIFTEEN,       //  2. fifteen - 2
                SCORE_THIRTYONE,     //  3. thirty-one - 2
                SCORE_PAIR,          //  4. pair - 2
                SCORE_HEELS,         //  5. heels - 2
                SCORE_RUN3,          //  6. run of 3 - 3
                SCORE_RUN4,          //  7. run of 4 - 4
                SCORE_TWOPAIR,       //  8. two pair - 4
                SCORE_FLUSH,         //  9. flush - 4
                SCORE_RUN5,          // 10. run of 5 - 5
                SCORE_FLUSH5,        // 11. 5 card flush - 5
                SCORE_RUN6,          // 12. run of 6 - 6
                SCORE_PAIRROYAL,     // 13. pair royal - 6
                SCORE_RUN7,          // 14. run of 7 - 7
                SCORE_DBLRUN3,       // 15. double run of 3 - 8
                SCORE_DBLRUN4,       // 16. double run of 4 - 10
                SCORE_4KIND,         // 17. 4 of a kind
                SCORE_TRIPLERUN,     // 18. triple run - 15
                SCORE_DBLDBLRUN      // 19. double double run - 16
            };

            // name and score for each of these

            std::pair<std::string,uint8_t> scoreStringsNPoints[20] = {
                std::pair<std::string,uint8_t>("nobs",1),                 // SCORE_NOBS = 0
                std::pair<std::string,uint8_t>("go",1),                   // SCORE_GO = 1
                std::pair<std::string,uint8_t>("fifteen",2),              // SCORE_FIFTEEN = 2
                std::pair<std::string,uint8_t>("thirty-one",2),           // SCORE_THIRTYONE = 3
                std::pair<std::string,uint8_t>("pair",2),                 // SCORE_PAIR = 4
                std::pair<std::string,uint8_t>("heels",2),                // SCORE_HEELS = 5
                std::pair<std::string,uint8_t>("run of 3",3),             // SCORE_RUN3 = 6
                std::pair<std::string,uint8_t>("run of 4",4),             // SCORE_RUN4 = 7
                std::pair<std::string,uint8_t>("two pair",4),             // SCORE_TWOPAIR = 8
                std::pair<std::string,uint8_t>("flush",4),                // SCORE_FLUSH = 9
                std::pair<std::string,uint8_t>("run of 5",5),             // SCORE_RUN5 = 10
                std::pair<std::string,uint8_t>("5 card flush",5),         // SCORE_FLUSH5 = 11
                std::pair<std::string,uint8_t>("run of 6",6),             // SCORE_RUN6 = 12
                std::pair<std::string,uint8_t>("pair royal",6),           // SCORE_PAIRROYAL = 13
                std::pair<std::string,uint8_t>("run of 7",7),             // SCORE_RUN7 = 14
                std::pair<std::string,uint8_t>("double run of 3",8),      // SCORE_DBLRUN3 = 15
                std::pair<std::string,uint8_t>("double run of 4",10),     // SCORE_DBLRUN4 = 16
                std::pair<std::string,uint8_t>("4 of a kind",12),         // SCORE_4KIND = 17
                std::pair<std::string,uint8_t>("triple run",15),          // SCORE_TRIPLERUN = 18
                std::pair<std::string,uint8_t>("double double run",16)    // SCORE_DBLDBLRUN = 19
            };

            //here is the card and deck handling object
            CardUtils cu;

        // nested classes =====================================================================================
        public:
            // score entry class
            // part_cards is the mask of participating cards,
            // where card n is part of the score if bit 1 << n is set
            class score_entry {
                public:
                    uint8_t part_cards = 0;      //mask of participating cards
                    uint8_t score_index = 0xFF;

                    //copy ctor - do you still need these?
                    score_entry(score_entry &other) {
                        part_cards = other.part_cards;
                        score_index = other.score_index;
                    }
                    // ctor for when you know the participating-card mask and score index
                    score_entry(uint8_t pc,uint8_t sci) {
                        part_cards = pc;
                        score_index = sci;
                    }
                    // ctor for when you know the score index but will be filling in
                    // participating cards with the settors below
                    score_entry(uint8_t sci) {
                        part_cards = 0;
                        score_index = sci;
                    }
                    // query for whether card 0..4 is involved in the score
                    bool is_card_involved(uint8_t index) {
                        if (index > 4) return false;
                        return (part_cards & (1 << index)) == 1;
                    }
                    void set_card_involved(uint8_t index) { if (index < 5) index |= (1 << index); }
                    void clear_card_involved(uint8_t index) { if (index < 5) index &= ~(1<<index); }
            };

        // ctor / dtor ========================================================================================
        public:
            Cribbage();
            ~Cribbage();

        // member functions ===================================================================================
        public:


            // given a hand of cards, starter card, and a score entry vector,
            // returns the total score for the hand
            // and fills the scores vector with the score entries for whatever the hand has in it,
            // empty if nothing
            uint8_t score_shew(std::vector<uint8_t> hand, uint8_t starter, std::vector<score_entry> &scores);

    };

}


#endif // CRIBBAGE_CPP_H_INCLUDED
