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

    // also scores hand + starter, but optimized for speed and doesn't worry about how scores would be read out
    // scores is a pointer so can use nullptr when build_list is false and not need a dummy vector?
    uint8_t Cribbage::score_shew(std::vector<uint8_t> hand, uint8_t starter,
        std::vector<score_entry> *scores, bool build_list) {
        //SHOULD THIS BE THE VERSION AND IT LOOKS JUST LIKE THE FAST ONE BUT WITH A FLAG SAYING WHETHER TO BUILD
        //THE LIST OR NOT AND LIST IS OF ALL THE PRIMITIVES AND PATTERN MATCHING ON THE PRIMITIVES LIST CAN DERIVE
        //THE NAMES e.g.
        //say you had primitives of pair, pair, pair you could remove those and replace with a pair royal
        //YES THAT DO IT LIKE THAT but first I'll ignore build_list and scores
        //precompute some useful views of the hand
        std::array<uint8_t,5> whole_hand;
        std::array<uint8_t,5> whole_vals;
        std::array<uint8_t,5> sorthand_ranks;
        std::array<uint8_t,5> sorthand_suits;
        prep_score_hand(hand, starter, whole_hand, whole_vals, sorthand_ranks, sorthand_suits );

        uint totscore = 0;

        uint8_t i,j;
        //fifteens - in a 5 card hand, there are:
        //5 choose 5 = 1 5-card sum
        //if we save it off, can compute the 4- and 3-card totals by subtraction
        uint8_t totvals = std::accumulate(whole_vals.begin(), whole_vals.end(), 0);
        if (totvals == 15) totscore += scorePoints[SCORE_FIFTEEN];
        // if the total value is < 15, don't need to check for any fewer-card ones. rare case - worth checking? Sure why not
        else if (totvals > 15) {
            //5 choose 4 = 5 4-card sums, which can be considered the 5 card sum minus each single card value
            for(i=0;i<5;i++) if (totvals - whole_vals[i] == 15) { totscore += scorePoints[SCORE_FIFTEEN]; }
            //5 choose 3 = 10 3-card sums (or total value minus 2 cards' values so only need two nested loops)
            for(i=0;i<4;i++)
                for(j=i+1;j<5;j++)
                    if (totvals - (whole_vals[i] + whole_vals[j]) == 15) { totscore += scorePoints[SCORE_FIFTEEN]; }
            //5 choose 2 = 10 2-card sums
            for(i=0;i<4;i++)
                for(j=i+1;j<5;j++)
                    if (whole_vals[i] + whole_vals[j] == 15) { totscore += scorePoints[SCORE_FIFTEEN]; }
        }

        //pairs
        //so just compare every two cards' ranks and count a pair if they're equal
        //can it be short-circuited? think re:
        for(i=0;i<4;i++)
            for(j=i+1;j<5;j++)
                if (sorthand_ranks[i] = sorthand_ranks[j]) { totscore += scorePoints[SCORE_PAIR]; }

        //runs
        //short circuit by # pairs? maybs, think re:
        //but if there is a run of 5, don't bother looking for any other runs
        //let's try doing this w/o normalization
        //would it help to save these off into variables? modern compiler should be smart enough to store them all
        //itself? Might try later
        if ((sorthand_ranks[0] == sorthand_ranks[1] -1) && (sorthand_ranks[1] == sorthand_ranks[2] -1) &&
            (sorthand_ranks[2] == sorthand_ranks[3] -1) && (sorthand_ranks[3] == sorthand_ranks[4] -1) &&
            (sorthand_ranks[4] == sorthand_ranks[5] -1)) {
            totscore += scorePoints[SCORE_RUN5];
        } else {
            //********************************************************************************************************
            //********************************************************************************************************
            //********************************************************************************************************
            // HEY THIS DOES NOT CAPTURE DOUBLE RUNS OF 4!
            // MAYBE I SHOULD JUST DO PATTERN MATCH AFTER ALL
            //********************************************************************************************************
            //********************************************************************************************************
            //********************************************************************************************************

            //runs of 4
            bool first4 = false, last4 = false;
            if ((sorthand_ranks[0] == sorthand_ranks[1] -1) && (sorthand_ranks[1] == sorthand_ranks[2] -1) &&
                (sorthand_ranks[2] == sorthand_ranks[3] -1) && (sorthand_ranks[3] == sorthand_ranks[4] -1)) {
                totscore += scorePoints[SCORE_RUN4];
                first4 = true;
            } else if ((sorthand_ranks[1] == sorthand_ranks[2] -1) && (sorthand_ranks[2] == sorthand_ranks[3] -1) &&
                       (sorthand_ranks[3] == sorthand_ranks[4] -1) && (sorthand_ranks[4] == sorthand_ranks[5] -1)) {
                totscore += scorePoints[SCORE_RUN4];
                last4 = true;
            }

            /*
            //********************************************************************************************************
            //********************************************************************************************************
            //********************************************************************************************************
            // HEY THIS DOES NOT CAPTURE DOUBLE/TRIPLE RUNS OF 3!
            // MAYBE I SHOULD JUST DO PATTERN MATCH AFTER ALL
            //********************************************************************************************************
            //********************************************************************************************************
            //********************************************************************************************************
            //runs of 3
            //WHICH OF THEM ARE DISALLOWED BY RUNS OF 4?
            //if first 4 cards are a run of 4, then 0 1 2 and 1 2 3 are out; 2 3 4 ok
            if (first4 == false && (sorthand_ranks[0] == sorthand_ranks[1] -1) &&
                (sorthand_ranks[1] == sorthand_ranks[2] -1) && (sorthand_ranks[2] == sorthand_ranks[3] -1)) {

            }
            //if second 4 cards are a run of 4, then 0 1 2 is ok but 1 2 3 and 2 3 4 are out
            */
        }

        //flushes

        //nobs

        if (build_list) {
            //HERE will have the postprocessing to figure out the named scores
        }

        return totscore;
    }

}
