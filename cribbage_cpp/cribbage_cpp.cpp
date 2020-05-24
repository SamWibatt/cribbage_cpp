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
        std::array<uint8_t,5> &sorthand_nranks, std::array<uint8_t,5> &whole_suits ) {

        // get the whole_hand array going - copy hand over, then drop starter at end
        std::copy_n(hand.begin(), 4, whole_hand.begin());
        whole_hand[4] = starter;

        // then construct the vals array from that
        std::transform(whole_hand.begin(),whole_hand.end(),whole_vals.begin(),
            [this](uint8_t card) -> uint8_t { return this->cu.val(card); });

        // then construct the sorted hand in sorthand_nranks
        std::copy(whole_hand.begin(),whole_hand.end(),sorthand_nranks.begin());
        std::sort(sorthand_nranks.begin(),sorthand_nranks.end());
        //finally, boil sorted_ranks down to its ranks and subtract off first card's rank
        //for run spotting
        uint8_t first_rank = cu.rank(sorthand_nranks[0]);
        std::transform(sorthand_nranks.begin(),sorthand_nranks.end(),sorthand_nranks.begin(),
            [first_rank,this](uint8_t card) -> uint8_t { return this->cu.rank(card) - first_rank; });

        //then get whole_hand's suits? Not sure we need to do this here, could defer until we
        //know we need to check for flushes
        //extract suits
        std::transform(whole_hand.begin(),whole_hand.end(),whole_suits.begin(),
            [this](uint8_t card) -> uint8_t { return this->cu.suit(card); });

    }


    const std::array<uint8_t,5> fivecard_patterns[11] = {
        {0, 1, 2, 3, 4},    // "run of 5"    //   5 = 5*1 per card
        {0, 0, 0, 1, 2},    // "triple run"    //  15 = 3*3 runs + 3*2 pairs
        {0, 1, 1, 1, 2},    // "triple run", 15],
        {0, 1, 2, 2, 2},    // "triple run", 15],
        {0, 0, 1, 1, 2},    // "double double run"    //  16 = 4*3 runs + 2*2 pairs
        {0, 0, 1, 2, 2},    // "double double run", 16],
        {0, 1, 1, 2, 2},    // "double double run", 16],
        {0, 0, 1, 2, 3},    // "double run of 4"    //  10 = 2*4 runs + 1*2 pairs
        {0, 1, 1, 2, 3},    // "double run of 4", 10],
        {0, 1, 2, 2, 3},    // "double run of 4", 10],
        {0, 1, 2, 3, 3},    // "double run of 4", 10]]
    };

    //scores parallel to the fivecard patterns
    const uint8_t fivecard_scores[11] = {
        Cribbage::SCORE_RUN5,      //"run of 5"           #  5 = 5*1 per card
        Cribbage::SCORE_TRIPLERUN, //"triple run"        # 15 = 3*3 runs + 3*2 pairs
        Cribbage::SCORE_TRIPLERUN, //"triple run"
        Cribbage::SCORE_TRIPLERUN, //"triple run"
        Cribbage::SCORE_DBLDBLRUN, //"double double run" // 16 = 4*3 runs + 2*2 pairs
        Cribbage::SCORE_DBLDBLRUN, //"double double run"
        Cribbage::SCORE_DBLDBLRUN, //"double double run"
        Cribbage::SCORE_DBLRUN4,   //"double run of 4"   # 10 = 2*4 runs + 1*2 pairs
        Cribbage::SCORE_DBLRUN4,   //"double run of 4"
        Cribbage::SCORE_DBLRUN4,   //"double run of 4"
        Cribbage::SCORE_DBLRUN4    //"double run of 4"
    };

    const std::array<uint8_t,5> fourcard_patterns[4] = {
        {0, 1, 2, 3},    // "run of 4"    //   4 = 4*1 per card
        {0, 0, 1, 2},    // "double run of 3"
        (0, 1, 1, 2),    // "double run of 3"
        (0, 1, 2, 2),    // "double run of 3"
    };

    //scores parallel to the fourcard patterns
    const uint8_t fourcard_scores[11] = {
        Cribbage::SCORE_RUN4,      //"run of 4"           #  4 = 4*1 per card
        Cribbage::SCORE_DBLRUN3,
        Cribbage::SCORE_DBLRUN3,
        Cribbage::SCORE_DBLRUN3
    };

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
        std::array<uint8_t,5> sorthand_nranks;
        std::array<uint8_t,5> sorthand_suits;
        prep_score_hand(hand, starter, whole_hand, whole_vals, sorthand_nranks, sorthand_suits );

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

        bool fivecard_found = false;
        //pattern scores
        //dunno why fivecard_patterns.size() doesn't work, yoding 11
        for (j = 0; j < 11; j++) {
            if(sorthand_nranks == fivecard_patterns[j]) {
                totscore += fivecard_scores[j];
                fivecard_found = true;
                break;
            }
        }

        //so, if a five carder matched, don't look for 4 or 3 card runs or pairs
        if (!fivecard_found) {

            //ok, how to do the fourcards?
            // *****************************************************************************************************
            // *****************************************************************************************************
            // *****************************************************************************************************
            // TODO WRITE THIS
            // TODO WRITE THIS
            // TODO WRITE THIS
            // TODO WRITE THIS
            // Is there a way to not do == but do the relative thing
            // for a first-4 it's easy enough bc it's 0-rel
            // could calc first-4 and second-4, then even 1st-3, 2nd-3, 3rd-3
            // why not, it's not much bytes
            // *****************************************************************************************************
            // *****************************************************************************************************
            // *****************************************************************************************************
            bool fourcard_found = false;

            // 4 card patterns!
            std::array<uint8_t,4> first4;
            std::array<uint8_t,4> last4;
            uint8_t firstlastrank = sorthand_nranks[1];     //first rank of last 4 cards
            std::copy_n(sorthand_nranks.begin(),4,first4.begin());
            std::transform(sorthand_nranks.begin() + 1,sorthand_nranks.end(),last4.begin(),
                [firstlastrank](uint8_t rank) -> uint8_t { return rank - firstlastrank; });

            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means
            // LEFT OFF HERE - do the 4-card pattern matches on first4 and last4 and figure out what that means

            if(!fourcard_found) {
                //pairs
                //so just compare every two cards' ranks and count a pair if they're equal
                //can it be short-circuited? think re:
                for(i=0;i<4;i++)
                    for(j=i+1;j<5;j++)
                        if (sorthand_nranks[i] == sorthand_nranks[j]) { totscore += scorePoints[SCORE_PAIR]; }
            }
        }
        /*
        //runs
        //short circuit by # pairs? maybs, think re:
        //but if there is a run of 5, don't bother looking for any other runs
        //let's try doing this w/o normalization
        //would it help to save these off into variables? modern compiler should be smart enough to store them all
        //itself? Might try later
        if ((sorthand_nranks[0] == sorthand_nranks[1] -1) && (sorthand_nranks[1] == sorthand_nranks[2] -1) &&
            (sorthand_nranks[2] == sorthand_nranks[3] -1) && (sorthand_nranks[3] == sorthand_nranks[4] -1) &&
            (sorthand_nranks[4] == sorthand_nranks[5] -1)) {
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
            if ((sorthand_nranks[0] == sorthand_nranks[1] -1) && (sorthand_nranks[1] == sorthand_nranks[2] -1) &&
                (sorthand_nranks[2] == sorthand_nranks[3] -1) && (sorthand_nranks[3] == sorthand_nranks[4] -1)) {
                totscore += scorePoints[SCORE_RUN4];
                first4 = true;
            } else if ((sorthand_nranks[1] == sorthand_nranks[2] -1) && (sorthand_nranks[2] == sorthand_nranks[3] -1) &&
                       (sorthand_nranks[3] == sorthand_nranks[4] -1) && (sorthand_nranks[4] == sorthand_nranks[5] -1)) {
                totscore += scorePoints[SCORE_RUN4];
                last4 = true;
            }


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
            if (first4 == false && (sorthand_nranks[0] == sorthand_nranks[1] -1) &&
                (sorthand_nranks[1] == sorthand_nranks[2] -1) && (sorthand_nranks[2] == sorthand_nranks[3] -1)) {

            }
            //if second 4 cards are a run of 4, then 0 1 2 is ok but 1 2 3 and 2 3 4 are out
            */
        //}

        //flushes

        //nobs

        if (build_list) {
            //HERE will have the postprocessing to figure out the named scores
        }

        return totscore;
    }

}
