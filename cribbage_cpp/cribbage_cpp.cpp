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

    const int NUM_FIVECARDERS = 11;     //bc for some reason I can't do .size() on the upcoming

    const std::array<uint8_t,5> fivecard_patterns[NUM_FIVECARDERS] = {
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
    const uint8_t fivecard_scores[NUM_FIVECARDERS] = {
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

    const int NUM_FOURCARDERS = 4;     //bc for some reason I can't do .size() on the upcoming

    const std::array<uint8_t,4> fourcard_patterns[NUM_FOURCARDERS] = {
        {0, 1, 2, 3},    // "run of 4"    //   4 = 4*1 per card
        {0, 0, 1, 2},    // "double run of 3"
        (0, 1, 1, 2),    // "double run of 3"
        (0, 1, 2, 2),    // "double run of 3"
    };

    //scores parallel to the fourcard patterns
    const uint8_t fourcard_scores[NUM_FOURCARDERS] = {
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
        std::array<uint8_t,5> whole_suits;
        prep_score_hand(hand, starter, whole_hand, whole_vals, sorthand_nranks, whole_suits );

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
        for (j = 0; j < NUM_FIVECARDERS; j++) {
            if(sorthand_nranks == fivecard_patterns[j]) {
                totscore += fivecard_scores[j];
                fivecard_found = true;
                break;
            }
        }

        //so, if a five carder matched, don't look for 4 or 3 card runs or pairs
        if (!fivecard_found) {
            // but since one wasn't, first look for 4-carders
            bool fourcard_found = false;

            // 4 card patterns!
            std::array<uint8_t,4> first4;
            std::array<uint8_t,4> last4;
            uint8_t firstlastrank = sorthand_nranks[1];     //first rank of last 4 cards
            std::copy_n(sorthand_nranks.begin(),4,first4.begin());

            //so, mimicking the python version, it assumes that if there's a match in the first 4, there won't be
            //in the last 4. So we can defer building the second four
            for (j = 0; j < NUM_FOURCARDERS; j++) {
                if(first4 == fourcard_patterns[j]) {
                    totscore += fourcard_scores[j];
                    fourcard_found = true;
                    break;
                }
            }

            if(!fourcard_found) {
                //build second four
                std::transform(sorthand_nranks.begin() + 1,sorthand_nranks.end(),last4.begin(),
                    [firstlastrank](uint8_t rank) -> uint8_t { return rank - firstlastrank; });

                for (j = 0; j < NUM_FOURCARDERS; j++) {
                    if(last4 == fourcard_patterns[j]) {
                        totscore += fourcard_scores[j];
                        fourcard_found = true;
                        break;
                    }
                }

                if(!fourcard_found) {
                    //still no fourcarder found, look for runs of 3 and pairs
                    /* python version of run spotter
                    look for better efficiencies:
                    re: nsrnks needing a sort ....won't they always be sorted? they do need normalizing.
                    Is it the case that all we need to do is a sliding window of first 3, 2nd 3, 3rd 3?
                    bc at this point we know that there aren't double or triple runs, which should be the
                    only way there could be non-contiguous runs in the sorted ranks, yes?
                    So let's try it that way


                    for i in range(0, 3):
                        for j in range(i + 1, 4):
                            for k in range(j + 1, 5):
                                # how do we look for a run? get the sorted of the 3 cards' ranks.
                                nsrnks = sorted([self.rank(cards[i]),self.rank(cards[j]),self.rank(cards[k])])
                                nsrnks = [x-min(nsrnks) for x in nsrnks]
                                if nsrnks == [0,1,2]:
                                    curscore += 3
                                    #print(cardstring(cards[i]),cardstring(cards[j]),cardstring(cards[k]),"... run of 3 -",
                                    #      curscore)
                                    score_subsets.append([[i,j,k], self.SCORE_RUN3])
                    */
                    //don't need to construct or normalize anything! just check for ranks in order
                    //should also be exclusive bc 0,1,2 as first 3 and 1,2,3 as 2nd would have been
                    //caught as a run of 4
                    for (j = 0; j < 2; j++) {
                        if((sorthand_nranks[j] == sorthand_nranks[j+1] -1) &&
                            (sorthand_nranks[j+1] == sorthand_nranks[j+2] -1)) {
                            totscore += scorePoints[SCORE_RUN3];
                            break;
                        }
                    }

                    //pairs
                    //so just compare every two cards' ranks and count a pair if they're equal
                    //can it be short-circuited? think re:
                    //could I use a permutation operator for this? Might be fun to try
                    //but I think this is more concise
                    for(i=0;i<4;i++)
                        for(j=i+1;j<5;j++)
                            if (sorthand_nranks[i] == sorthand_nranks[j]) { totscore += scorePoints[SCORE_PAIR]; }
                }
            }
        }

        // ************************************************************************************************************
        // ************************************************************************************************************
        // ************************************************************************************************************
        // so is there a permutationy way to handle runs that will let us avoid the sorting and normalization?
        // worth considering but I think I'm prematurely optimizing enough
        // ************************************************************************************************************
        // ************************************************************************************************************
        // ************************************************************************************************************

        //flushes - only counts if all 4 cards in the hand have the same suit. Then if it's also
        //the same suit as the starter, you get an extra point
        if ((whole_suits[0] == whole_suits[1]) && (whole_suits[0] == whole_suits[2]) &&
            (whole_suits[0] == whole_suits[3])) {
                if (whole_suits[0] == whole_suits[4]) {
                    totscore += SCORE_FLUSH5;
                } else {
                    totscore += SCORE_FLUSH;
                }
            }

        //nobs: if starter is not a jack, and hand includes a jack with the same suit as starter
        //jack is rank 10
        if (cu.rank(whole_hand[4]) != 10) {
            for(j = 0; j < 4; j++) {
                if (whole_suits[j] == whole_suits[4] && cu.rank(whole_hand[j]) == 10) {
                    totscore += SCORE_NOBS;
                    break;
                }
            }
        }

        return totscore;
    }

}
