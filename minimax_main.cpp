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
    std::vector<std::unique_ptr<MinimaxNode>> children;
    node.find_legal_countermoves(!is_max,children);

    // if maximizingPlayer then
    if(is_max) {
      //     value := −∞
      val = min_node_value; 

      //     for each child of node do
      //         value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
      //         α := max(α, value)
      //         if α ≥ β then
      //             break (* β cut-off *)
      for(auto j = 0; j < children.size(); j++) {
        // **************************************************************************************************
        // HEY IN HERE SOMEWHERE DO A PLATSPEC CALL TO BUILD A GRAPH? or do like with build_lists in scoring?
        // **************************************************************************************************
        // do we need to pass alpha and beta back? I'm going to try not.
        val = std::max(alpha, alphabeta(*(children[j]), depth-1, alpha, beta, false));
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
      for(auto j = 0; j < children.size(); j++) {
        // **************************************************************************************************
        // HEY IN HERE SOMEWHERE DO A PLATSPEC CALL TO BUILD A GRAPH? or do like with build_lists in scoring?
        // **************************************************************************************************
        // do we need to pass alpha and beta back? I'm going to try not.
        val = std::min(beta, alphabeta(*(children[j]), depth-1, alpha, beta, true));
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
    //ok, so think about this.
    //max node's children are the results of max plays - see https://www.neverstopbuilding.com/blog/minimax
    //so I think I can stick with card_to_play and just change how they enumerate.

    //what makes a terminal node? 

    //we have a card to play, a stack to play it on.
    //error_card_val is root case - assume that's not terminal?
    if(card_to_play == cu.ERROR_CARD_VAL) return false;
    
    //otherwise - figure out total of stack so far - same way as in play_card
    index_t curtotal = std::accumulate(
      stackcards.begin(), stackcards.end(), 0,
      [this](index_t a, card_t b) { return a + index_t(cu.val(b)); });
    if(curtotal + cu.val(card_to_play) > 31) {
      //total with card to play being > 31 means it's illegal
      return true;
    }

    //otherwise, if there are no cards left in the hand, it's a terminal node
    //wait, is this always so? On min nodes too? Maybe on min nodes we need to be looking at if there
    //are any rankcards left...???
    // FIGURE THIS OUT ********************************************************************************
    if(handcards.empty()) return true;

    return false;
  }

  node_value_t CribbageCountNode::heuristic_value() {
    //ok! So what's the value of a cribbage count node?
    index_t playscore = 0;
    if(card_to_play != cu.ERROR_CARD_VAL) {
      playscore = cr.play_card(stackcards,card_to_play,nullptr,false,false);
      if(playscore != cu.ERROR_CARD_VAL) {
        //we have to know if this is a max or min node. if max, add to cumulative
        //if min, subtract from it
        if(max_node) {
          cumulativeScore += playscore;
        } else {
          cumulativeScore -= playscore;
        }
      }
    }

    return cumulativeScore;
  }

  //WOULD BE NICE TO DO THIS AS A GENERATOR AND NOT NEED TO CREATE ALL THE NODES
  //but in this game's case the fan-out is never worse than 13
  void CribbageCountNode::find_legal_countermoves(bool is_max, std::vector<std::unique_ptr<MinimaxNode>> &rv) {
    //std::vector<MinimaxNode> rv;
    rv.clear();
    std::vector<card_t> nu_stack, nu_hand;
    std::array<index_t,13> nu_rrc;
    nu_stack.reserve(8);      //shouldn't get longer than 8
    nu_hand.reserve(4);       //player's hand no more than 4

    //stack all children will play on to will be this's stack + this's card_to_play
    //wait not sure re: that
    std::copy(stackcards.begin(), stackcards.end(), nu_stack.begin());
    //root condition: card_to_play is none...???
    if(card_to_play != cu.ERROR_CARD_VAL) nu_stack.push_back(card_to_play);

    //create the child nodes! This is all possible, not all legal.
    //so wait - if we're at a max node, the children are the nodes that arise from each of max's moves, yes?
    //so we want the state that arises from playing card 1, card 2, card 3, card 4
    //at the root.
    //so maybe the thing to do is not have a card_to_play but a card_played
    
    if(is_max) {
      //generate the moves that max player can make
      // remaining rank counts will be the same for all children, same as this's
      std::copy(remainingRankCounts.begin(), remainingRankCounts.end(), nu_rrc.begin());

      // iterate through handcards and create nodes that have each one as card_to_play
      // and that card removed from handcards
      for(auto i = 0; i < handcards.size(); i++) {
        nu_hand.clear();
        std::copy(handcards.begin(),handcards.end(), std::back_inserter(nu_hand));
        nu_hand.erase(nu_hand.begin()+i);   // hand with ith card removed
        //beware object slicing! https://stackoverflow.com/questions/8777724/store-derived-class-objects-in-base-class-variables
        rv.emplace_back(new CribbageCountNode(depth-1, false,cumulativeScore,nu_stack,nu_hand, nu_rrc, handcards[i]));
      }
    } else {
      //hand going in is this's hand, opponent is playing
      nu_hand.clear();
      std::copy(handcards.begin(), handcards.end(), std::back_inserter(nu_hand));

      //generate the moves that this min node can make
      // so our loop is over the 13 ranks, and if any of that rank is left in remainingRankCounts,
      // play it and hand in a rankcounts with its count decremented.
      for(card_t j = 0; j< 13; j++) {
        //if any cards of rank j remain, add one as a possible countermove
        if(remainingRankCounts[j] > 0) {
          std::copy(remainingRankCounts.begin(),remainingRankCounts.end(),nu_rrc.begin());
          //COULD DO THE LEGAL CHECK HERE AND NOT EVEN PROPOSE ILLEGAL NODES but see about that
          nu_rrc[j]--;  //lower next generation's count of this card by 1
          //card value is j<<2 so it acts like a rank (suit is unimportant)
          //beware object slicing! https://stackoverflow.com/questions/8777724/store-derived-class-objects-in-base-class-variables
          rv.emplace_back(new CribbageCountNode(depth-1, true,cumulativeScore,nu_stack,nu_hand, nu_rrc, j<<2));
        }
      }
    }


    //return rv;
  }

  void CribbageCountNode::print_node(index_t max_depth) {
    std::string indent((max_depth - depth)*2,' ');
    bool term = is_terminal();      //ugh, side effect sets cumulative value - let's fix that
    node_value_t val = heuristic_value();
    plprintf("%smax: %s depth: %d term: %s cscore: %2d card_to_play: %s\n",indent.c_str(), max_node?"Y":"N", depth, 
              term?"Y":"N", int(cumulativeScore), card_to_play == cu.ERROR_CARD_VAL?"(root)":(cu.cardstring(card_to_play)).c_str());
    //print rank headings, then stack
    plprintf("%sA234567890JQK stack: ",indent.c_str());
    if(stackcards.empty())
      plprintf("empty");
    else
      for(auto j = 0; j < stackcards.size(); j++) plprintf("%s ", (cu.cardstring(stackcards[j])).c_str());
    //print rank counts, then hand
    plprintf("\n%s",indent.c_str());
    for(auto j = 0; j < 13; j++) plprintf("%d",remainingRankCounts[j]);
    plprintf(" hand: ");
    if(handcards.empty()) 
      plprintf("empty");
    else
      for(auto j = 0; j < handcards.size(); j++) plprintf("%s ", (cu.cardstring(handcards[j])).c_str());
    plprintf("\n%s---------------------------------------------------\n",indent.c_str());
  }
}

using namespace minimax;

//builds a CribbageCountNode to act as a root in tests
//returns false if the scenario is inconsistent, like there are five kings among the hand and stack
//also more subtle illegalities like too long a hand or stack
//...this should be a method in one of the classes maybe
bool build_scenario(bool max, index_t depth, node_value_t cumuscore, std::string playcardstr, 
                    std::vector<std::string> player_cardstrings, 
                    std::vector<std::string> stack_cardstrings, CribbageCountNode &node) {

  //check for illegal length of hand or stack
  if(player_cardstrings.size() > 4) {
    //too long a hand
    return false;
  }

  if(stack_cardstrings.size() > 8) {
    //too long a stack
    return false;
  }

  //convert playcardstr into a card value; ok if it's illegal - that means root
  card_t playcard = cu.stringcard(playcardstr);

  //start with full deck remaining in 
  std::array<index_t,13> start_rrc = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
  // convert the player card strings to cards, then to ranks, likewise stack cards
  // actually we want ranks << 2, to align with play_card's expectation of cards with suits
  std::vector<card_t> handcards(player_cardstrings.size());
  std::transform(player_cardstrings.begin(), player_cardstrings.end(), handcards.begin(),
                [](std::string c) { return cu.rank(cu.stringcard(c)) << 2; });

  std::vector<card_t> stackcards(stack_cardstrings.size());
  std::transform(stack_cardstrings.begin(), stack_cardstrings.end(), stackcards.begin(),
                [](std::string c) { return cu.rank(cu.stringcard(c)) << 2; });

  //if there are any ERROR_CARD_VAL in those, fail
  if(std::find(std::begin(handcards), std::end(handcards), cu.ERROR_CARD_VAL) != std::end(handcards)) {
    //illegal card in hand
    return false;
  }

  if(std::find(std::begin(stackcards), std::end(stackcards), cu.ERROR_CARD_VAL) != std::end(stackcards)) {
    //illegal card in stack
    return false;
  }

  // take the ranks currently used by handcards and stackcards out of start_rrc
  for(card_t c : handcards) start_rrc[cu.rank(c)]--;
  for(card_t c : stackcards) start_rrc[cu.rank(c)]--;

  //check to see if any ranks have gone negative (more than 4 cards of one rank in the scenario)
  for(index_t ct : start_rrc) {
    if(ct < 0 || ct > 4) {
      //the > 4 case is bc index_t can be unsigned
      return false;
    }
  }

  //and so finally, build our node
  node = CribbageCountNode(depth, max, cumuscore, stackcards, handcards, start_rrc, playcard);  


  return true;

}

// main method ------------------------------------------------------------------------------------------
void run() {
  plprintf("Hello and welcome to MINIMAX_MAIN, the minty minimax fiddlement utility.\n");

  //try a max depth of 9 - should never reach that far
  index_t max_depth = 9;
  MinimaxRunner mr = MinimaxRunner(max_depth);

  //OK! So set up a cribbage hand in this thing's terms and see what it thinks the best play is!
  //let's start with our hand is ... and we only care about ranks ...
  //5, 6, Q, J - which is 4, 5, 11, 10 in zero-relative rank, and shift left by 2 to align as though they had suits
  //just at a guess
  // GENERALIZE INTO A SCENARIO BUILDING FUNCTION ===================================================================


  /* original by-hand way
  std::vector<card_t> player_hand = { 4 << 2, 5 << 2, 11 << 2, 10 << 2 };
  
  //here's the list of remaining (zero-rel) ranks - all 13 have 4 cards left except for 4, 5, 10, 11
  std::array<index_t,13> start_rrc = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 };
  start_rrc[4]--;
  start_rrc[5]--;
  start_rrc[10]--;
  start_rrc[11]--;

  //OK ONE WEIRDNESS, ROOT NODE DOESN'T REALLY HAVE A CARD TO PLAY
  //THINK RE HOW TO DO THIS ************************************************************************** 
  //    CribbageCountNode(bool mn, node_value_t cs, std::vector<card_t> &sc, std::vector<card_t> &hc, 
  //                      std::array<index_t,13> &rrc, card_t ctp) : CribbageCountNode() 
  std::vector<card_t> stackcards;    //starts empty
  CribbageCountNode root = CribbageCountNode(mr.get_max_depth(), true, node_value_t(0), 
                                              stackcards, player_hand, start_rrc, cu.ERROR_CARD_VAL);
  which got
  Root node:
  max: Y depth: 9 term: N cscore:  0 card_to_play: (root)
  A234567890JQK stack: empty
  4444334444334 hand: 5h 6h Qh Jh 
  ---------------------------------------------------
  Children of root:
    max: N depth: 8 term: N cscore:  0 card_to_play: 5h
    A234567890JQK stack: empty
    4444334444334 hand: 6h Qh Jh 
    ---------------------------------------------------
    max: N depth: 8 term: N cscore:  0 card_to_play: 6h
    A234567890JQK stack: empty
    4444334444334 hand: 5h Qh Jh 
    ---------------------------------------------------
    max: N depth: 8 term: N cscore:  0 card_to_play: Qh
    A234567890JQK stack: empty
    4444334444334 hand: 5h 6h Jh 
    ---------------------------------------------------
    max: N depth: 8 term: N cscore:  0 card_to_play: Jh
    A234567890JQK stack: empty
    4444334444334 hand: 5h 6h Qh 
    ---------------------------------------------------
  Children of first child of root:
      max: Y depth: 7 term: N cscore:  0 card_to_play: Ah
      A234567890JQK stack: 5h 
      3444334444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 2h
      A234567890JQK stack: 5h 
      4344334444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 3h
      A234567890JQK stack: 5h 
      4434334444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 4h
      A234567890JQK stack: 5h 
      4443334444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  2 card_to_play: 5h
      A234567890JQK stack: 5h 
      4444234444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 6h
      A234567890JQK stack: 5h 
      4444324444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 7h
      A234567890JQK stack: 5h 
      4444333444334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 8h
      A234567890JQK stack: 5h 
      4444334344334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  0 card_to_play: 9h
      A234567890JQK stack: 5h 
      4444334434334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  2 card_to_play: 0h
      A234567890JQK stack: 5h 
      4444334443334 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  2 card_to_play: Jh
      A234567890JQK stack: 5h 
      4444334444234 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  2 card_to_play: Qh
      A234567890JQK stack: 5h 
      4444334444324 hand: 6h Qh Jh 
      ---------------------------------------------------
      max: Y depth: 7 term: N cscore:  2 card_to_play: Kh
      A234567890JQK stack: 5h 
      4444334444333 hand: 6h Qh Jh 
      ---------------------------------------------------                                                
  */

  //CribbageCountNode root = CribbageCountNode(mr.get_max_depth(), true, node_value_t(0), 
  //                                            stackcards, player_hand, start_rrc, cu.ERROR_CARD_VAL);

  std::vector<std::string> player_cardstrings = { "5d", "6s", "Qh", "Jc" };
  std::vector<std::string> stack_cardstrings;   //empty
  CribbageCountNode root;
  bool res = build_scenario(true, mr.get_max_depth(), 0, "", player_cardstrings, 
                            stack_cardstrings, root); 
  if(res == true) {

    plprintf("Root node:\n");
    root.print_node(max_depth);

    std::vector<std::unique_ptr<MinimaxNode>> kids;
    plprintf("Children of root:\n");
    root.find_legal_countermoves(root.is_max_node(),kids);
    for(auto j = 0; j < kids.size(); j++) {
      kids[j]->print_node(max_depth);
    }

    //let's look at children of last kid
    std::vector<std::unique_ptr<MinimaxNode>> kids2;
    plprintf("Children of first child of root:\n");
    kids[0]->find_legal_countermoves(kids[0]->is_max_node(),kids2);
    for(auto j = 0; j < kids2.size(); j++) {
      kids2[j]->print_node(max_depth);
    }
  } else {
    plprintf("Failed to set up root scenario!");
  }
}