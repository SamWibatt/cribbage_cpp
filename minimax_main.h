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
      //what goes in a node representing a cribbage count state?
      // It should reflect a *state*. Initial version had this card_to_play thing which makes it look weird.
      // see the tic-tac-toe example at https://www.neverstopbuilding.com/blog/minimax - given an initial state
      // where max player X is generating the next layer of nodes, the next layer shows after X has made their
      // move and the score is from X's point of view.

      // so:
      // our state is
      // - cards left in max player's hand (handcards) - all in hearts suit bc suits don't matter
      std::vector<card_t> handcards;

      // - cards already played in the stack (stackcards) - all in hearts suit bc suits don't matter
      std::vector<card_t> stackcards;

      // - count of ranks left in the deck (suits don't matter, doing it by ranks stops big fanout, write up)
      //cards of each rank represented. initialize to all 4s, then subtract 1 as a card of that rank
      //is used in either dealing the max player's hand or proposing a min player's countermove.
      std::array<index_t,13> remainingRankCounts;

      // - score so far, from max's point of view
      node_value_t cumulativeScore;

      // flag for whether we've already calculated cumulative score, so we don't call play_card more than once
      // for a given node and don't need to call it during child expansion
      bool calculated_score_yet;
    public:
      // ctor / dtor -------------------------------------------------------------------------------------

      CribbageCountNode() { calculated_score_yet = false; }
      CribbageCountNode(index_t dp, bool mn, node_value_t cs, std::vector<card_t> &sc, std::vector<card_t> &hc, 
                        std::array<index_t,13> &rrc) : CribbageCountNode() {
        depth = dp;
        max_node = mn;
        cumulativeScore = cs;
        stackcards = sc;
        handcards = hc;
        remainingRankCounts = rrc;
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
