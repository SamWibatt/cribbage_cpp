//minimax.cpp! Let's try doing some AI stuff
// for now, let's just do it as a standalone main, and we'll move it around as needed
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include "minimax_main.h"
#include "plat_minimax_main.h"

CardUtils cu;
Cribbage cr;


namespace minimax {
  /*
  Right so
  Let us bull on and write this, like I did with the Python implementations! I've gotten ossified.
  Go nuts and have fun! Celebrate the fact that I have at least a basic knowledge of C++11, CMake,
  Google test, and code coverage! CELEBRATE! LEARN! APPLY!

  So here is code for minimax with alpha-beta. From wikipedia!

  function alphabeta(node, depth, α, β, maximizingPlayer) is
      if depth = 0 or node is a terminal node then
          return the heuristic value of node
      if maximizingPlayer then
          value := −∞
          for each child of node do
              value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
              α := max(α, value)
              if α ≥ β then
                  break (* β cut-off *)
          return value
      else
          value := +∞
          for each child of node do
              value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
              β := min(β, value)
              if β ≤ α then
                  break (* α cut-off *)
          return value

  (* Initial call *)
  alphabeta(origin, depth, −∞, +∞, TRUE)

  in our case it's
  index_t maxd = 9; 
  set_max_depth(maxd);
  alphabeta(root, maxd, min_node_value, max_node_value, true)
  */

  node_value_t MinimaxRunner::alphabeta(MinimaxNode &node, index_t depth, node_value_t alpha, node_value_t beta, bool is_max) {
    node_value_t val;

    // if depth = 0 or node is a terminal node then
    //     return the heuristic value of node
    if(depth == 0 || node.is_terminal()) { return node.heuristic_value(); }

    // slightly confused here - if this is a max node, the children are min nodes & vv, yes? hence !is_max
    std::vector<MinimaxNode> children = node.find_legal_countermoves(!is_max);

    // if maximizingPlayer then
    if(is_max) {
      //     value := −∞
      val = min_node_value; 

      //     for each child of node do
      //         value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
      //         α := max(α, value)
      //         if α ≥ β then
      //             break (* β cut-off *)
      for(MinimaxNode n : children) {
        // do we need to pass alpha and beta back? I'm going to try not.
        val = std::max(alpha, alphabeta(n, depth-1, alpha, beta, false));
        alpha = std::max(alpha, val);
        if(alpha >= beta) break;    //beta cutoff
      }

      //     return value
      return val; 
    } else {
      //     value := +∞
      val = max_node_value;

      //     for each child of node do
      //         value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
      //         β := min(β, value)
      //         if β ≤ α then
      //             break (* α cut-off *)
      for(MinimaxNode n : children) {
        // do we need to pass alpha and beta back? I'm going to try not.
        val = std::min(beta, alphabeta(n, depth-1, alpha, beta, true));
        alpha = std::min(alpha, val);
        if(alpha >= beta) break;    //beta cutoff
      }
    }
    //shouldn't be reachable, but this shuts compiler warning up
    return val;
  }


  //CribbageCountNode implementations =================================================================================================
  CribbageCountNode::CribbageCountNode() {
    //ctor stuff
  }

  CribbageCountNode::~CribbageCountNode() {
    //dtor stuff
  }

  bool CribbageCountNode::is_terminal() {
    //what makes a terminal node? When a node is terminal, we return its heuristic value,
    //so there needs to be a card to play and no cards left in the hand.
    //return handcards.empty();
    //wait, rethink. That's still true, but not the only condition
    //this is always called before heuristic_value, so we could have other conditions;
    //could do the call to play_card here and if it's an error, then what?
    //let's say that's a terminal node. That doesn't let us play all the way to the end,
    //just to 31, and... let's go with that

    //we have a card to play, a stack to play it on.
    //we don't want score lists, do we want to add to stack? Prolly not
    index_t playscore = cr.play_card(stackcards,card_to_play,nullptr,false,false);  

    //so if that isn't a legal move, this is a terminal node, no change to cumulative score
    if(playscore == cr.ERROR_SCORE_VAL) return true;

    //HERE DO SOMETHING WITH CUMULATIVE_SCORE...............
    //add if we're maxing, subtract if we're minning?
    //or does that even make sense?
    //FIGURE THIS OUT ************************************************************************
    //make it so that returning cumulative value is the heuristic value
    //we have to know if this is a max or min node. if max, add to cumulative
    //if min, subtract from it
    if(max_node) {
      cumulativeScore += playscore;
    } else {
      cumulativeScore -= playscore;
    }

    //otherwise, if there are no cards left in the hand, it's a terminal node
    if(handcards.empty()) return true;

    return false;
  }

  node_value_t CribbageCountNode::heuristic_value() {
    //ok! So what's the value of a cribbage count node?
    //since is_terminal did the calc, cumulativeScore is all we need
    return cumulativeScore;
  }

  std::vector<MinimaxNode> CribbageCountNode::find_legal_countermoves(bool is_max) {
    std::vector<MinimaxNode> rv;

    //OK HERE IS THE LAST BIT I NEED TO WRITE
    if(is_max) {
      // it's a max node, we're generating the min nodes that follow.
      // so... hm. what's the equivalent to handcards for the min case? We need to step through
      // that. It is: opponentcards!
    } else {
      // it's a min no de, generate the max nodes that follow.
      // iterate through handcards and create nodes that have each one as card_to_play
      // and that card removed from handcards
    }

    return rv;
  }
}

// main method ------------------------------------------------------------------------------------------
void run() {
  plprintf("Hello and welcome to MINIMAX_MAIN, the minty minimax fiddlement utility.\n");
}