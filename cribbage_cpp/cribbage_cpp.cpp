/**

cribbage_cpp - C++ implementation of cribbage utilities and scoring

Targeting embedded systems, so uses small containers like card_t at the risk of loss of speed on larger machines

*/


#include "cribbage_cpp.h"

namespace cribbage_cpp {

    Cribbage::Cribbage() {
        //ctor stuff here
    }

    Cribbage::~Cribbage() {
        //dtor stuff here
    }

    //sort_map is untouched if build_list is false
    void Cribbage::prep_score_hand(std::vector<card_t> &hand, card_t starter,
        std::array<card_t,5> &whole_hand, std::array<card_t,5> &whole_vals,
        std::array<card_t,5> &sorthand_nranks, std::array<card_t,5> &whole_suits,
        std::array<index_t,5> &sort_map, bool build_list ) {

        // get the whole_hand array going - copy hand over, then drop starter at end
        std::copy_n(hand.begin(), 4, whole_hand.begin());
        whole_hand[4] = starter;

        // then construct the vals array from that
        std::transform(whole_hand.begin(),whole_hand.end(),whole_vals.begin(),
            [this](card_t card) -> card_t { return this->cu.val(card); });

        // then construct the sorted hand in sorthand_nranks
        if (!build_list) {
            std::copy(whole_hand.begin(),whole_hand.end(),sorthand_nranks.begin());
            std::sort(sorthand_nranks.begin(),sorthand_nranks.end());
        } else {
            // if we're building score lists, we need to build a mapping from sorted back to original too
            // THIS ASSUMES THAT THE VALUES STORED IN CARD_T AND INDEX_T CAN COEXIST IN 32-BIT CONTAINERS!
            // which is true. cards are only ever 0..51 and, in this case, index is 0..4
            // so: build an array of card << 16 | index - want card in the upper half bc that's what we sort by
            std::array<uint32_t,5> mapsort_temp;
            for(uint32_t j = 0; j < 5; j++) mapsort_temp[j] = uint32_t(whole_hand[j]) << 16 | j;
            std::sort(mapsort_temp.begin(),mapsort_temp.end());
            // copy cards over to sorthand_nranks
            std::transform(mapsort_temp.begin(),mapsort_temp.end(),sorthand_nranks.begin(),
                [](uint32_t x){ return card_t(x >> 16); });
            // copy indices over to sort_map
            std::transform(mapsort_temp.begin(),mapsort_temp.end(),sort_map.begin(),
                [](uint32_t x){ return index_t(x & 0x0000FFFF); });

        }
        //finally, boil sorted_ranks down to its ranks and subtract off first card's rank
        //for run spotting
        card_t first_rank = cu.rank(sorthand_nranks[0]);
        std::transform(sorthand_nranks.begin(),sorthand_nranks.end(),sorthand_nranks.begin(),
            [first_rank,this](card_t card) -> card_t { return this->cu.rank(card) - first_rank; });

        //then get whole_hand's suits? Not sure we need to do this here, could defer until we
        //know we need to check for flushes
        //extract suits
        std::transform(whole_hand.begin(),whole_hand.end(),whole_suits.begin(),
            [this](card_t card) -> card_t { return this->cu.suit(card); });

    }

    const int NUM_FIVECARDERS = 11;     //bc for some reason I can't do .size() on the upcoming

    const std::array<card_t,5> fivecard_patterns[NUM_FIVECARDERS] = {
        {{0, 1, 2, 3, 4}},    // "run of 5"    //   5 = 5*1 per card
        {{0, 0, 0, 1, 2}},    // "triple run"    //  15 = 3*3 runs + 3*2 pairs
        {{0, 1, 1, 1, 2}},    // "triple run", 15],
        {{0, 1, 2, 2, 2}},    // "triple run", 15],
        {{0, 0, 1, 1, 2}},    // "double double run"    //  16 = 4*3 runs + 2*2 pairs
        {{0, 0, 1, 2, 2}},    // "double double run", 16],
        {{0, 1, 1, 2, 2}},    // "double double run", 16],
        {{0, 0, 1, 2, 3}},    // "double run of 4"    //  10 = 2*4 runs + 1*2 pairs
        {{0, 1, 1, 2, 3}},    // "double run of 4", 10],
        {{0, 1, 2, 2, 3}},    // "double run of 4", 10],
        {{0, 1, 2, 3, 3}},    // "double run of 4", 10]]
    };

    //scores parallel to the fivecard patterns
    const card_t fivecard_score_indices[NUM_FIVECARDERS] = {
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

    const int NUM_FOURCARDERS = 5;     //bc for some reason I can't do .size() on the upcoming

    const std::array<card_t,4> fourcard_patterns[NUM_FOURCARDERS] = {
        {{0, 0, 0, 0}},    // 4 of a kind
        {{0, 1, 2, 3}},    // "run of 4"    //   4 = 4*1 per card
        {{0, 0, 1, 2}},    // "double run of 3"
        {{0, 1, 1, 2}},    // "double run of 3"
        {{0, 1, 2, 2}}    // "double run of 3"
    };

    //scores parallel to the fourcard patterns
    const card_t fourcard_score_indices[NUM_FOURCARDERS] = {
        Cribbage::SCORE_4KIND,
        Cribbage::SCORE_RUN4,      //"run of 4"           #  4 = 4*1 per card
        Cribbage::SCORE_DBLRUN3,
        Cribbage::SCORE_DBLRUN3,
        Cribbage::SCORE_DBLRUN3
    };

    // also scores hand + starter, but optimized for speed and doesn't worry about how scores would be read out
    // scores is a pointer so can use nullptr when build_list is false and not need a dummy vector?
    index_t Cribbage::score_shew(std::vector<card_t> hand, card_t starter,
        std::vector<score_entry> *scores, bool build_list) {
        //SHOULD THIS BE THE VERSION AND IT LOOKS JUST LIKE THE FAST ONE BUT WITH A FLAG SAYING WHETHER TO BUILD
        //THE LIST OR NOT AND LIST IS OF ALL THE PRIMITIVES AND PATTERN MATCHING ON THE PRIMITIVES LIST CAN DERIVE
        //THE NAMES e.g.
        //say you had primitives of pair, pair, pair you could remove those and replace with a pair royal
        //YES THAT DO IT LIKE THAT but first I'll ignore build_list and scores
        //precompute some useful views of the hand
        std::array<card_t,5> whole_hand;
        std::array<card_t,5> whole_vals;
        std::array<card_t,5> sorthand_nranks;
        std::array<card_t,5> whole_suits;
        std::array<index_t,5> sort_map;
        prep_score_hand(hand, starter, whole_hand, whole_vals, sorthand_nranks, whole_suits,
            sort_map, build_list );

        //figure out if we will be building a score list. build_list should only be true if scores is non-nullptr, but be sure
        bool make_list = (scores != nullptr && build_list == true) ? true : false;
        if (make_list) scores->clear();

        index_t totscore = 0;

        index_t i,j;
        //fifteens - in a 5 card hand, there are:
        //5 choose 5 = 1 5-card sum
        //if we save it off, can compute the 4- and 3-card totals by subtraction
        index_t totvals = index_t(std::accumulate(whole_vals.begin(), whole_vals.end(), 0));
        if (totvals == 15) {
            totscore += scorePoints[SCORE_FIFTEEN];
            // participating cards is all of them - so 0001 1111 bc we count from right, first card = rightmost
            // bit; therefore 0x1F
            if(make_list) scores->push_back(score_entry(0x1F,SCORE_FIFTEEN));
        }
        // if the total value is < 15, don't need to check for any fewer-card ones. rare case - worth checking? Sure why not
        else if (totvals > 15) {
            //5 choose 4 = 5 4-card sums, which can be considered the 5 card sum minus each single card value
            for(i=0;i<5;i++) if (totvals - index_t(whole_vals[i]) == 15) {
                totscore += scorePoints[SCORE_FIFTEEN];
                //participating cards is all but i
                if(make_list) scores->push_back(score_entry(0x1F & ~(1 << i),SCORE_FIFTEEN));
            }
            //5 choose 3 = 10 3-card sums (or total value minus 2 cards' values so only need two nested loops)
            for(i=0;i<4;i++)
                for(j=i+1;j<5;j++)
                    if (totvals - index_t(whole_vals[i] + whole_vals[j]) == 15) {
                        totscore += scorePoints[SCORE_FIFTEEN];
                        //participating cards all but i and j
                        if(make_list) scores->push_back(score_entry(0x1F & ~(1 << i | 1 << j),SCORE_FIFTEEN));
                    }
            //5 choose 2 = 10 2-card sums
            for(i=0;i<4;i++)
                for(j=i+1;j<5;j++)
                    if (whole_vals[i] + whole_vals[j] == 15) {
                        totscore += scorePoints[SCORE_FIFTEEN];
                        //participating cards are i and j
                        if(make_list) scores->push_back(score_entry((1 << i | 1 << j),SCORE_FIFTEEN));
                    }
        }

        bool fivecard_found = false;
        //pattern scores
        //dunno why fivecard_patterns.size() doesn't work, yoding 11
        for (j = 0; j < NUM_FIVECARDERS; j++) {
            if(sorthand_nranks == fivecard_patterns[j]) {
                totscore += scorePoints[fivecard_score_indices[j]];
                //participating cards all of them!
                if(make_list) scores->push_back(score_entry(index_t(0x1F),fivecard_score_indices[j]));
                fivecard_found = true;
                break;
            }
        }

        //so, if a five carder matched, don't look for 4 or 3 card runs or pairs
        if (!fivecard_found) {
            // but since one wasn't, first look for 4-carders
            bool fourcard_found = false;

            // 4 card patterns!
            std::array<card_t,4> first4;
            std::array<card_t,4> last4;
            card_t firstlastrank = sorthand_nranks[1];     //first rank of last 4 cards
            std::copy_n(sorthand_nranks.begin(),4,first4.begin());

            //so, mimicking the python version, it assumes that if there's a match in the first 4, there won't be
            //in the last 4. So we can defer building the second four
            for (j = 0; j < NUM_FOURCARDERS; j++) {
                if(first4 == fourcard_patterns[j]) {
                    totscore += scorePoints[fourcard_score_indices[j]];
                    //participating cards first 4 = 0x0f
                    if(make_list) {
                        index_t partcards = 0;
                        for(i = 0; i < 4; i++) partcards |= (1 << sort_map[i]);
                        scores->push_back(score_entry(partcards,fourcard_score_indices[j]));
                    }
                    fourcard_found = true;
                    break;
                }
            }

            if(!fourcard_found) {
                //build second four
                std::transform(sorthand_nranks.begin() + 1,sorthand_nranks.end(),last4.begin(),
                    [firstlastrank](card_t rank) -> card_t { return rank - firstlastrank; });

                for (j = 0; j < NUM_FOURCARDERS; j++) {
                    if(last4 == fourcard_patterns[j]) {
                        totscore += scorePoints[fourcard_score_indices[j]];
                        //participating cards last 4 = 0x1e!
                        if(make_list) {
                            index_t partcards = 0;
                            for(i = 1; i < 5; i++) partcards |= (1 << sort_map[i]);
                            scores->push_back(score_entry(partcards,fourcard_score_indices[j]));
                        }
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
                    for (j = 0; j < 3; j++) {
                        if((sorthand_nranks[j] == sorthand_nranks[j+1] -1) &&
                            (sorthand_nranks[j+1] == sorthand_nranks[j+2] -1)) {
                            totscore += scorePoints[SCORE_RUN3];
                            //participating cards are j, j+1, j+2 = 7 << j, yes? no. This is sorted hand
                            if(make_list) scores->push_back(score_entry((1 << sort_map[j]) | (1 << sort_map[j+1]) | (1 << sort_map[j+2]),
                                SCORE_RUN3));
                            break;
                        }
                    }

                    //pairs
                    //so just compare every two cards' ranks and count a pair if they're equal
                    //can it be short-circuited? think re: - yes, 4 of a kind is up above as a 4 card pattern
                    //could I use a permutation operator for this? Might be fun to try
                    //but I think this is more concise
                    //FOR BUILDING SCORE LISTS, WILL NEED TO KEEP TRACK OF PAIR RANKS N STUFF?
                    //that can all be postprocessing
                    //do count # pairs
                    index_t numpairs = 0;
                    for(i=0;i<4;i++)
                        for(j=i+1;j<5;j++)
                            if (sorthand_nranks[i] == sorthand_nranks[j]) numpairs++;

                    if (numpairs != 0) {
                        totscore += numpairs * scorePoints[SCORE_PAIR];
                        if(make_list) {
                            //FIGURE OUT HOW TO BUILD THE SCORE LIST! could just redo the
                            //whole pairs loop
                            //what are our possibilities?
                            //1, 2, 3, or 4 pairs.
                            //if it's 1 or 3, there is only 1 rank to worry re:
                            //otherwise there are 2
                            //OR, could just scan the list and record which ranks from
                            //A..K are which cards and also count how many.
                            //this might actually be a faster way to do pairs!
                            index_t rankcounts[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
                            index_t rankcards[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
                            for(j = 0; j < 5; j++) {
                                rankcounts[sorthand_nranks[j]]++;
                                // bits 0..5 store which cards 0..5 represent that rank.
                                // need to map back to original hand order
                                rankcards[sorthand_nranks[j]] |= (1<<sort_map[j]);
                            }
                            for(j = 0; j < 13; j++) {
                                //have to remap from sorted order to original hand order
                                if(rankcounts[j] == 2) {
                                    scores->push_back(score_entry(rankcards[j],SCORE_PAIR));
                                } else if(rankcounts[j] == 3) {
                                    scores->push_back(score_entry(rankcards[j],SCORE_PAIRROYAL));
                                }
                            }
                        }
                    }
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
        //the same suit as the starter (whole_hand[4])s, you get an extra point
        if ((whole_suits[0] == whole_suits[1]) && (whole_suits[0] == whole_suits[2]) &&
            (whole_suits[0] == whole_suits[3])) {
                if (whole_suits[0] == whole_suits[4]) {
                    totscore += scorePoints[SCORE_FLUSH5];
                    //participating cards all of them!
                    if(make_list) scores->push_back(score_entry(0x1F,SCORE_FLUSH5));

                } else {
                    totscore += scorePoints[SCORE_FLUSH];
                    //participating cards first 4!
                    if(make_list) scores->push_back(score_entry(0x0F,SCORE_FLUSH));
                }
            }

        //nobs: if starter is not a jack, and hand includes a jack with the same suit as starter
        //jack is rank 10
        if (cu.rank(starter) != 10) {
            for(j = 0; j < 4; j++) {
                if (whole_suits[j] == cu.suit(starter) && cu.rank(whole_hand[j]) == 10) {
                    totscore += scorePoints[SCORE_NOBS];
                    //participating cards starter (0x10) and j
                    if(make_list) scores->push_back(score_entry(0x10 | (1<<j),SCORE_NOBS));
                    break;
                }
            }
        }

        return totscore;
    }


    // given a stack of cards in progress and a card to add to it,
    // returns the incremental score for playing that card
    // and modifies the stack of cards to include the new one (if it is a legal play).
    // if build_list is true and scores is not nullptr, scores will contain score_entrys
    // for any scoring combinations made by the play.
    // Score_entry is such that lowest bit is... the leftmost?
    // rightmost makes more sense, if the stack grows to the right, which I believe it will.
    // just need a different render algorithm, which the graphic game will have anyway. QED.
    index_t Cribbage::
    play_card(std::vector<card_t> &stack, card_t card, std::vector<score_entry> *scores, bool build_list) {

        //zeroth: clear scores, if we're doing scores, which we are if build_list
        //is true and scores isn't nullptr.
        bool make_list = (build_list) ? scores != nullptr : false;

        if(make_list) scores->clear();

        //first: see if it is legal to play this card.
        //so initial value is val of card, add on vals of stack, see if it's <= 31. If so, legal!
        // so first find the total of stack
        // tip: "a" in the lambda here is the running total
        index_t curtotal = std::accumulate(stack.begin(), stack.end(), 0,
            [this](index_t a, card_t b){return a + index_t(this->cu.val(b));});

        //debug
        /*
        for(auto j = 0; j < stack.size(); j++) printf("%s ",getCardUtils().cardstring(stack[j]).c_str());
        printf("playing %s ",getCardUtils().cardstring(card).c_str());
        printf("total value = %d\n", curtotal + index_t(cu.val(card)));
        */

        if(curtotal + index_t(cu.val(card)) > 31) {
            //too big! no change to stack, scores nothing
            return 0;
        }

        index_t total_score = 0;
        index_t partcards;

        //ok, so it's a legal play. put our card on the stack!
        stack.push_back(card);
        curtotal += index_t(cu.val(card));

        //TEST: print last 3 cards in stack - worked!
        //printf("last 3 cards in stack\n");
        //std::for_each(stack.end()-3, stack.end(), [this](card_t c){printf("%s ",this->cu.cardstring(c).c_str());});
        //printf("\n");

        //everything after this cares about ranks, so let's precompute them
        std::vector<card_t> rankstack;
        rankstack.reserve(stack.size());
        std::transform(stack.begin(),stack.end(),rankstack.begin(),
            [this](card_t c){ return this->cu.rank(c); });


        //look for fifteen or thirty-one!!
        if(curtotal == 15) {
            total_score += scorePoints[SCORE_FIFTEEN];
            if(make_list) {
                partcards = (1 << (stack.size()+1))-1;     //e.g. if stack has 3 cards, (1<<3)-1 = binary 111
                scores->push_back(score_entry(partcards,SCORE_FIFTEEN));
            }
        } else if(curtotal == 31) {
            total_score += scorePoints[SCORE_THIRTYONE];
            if(make_list) {
                partcards = (1 << (stack.size()+1))-1;     //e.g. if stack has 3 cards, (1<<3)-1 = binary 111
                scores->push_back(score_entry(partcards,SCORE_THIRTYONE));
            }
        }

        //THEN pair/3/4 of a kind
        // get rank of newly played card, which is also stack[stack.size()-1]
        // count number of cards of that rank - count 1 for the newly played card
        // so, the second card is stack[(stack.size()-1)-1], third stack[(stack.size()-1)-2], fourth stack[(stack.size()-1)-3]
        // as long as those have the same rank as the new card, add 1 to # rank matches. If one mismatches, stop
        card_t currank = cu.rank(card);
        index_t numrankmatch = 1;
        for(auto j = 1; j < 4 && j < rankstack.size()-1; j--)
            if(rankstack[(rankstack.size()-1)-j]) == currank) numrankmatch++; else break;
        partcards = (1<<numrankmatch) - 1;
        switch(numrankmatch) {
            //1-4 should be the only possibilities, but still.
            case 2: total_score += scorePoints[SCORE_PAIR];
                if(make_list) scores->push_back(score_entry(partcards,SCORE_PAIR));
                break;
            case 3: total_score += scorePoints[SCORE_PAIRROYAL];
                if(make_list) scores->push_back(score_entry(partcards,SCORE_PAIRROYAL));
                break;
            case 4: total_score += scorePoints[SCORE_4KIND];
                if(make_list) scores->push_back(score_entry(partcards,SCORE_4KIND));
                break;
            default: break;
        }

        //THEN DO RUNS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //THEN DO RUNS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //THEN DO RUNS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        //THEN DO RUNS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // python
        //    # FIGURE OUT RUNS
        //    # this works:
        //    # >>> h = [5, 3, 7, 4, 6]
        //    # >>> for i in range(-1, -(len(h)+1), -1):
        //    # ...     print([x-min(h[i:]) for x in sorted(h[i:])])
        //    # ...
        //    # [0]
        //    # [0, 2]
        //    # [0, 2, 3]
        //    # [0, 1, 3, 4]
        //    # [0, 1, 2, 3, 4]
        //    # that looks like a way to spot runs
        //    # like pairs, go until you find a run...? an intervening non-run does not disqualify.
        //    # what is the longest possible? The whole length of newcards, I suppose.
        //    # look at them all, and pick the highest run, if any.
        //    # >>> for i in range(-1, -(len(h)+1), -1):
        //    # ...     ns = [x-min(h[i:]) for x in sorted(h[i:])]
        //    # ...     if ns == list(range(0,-i)):
        //    # ...         print(ns," = RUN!!!!!!!!! of",-i)
        //    # ...     else:
        //    # ...         print(ns," = not run :(")
        //    # ...
        //    # [0]  = RUN!!!!!!!!! of 1
        //    # [0, 2]  = not run :(
        //    # [0, 2, 3]  = not run :(
        //    # [0, 1, 3, 4]  = not run :(
        //    # [0, 1, 2, 3, 4]  = RUN!!!!!!!!! of 5
        //    # so there you have it. just start at -3 bc no shorter run matters
        //    # ALSO: need to get rank of card, not just card
        //    longestrun = 0
        //    for i in range(-3, -(len(newcards)+1), -1):
        //        sorty = [self.rank(x) for x in sorted(newcards[i:])]
        //        ns = [x-min(sorty) for x in sorty]
        //        if ns == list(range(0,-i)):
        //            longestrun = -i

        // so for C++, see if we can avoid sorting every step of the way. first, short-circuit if there are fewer than 3 cards.
        // noodle: sorting is an easy way to compare against a pattern.
        // could do just a cascading if? not really, doesn't work easily with ordering. How did I do it in vpok?
        // I think it was take max and min, and the diffie between them was #cards -1, it's a straight, as long as there are no pairs.
        // ... that is what vpok did, but the writuep doesn't mention there not being pairs!
        // but the code does:
        // call	countpairs		;see how many pairs are in the hand.
        // movf	numpairs,f		;moving onto itself sets zero flag
        // btfsc	STATUS,Z		;if nonzero, don't look for straights
        // so, since we're counting from the end of the stack - we already know if there were pairs, because we just found them.
        // therefore, if there were any pairs, there can't be a run.
        // which means:
        // for j = [3, stack.size())
        //    find min and max of last j cards in stack
        //      if((max-min) == j-1) longestrun = j;
        index_t longestrun = 0;
        if(stack.size() > 2 && numrankmatch < 2) {

            //we want to test up to stack.size() cards, so do inclusive end condition
            for(index_t j=3;j<=stack.size();j++) {
                //std::max_element and std::min_element look promising
                //how to iterate over the last n? let's do a test up above
                //printf("last 3 cards in stack\n");
                //std::for_each(stack.end()-3, stack.end(), [this](card_t c){printf("%s ",this->cu.cardstring(c).c_str());});
                //printf("\n");
                //other trick is that we are looking at RANKS, not cards so use rankstack
                index_t rankmax = std::max_element(rankstack.end()-j),rankstack.end());
                index_t rankmin = std::min_element(rankstack.end()-j),rankstack.end());
                if((rankmax-rankmin) == j-1) longestrun = j;
            }

            //    # ew, this is gross
            //    if longestrun == 3:
            //        scorelist.append((self.SCORE_RUN3,3))
            //    elif longestrun == 4:
            //        scorelist.append((self.SCORE_RUN4,4))
            //    if longestrun == 5:
            //        scorelist.append((self.SCORE_RUN5,5))
            //    elif longestrun == 6:
            //        scorelist.append((self.SCORE_RUN6,6))
            //    elif longestrun == 7:
            //        scorelist.append((self.SCORE_RUN7,7))
            //
            //    return(newcards,curtotal,scorelist)
        }

        return total_score;
    }
}
