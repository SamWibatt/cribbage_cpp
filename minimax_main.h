//minimax.cpp! Let's try doing some AI stuff
// for now, let's just do it as a standalone main, and we'll move it around as needed
#ifndef MINIMAX_MAIN_H_INCLUDED
#define MINIMAX_MAIN_H_INCLUDED

#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include <memory>
#include "plat_minimax_main.h"


namespace minimax {
  // types
  //node_value_t should be a signed type
  typedef int32_t node_value_t;

  // constants
  // set these to be the most positive and most negative values possible, they're the +/- infinity for alpha and beta
  const node_value_t min_node_value = INT32_MIN;
  const node_value_t max_node_value = INT32_MAX;

  class MinimaxNode {

    protected:
      //it has to know if it's a max or min for score evaluation
      bool max_node;
   
      //depth, as in move/countermove depth. starts at max_depth and counts down
      index_t depth;


    public:
      MinimaxNode() { /* plprintf("Hey in MinimaxNode\n");*/ }
      MinimaxNode(index_t dep) : MinimaxNode() { depth = dep; } 
      virtual ~MinimaxNode() {} 

      //accessors ----------------------------------------------------------------------------------------------
      bool is_max_node() { return max_node; }
      void set_max_node(bool numax) { max_node = numax; }

      index_t get_depth() { return depth; }
      void set_depth(index_t d) { depth = d; }

    public:
      virtual node_value_t heuristic_value() { return 0; }
      virtual bool is_terminal() { return true; }
      
      //find all the legal countermoves to this state - should it be all *possible* countermoves?
      virtual void find_legal_countermoves(bool is_max, std::vector<std::unique_ptr<MinimaxNode>> &rv) { 
        rv.clear();
      }

      // for debugging and testing
      virtual void print_node(index_t max_depth) {}

  };

  class MinimaxRunner {
    protected:
      //maximum depth of descent, huge or known larger than max possible depth for unlimited
      index_t max_depth = 0;

    public:
      MinimaxRunner() {}
      MinimaxRunner(index_t md) : MinimaxRunner() { max_depth = md; }
      virtual ~MinimaxRunner() {}

      index_t get_max_depth() { return max_depth; }
      void set_max_depth(index_t nm) { max_depth = nm; }

    public:
      virtual node_value_t alphabeta(MinimaxNode &node, index_t depth, node_value_t alpha, node_value_t beta, bool is_max);
  };

  class CribbageCountNode : public MinimaxNode {
    protected:
      //what goes in a node?

      // cumulative score leading up to this point...? I guess so. If we can carry along the cards
      // and stack, and could carry along the previous board states in Othello, we can carry this and not be cheating
      node_value_t cumulativeScore;

      //stack so far - in terms of ranks. Is it misleading to use card_t?
      //no, let's ignore suit by considering everything to have suit 0, hearts.
      //that way, they will still work with the cardutils val() and rank() and therefore
      //the cribbage_core play_card.
      std::vector<card_t> stackcards;

      //cards in hand after the one being played, for finding children
      std::vector<card_t> handcards;

      //card that was played to get to this state
      card_t card_to_play;

      //ranks of cards remaining in deck - this is how we can impose the constraint that there be only 4
      //cards of each rank represented. initialize to all 4s, then subtract 1 as a card of that rank
      //is used in either dealing the max player's hand or proposing a min player's countermove.
      //SO IF I WANT TO TRY A TOY VERSION, HOW ABOUT HAVING THE OPPONENT'S HAND IN HERE RATHER THAN THE 
      //WHOLE REST OF THE DECK?
      //or how about those are the same thing? 
      //let's just call it opponentcards - of course, it'll be just hearts
      //no the rank thing was better
      std::array<index_t,13> remainingRankCounts;

    public:
      // ctor / dtor -------------------------------------------------------------------------------------

      CribbageCountNode();
      CribbageCountNode(index_t dp, bool mn, node_value_t cs, std::vector<card_t> &sc, std::vector<card_t> &hc, 
                        std::array<index_t,13> &rrc, card_t ctp) : CribbageCountNode() {
        depth = dp;
        max_node = mn;
        cumulativeScore = cs;
        stackcards = sc;
        handcards = hc;
        remainingRankCounts = rrc;
        card_to_play = ctp;
      }
      virtual ~CribbageCountNode();

      // accessors ---------------------------------------------------------------------------------------
      node_value_t get_cumulative_score() { return cumulativeScore; }
      void set_cumulative_score(node_value_t s) { cumulativeScore = s; }

      //do we want a ref here?
      std::vector<card_t> &get_stackcards() { return stackcards; }
      void set_stackcards(std::vector<card_t> sc) { stackcards = sc; }

      //do we want a ref here?
      std::vector<card_t> &get_handcards() { return handcards; }
      void set_handcards(std::vector<card_t> sc) { handcards = sc; }

      card_t get_card_to_play() { return card_to_play; }
      void set_card_to_play(card_t ctp) { card_to_play = ctp; }

      //do we want a ref here?
      std::array<index_t,13> &get_remaining_rank_counts() { return remainingRankCounts; }
      void set_opponentcards(std::array<index_t,13> oc) { remainingRankCounts = oc; }

      // actions -----------------------------------------------------------------------------------------
      //heuristic value should be signed to allow for negative numbers - some states might be really baddddd
      node_value_t heuristic_value() override;
      bool is_terminal() override;
      void find_legal_countermoves(bool is_max, std::vector<std::unique_ptr<MinimaxNode>> &rv) override;
      void print_node(index_t max_depth) override;
  };
}

#endif
