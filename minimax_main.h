//minimax.cpp! Let's try doing some AI stuff
// for now, let's just do it as a standalone main, and we'll move it around as needed
#ifndef MINIMAX_MAIN_H_INCLUDED
#define MINIMAX_MAIN_H_INCLUDED

#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include <memory>
#include <unordered_map>
#include "plat_minimax_main.h"


namespace minimax {
  // types
  // node_id_t needs to be able to hold all possible node IDs uniquely
  typedef uint16_t node_id_t;

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

      //node ID, for making graphs and stuff
      node_id_t node_id;

    public:
      static node_id_t next_node_id;


    public:
      MinimaxNode() { node_id = MinimaxNode::next_node_id++; }   //probably a better and thread-safe way to do ID
      MinimaxNode(index_t dep) : MinimaxNode() { depth = dep; } 
      virtual ~MinimaxNode() {} 

      //accessors ----------------------------------------------------------------------------------------------
      bool is_max_node() { return max_node; }
      void set_max_node(bool numax) { max_node = numax; }

      index_t get_depth() { return depth; }
      void set_depth(index_t d) { depth = d; }

      node_id_t get_node_id() { return node_id; }

    public:
      virtual node_value_t heuristic_value() { return 0; }
      virtual bool is_terminal() { return true; }
      
      //find all the legal countermoves to this state - should it be all *possible* countermoves?
      virtual void find_legal_countermoves(bool is_max, std::vector<std::unique_ptr<MinimaxNode>> &rv) { 
        rv.clear();
      }

      // for debugging and testing
      virtual std::string to_string() { return ""; }
      virtual void print_node(index_t max_depth) {}

  };

  class MinimaxGraphNode {
    public:
      //show this node's relationship to children with e.g.
      //N11 -> { N17, N18, N19 }
      bool max;
      node_id_t id;
      std::vector<node_id_t> children;
      index_t depth;
      std::string tooltip;
    
    public:
      MinimaxGraphNode() {}
      MinimaxGraphNode(node_id_t nid, bool mx, index_t dep) : MinimaxGraphNode() {
        id = nid;
        max = mx;
        depth = dep;
      }

      std::string get_name() {
        char nam[32]; 
        sprintf(nam,"N%d",id);
        return std::string(nam);
      }

      //I expect to fill this in with node.to_string()
      void set_tooltip(std::string tip) { tooltip = tip; }

      void add_child(node_id_t kid) { children.push_back(kid); }
      std::vector<node_id_t> &get_children() { return children; }
  };

  class MinimaxRunner {
    protected:
      //maximum depth of descent, huge or known larger than max possible depth for unlimited
      index_t max_depth = 0;

      // build a DOT (graphviz) graph if this is true - for debugging
      bool building_graph = false;
      std::unordered_map<node_id_t,std::shared_ptr<MinimaxGraphNode>> graphnodes;
      node_id_t root_node_id;     //need to memorize this to fish it out of graphnodes to start the graph
      
      

    public:
      MinimaxRunner() {}
      MinimaxRunner(index_t md, bool bg) : MinimaxRunner() { 
        max_depth = md; 
        building_graph = bg; 
      }
      virtual ~MinimaxRunner() {}

      index_t get_max_depth() { return max_depth; }
      void set_max_depth(index_t nm) { max_depth = nm; }

      bool is_building_graph() { return building_graph; }
      void set_building_graph(bool bg) { building_graph = bg; }

      node_id_t get_root_node_id() { return root_node_id; }
      void set_root_node_id(node_id_t rni) { root_node_id = rni; }

    public:
      virtual node_value_t alphabeta(MinimaxNode &node, index_t depth, node_value_t alpha, 
                                      node_value_t beta, bool is_max);
      // emit alpha-beta graph to a file - default implementation to a .dot file for graphviz to make
      // an svg with mouseover tooltips. set_building_graph(true) has to have been called before
      // alphabeta for this to work
      virtual void render_graph_aux(std::vector<node_id_t> &gnids, FILE *fp);    //recursive auxiliary function
      virtual void render_graph(std::string filepath);                                      
  };

  class CribbageCountNode : public MinimaxNode {
    protected:
      //what goes in a node representing a cribbage count state?
      // It should reflect a *state*. Initial version had this card_to_play thing which makes it look weird.
      // see the tic-tac-toe example at https://www.neverstopbuilding.com/blog/minimax - given an initial state
      // where max player X is generating the next layer of nodes, the next layer shows after X has made their
      // move and the score is from X's point of view.

      // winning the game is worth a lot. We'll decide how much
      node_value_t GAME_WIN_VALUE = 10000;

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

      // - stack total so far, for ease of computing legality of countermoves
      index_t stackTotal;

      // - actual game scores of the max player and min player to this point! if a move makes a player's
      //   score go over 120, they win the game.
      index_t max_player_gamescore;
      index_t min_player_gamescore;

      // flag for whether we've already calculated cumulative score, so we don't call play_card more than once
      // for a given node and don't need to call it during child expansion
      bool calculated_score_yet;
    public:
      // ctor / dtor -------------------------------------------------------------------------------------

      CribbageCountNode() { 
        cumulativeScore = 0;
        calculated_score_yet = false;
        stackTotal = 0; 
        max_player_gamescore = 0;
        min_player_gamescore = 0;
      }
      CribbageCountNode(index_t dp, bool mn, node_value_t cs, index_t st, index_t mxpgs, index_t mnpgs,
                        std::vector<card_t> &sc, std::vector<card_t> &hc, 
                        std::array<index_t,13> &rrc) : CribbageCountNode() {
        depth = dp;
        max_node = mn;
        cumulativeScore = cs;
        max_player_gamescore = mxpgs;
        min_player_gamescore = mnpgs;
        stackcards = sc;
        stackTotal = st;
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
      std::string to_string() override;
      void print_node(index_t max_depth) override;
  };
}

#endif
