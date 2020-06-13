//minimax.cpp! Let's try doing some AI stuff
// for now, let's just do it as a standalone main, and we'll move it around as needed
#ifndef MINIMAX_MAIN_H_INCLUDED
#define MINIMAX_MAIN_H_INCLUDED

#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include "plat_minimax_main.h"



// types
typedef int32_t node_value_t;

class CribbageCountNode {
  protected:
    //what goes in a node?
    //depth, as in move/countermove depth. Can be no more than 8.
    index_t depth;

    // cumulative score leading up to this point...? I guess so. If we can carry along the cards
    // and stack, and could carry along the previous board states in Othello, we can carry this and not be cheating
    node_value_t cumulativeScore;

    // alpha and beta - are these a node thing or just in the caller?
    node_value_t alpha, beta;

    //stack so far - in terms of ranks. Is it misleading to use card_t?
    //no, let's ignore suit by considering everything to have suit 0, hearts.
    //that way, they will still work with the cardutils val() and rank() and therefore
    //the cribbage_core play_card.
    std::vector<card_t> stackcards;

    //cards in hand
    std::vector<card_t> handcards;

    //ranks of cards remaining in deck - this is how we can impose the constraint that there be only 4
    //cards of each rank represented. initialize to all 4s, then subtract 1 as a card of that rank
    //is used in either dealing the max player's hand or proposing a min player's countermove.
    //SO IF I WANT TO TRY A TOY VERSION, HOW ABOUT HAVING THE OPPONENT'S HAND IN HERE RATHER THAN THE 
    //WHOLE REST OF THE DECK?
    std::array<index_t,13> remainingRankCounts;

  public:
    // accessors ---------------------------------------------------------------------------------------
    index_t get_depth() { return depth; }
    void set_depth(index_t d) { depth = d; }

    node_value_t get_cumulative_score() { return cumulativeScore; }
    void set_cumulative_score(node_value_t s) { cumulativeScore = s; }

    node_value_t get_alpha() { return alpha; }
    void set_alpha(node_value_t s) { alpha = s; }

    node_value_t get_beta() { return beta; }
    void set_beta(node_value_t s) { beta = s; }

    //do we want a ref here?
    std::vector<card_t> &get_stackcards() { return stackcards; }
    void set_stackcards(std::vector<card_t> sc) { stackcards = sc; }

    //do we want a ref here?
    std::vector<card_t> &get_handcards() { return handcards; }
    void set_handcards(std::vector<card_t> sc) { handcards = sc; }

    //do we want a ref here?
    std::array<index_t,13> get_remaining_rank_counts() { return remainingRankCounts; }
    void set_remaining_rank_counts(std::array<index_t,13> rrc) { remainingRankCounts = rrc; }

    // actions -----------------------------------------------------------------------------------------
    //heuristic value should be signed to allow for negative numbers - some states might be really baddddd
    node_value_t heuristicValue();
};

#endif
