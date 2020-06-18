//minimax.cpp! Let's try doing some AI stuff
// for now, let's just do it as a standalone main, and we'll move it around as needed
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include "minimax_main.h"
#include "plat_minimax_main.h"

//TEMP! until I refactor file printing for graph into platform-specific, which will 
//almost certainly be stubbed to nothing on arduino - but MAYBE NOT. I suppose I could write
//things to spi flash or some such.
#include <cstdio>

CardUtils cu;
Cribbage cr;


namespace minimax {

  //initialize static member of minimaxnode
  node_id_t MinimaxNode::next_node_id = 0;

  node_value_t MinimaxRunner::alphabeta(MinimaxNode &node, index_t depth, node_value_t alpha, node_value_t beta, bool is_max) {
    node_value_t val;

    std::string indent((max_depth - depth)*2,' ');    //DEBUG

    // if depth = 0 or node is a terminal node then
    //     return the heuristic value of node
    if(depth == 0 || node.is_terminal()) {
      node.print_node(max_depth);
      plprintf("%sTerminal node! id %lu value %d\n",indent.c_str(),node.get_node_id(), node.heuristic_value()); 
      return node.heuristic_value(); 
    }

    // slightly confused here - if this is a max node, the children are min nodes & vv, yes? hence !is_max
    // or does this make everything backwards?
    std::vector<std::unique_ptr<MinimaxNode>> children;
    //let's try not negating is_max !!!!!!!!!!!!!!!!!!!!!!!!! that seems to get the stack in the right order
    node.find_legal_countermoves(is_max,children);

    //ok so if we're building a graph, record this node. Will add its children as and if they're explored - will reflect pruning
    //is node's is_max_node the same as is_max?
    //also not sure about the depth here; goes like the indent when printing nodes, 0 = root, think that's ok
    //how do I avoid creating this if we're not building graph? default ctor isn't much faster
    //work that out
    MinimaxGraphNode cur_graph_node(node.get_node_id(), node.is_max_node(), depth-max_depth);

    // if maximizingPlayer then
    // ...urgh, I think I have this backwards again
    if(is_max) {
      //     value := −∞
      val = min_node_value; 

      //     for each child of node do
      //         value := max(value, alphabeta(child, depth − 1, α, β, FALSE))
      //         α := max(α, value)
      //         if α ≥ β then
      //             break (* β cut-off *)
      plprintf("%sMaxnode depth %d #kids %d\n",indent.c_str(),depth,children.size());
      // see what we're getting down at the leaves - I think I'm screwing up the is_terminal spotting
      if(children.empty()){
        node.print_node(max_depth);
        plprintf("%sreached no-legal-moves leaf, id %lu val %d\n",indent.c_str(), node.get_node_id(), node.heuristic_value());
        return node.heuristic_value();
      }
      for(auto j = 0; j < children.size(); j++) {
        // here do like with build_lists in scoring to generate a graph node for debugging - actually here all we
        // need is to associate the child with current node
        if(building_graph) cur_graph_node.add_child(children[j]->get_node_id());

        // do we need to pass alpha and beta back? i.e., make them references? I'm going to try not.
        val = std::max(alpha, alphabeta(*(children[j]), depth-1, alpha, beta, false));
        plprintf("%sMaxnode id %lu child %d val: %d\n",indent.c_str(), node.get_node_id(), j,val);
        alpha = std::max(alpha, val);
        plprintf("%sMax node! Alpha now %d, beta %d\n",indent.c_str(),int(alpha), int(beta));
        if(alpha >= beta) {
          plprintf("%sbeta cutoff!\n",indent.c_str());
          break;    //beta cutoff
        }
      }

      //     return value
      plprintf("%sMaxnode id %lu final val %d\n",indent.c_str(), node.get_node_id(), val);
      return val; 
    } else {
      //     value := +∞
      val = max_node_value;

      //     for each child of node do
      //         value := min(value, alphabeta(child, depth − 1, α, β, TRUE))
      //         β := min(β, value)
      //         if β ≤ α then
      //             break (* α cut-off *)
      plprintf("%sMinnode depth %d #kids %d\n",indent.c_str(),depth,children.size());
      // see what we're getting down at the leaves - I think I'm screwing up the is_terminal spotting
      // sure enough, *this* is a case of a terminal that is_terminal doesn't catch. bc it doesn't want to
      // calculate how many children a node will have
      if(children.empty()){
        node.print_node(max_depth);
        plprintf("%sreached no-legal-moves leaf, id %lu val %d\n",indent.c_str(),node.get_node_id(),node.heuristic_value());
        return node.heuristic_value();
      }
      for(auto j = 0; j < children.size(); j++) {
        // here do like with build_lists in scoring to generate a graph node for debugging - actually here all we
        // need is to associate the child with current node
        if(building_graph) cur_graph_node.add_child(children[j]->get_node_id());

        // do we need to pass alpha and beta back? I'm going to try not.
        val = std::min(beta, alphabeta(*(children[j]), depth-1, alpha, beta, true));
        plprintf("%sMinnode id %lu child %d val: %d\n",indent.c_str(),node.get_node_id(),j,val);
        beta = std::min(beta, val);
        plprintf("%sMin node! Alpha now %d, beta %d\n",indent.c_str(),int(alpha), int(beta));
        if(beta <= alpha) {
          plprintf("%salpha cutoff!\n",indent.c_str());
          break;    //alpha cutoff
        }
      }
      plprintf("%sMinnode id %lu final val %d\n",indent.c_str(), node.get_node_id(), val);

      //and finally, add current graph node to our list of nodes if we're doing that
      if(building_graph) graphnodes[node.get_node_id()] = cur_graph_node;

      return val;
    }
  }

  //if alphabeta was called with build_graph true, 
  void MinimaxRunner::render_graph(std::string filepath) {
    //let's only do this if there is a graph
    if(graphnodes.empty()) {
      plprintf("render_graph: No graph nodes found!\n");
      return;
    }

    // ok now here's a thing - do I write platspec I/O for files, too? I suppose I should :P
    // put that in as an issue - let's just do it linux-style for now and then refactor into platspec, like I did with printf
    // here, pop open the file we're writing to
    FILE* fp = std::fopen(filepath.c_str(), "wt");
    if(fp == nullptr) {
        plprintf("Graph file opening failed\n");
        return;
    }

    // so how to traverse?
    // sounds like recurse!
    node_id_t cur_node_id = root_node_id;

    // WRITE THE GUTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // WRITE THE GUTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // WRITE THE GUTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    std::fclose(fp);
  }


  //CribbageCountNode implementations =================================================================================================

  CribbageCountNode::~CribbageCountNode() {
    //dtor stuff
  }


  bool CribbageCountNode::is_terminal() {
    //ok, so think about this.
    //max node's children are the results of max plays - see https://www.neverstopbuilding.com/blog/minimax

    //what makes a terminal node? If the stack total is > 31, it should never have existed
    //if total is == 31, this is terminal regardless of player
    if(stackTotal == 31) return true;

    //if it's a max node and there are no cards left in hand, it's terminal
    //if it's a min node and there is nothing left in remaining ranks, it's terminal
    //or perhaps that's backwards
    if(is_max_node()) return handcards.empty();
    //so if max element in remainingRankCounts has a value of 0, they're all 0 and therefore nothing left
    auto maxy = std::max_element(remainingRankCounts.begin(), remainingRankCounts.end());
    return *maxy == 0;

    //THIS DOESN'T CATCH THE CASE THAT THERE ARE NO LEGAL MOVES FROM HERE - the runner itself will have to look for that!
    //or if I rewrite the find_legal_countermoves to be the stripped down version that doesn't build out the states, this could
    //run that or rely on its having been run elsewhere?
  }

  node_value_t CribbageCountNode::heuristic_value() {
    //ok! So what's the value of a cribbage count node?
    //post card-to-play, I think it's just the cumulative score
    // OR,
    // could do the heuristic evaluation by treating the last member of the stack as card_to_play
    // if there is no stack, return 0
    if(stackcards.empty()) return 0;

    // CACHE THIS SO WE DON'T END UP CALLING THE PLAY_CARD MORE THAN ONCE
    // ...also bc this is destructive and would change the node's value!
    if(calculated_score_yet == false) {
      card_t card_to_play = stackcards.back();
      //remember then the stack it's playing ONTO doesn't include it
      stackcards.pop_back();
      index_t playscore = 0;
      //last flag true to add the card back on to the stack - gross, but wev
      playscore = cr.play_card(stackcards,card_to_play,nullptr,false,true);
      if(playscore != cu.ERROR_CARD_VAL) {
        //check to see if this puts the player over the line!
        //used to have opposite sense of max and min
        if(!max_node) {
          //it's the result of a min player's play, yes? so the playscore gets added to min player's score
          min_player_gamescore += playscore;
          //if that wins the game for min player, BIG YIKES
          if(min_player_gamescore > 120) {
            cumulativeScore -= GAME_WIN_VALUE;
          }
        } else {
          // yay, it was a max play that did it and max wins if score over 120
          max_player_gamescore += playscore;
          //if that wins the game for max player, BIG YAY
          if(max_player_gamescore > 120) {
            cumulativeScore += GAME_WIN_VALUE;
          }
        }

        //we have to know if this is a max or min node. if max, add to cumulative
        //if min, subtract from it - this might be backwards - indeed it seems to be.
        //so - if *this* is a max node, the *move* came from a min node... yes? Will figure out soon
        //gone back on that, the runner was getting the sense backwards, see if this is better
        //nope, back again
        if(max_node) {
          cumulativeScore -= playscore;
        } else {
          cumulativeScore += playscore;
        }
      } else {
        //card was an illegal play! Should this be possible? Let's say not
        //ERROR
      }
      calculated_score_yet = true;
    }

    return cumulativeScore;
  }

  //WOULD BE NICE TO DO THIS AS A GENERATOR AND NOT NEED TO CREATE ALL THE NODES
  //but in this game's case the fan-out is never worse than 13
  //COULD DO THAT where this is a thing that returns a list of the
  //actual *moves* but doesn't generate the states, and there's
  //a get-state-from-move function that generates the states as needed.
  //not a huge problem but would be an efficiency thing for speed a little bit
  //and space somewhat more
  void CribbageCountNode::find_legal_countermoves(bool is_max, std::vector<std::unique_ptr<MinimaxNode>> &rv) {
    //std::vector<MinimaxNode> rv;
    rv.clear();
    std::vector<card_t> nu_stack, nu_hand;
    std::array<index_t,13> nu_rrc;
    nu_stack.reserve(8);      //shouldn't get longer than 8
    nu_hand.reserve(4);       //player's hand no more than 4

    //create the child nodes! This is all possible, not all legal.
    //should we do only legal? Sounds like a good idea

    //if we're at a max node, the children are the nodes that arise from each of max's moves, yes?
    //so we want the state that arises from playing card 1, card 2, card 3, card 4
    //at the root.

    //stack all children will play on to will be this's stack, each will add its own card
    std::copy(stackcards.begin(), stackcards.end(), std::back_inserter(nu_stack));

    // find out current value of stack to spot legal moves - no move that makes total > 31 is legal

    if(is_max) {
      //generate the moves that max player can make
      // remaining rank counts will be the same for all children, same as this's
      std::copy(remainingRankCounts.begin(), remainingRankCounts.end(), nu_rrc.begin());

      // iterate through handcards and create nodes that have each one as card_to_play
      // and that card removed from handcards
      for(auto i = 0; i < handcards.size(); i++) {
        //figure out if handcards[i] is a legal play: any card that makes the total > 31 is illegal
        index_t playTotal = stackTotal + index_t(cu.val(handcards[i]));
        if (playTotal <= 31) {
          nu_hand.clear();
          std::copy(handcards.begin(),handcards.end(), std::back_inserter(nu_hand));
          nu_hand.erase(nu_hand.begin()+i);   // hand with ith card removed
          nu_stack.push_back(handcards[i]);   // add card being played to the stack
          //beware object slicing! https://stackoverflow.com/questions/8777724/store-derived-class-objects-in-base-class-variables
          rv.emplace_back(new CribbageCountNode(depth-1, false,cumulativeScore, playTotal, max_player_gamescore, min_player_gamescore,
              nu_stack,nu_hand, nu_rrc));
          nu_stack.pop_back();                // restore stack for next child along
        }
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
          //figure out if j<<2 is a legal play: any card that makes the total > 31 is illegal
          //card value is j<<2 so it acts like a rank (suit is unimportant)
          index_t playTotal = stackTotal + index_t(cu.val(j << 2));
          if (playTotal <= 31) {
            std::copy(remainingRankCounts.begin(),remainingRankCounts.end(),nu_rrc.begin());
            nu_rrc[j]--;  //lower next generation's count of this card by 1

            nu_stack.push_back(j<<2);

            //beware object slicing! https://stackoverflow.com/questions/8777724/store-derived-class-objects-in-base-class-variables
            rv.emplace_back(new CribbageCountNode(depth-1, true,cumulativeScore, playTotal, max_player_gamescore, min_player_gamescore,
              nu_stack,nu_hand, nu_rrc));
            nu_stack.pop_back();                // restore stack for next child along
          }
        }
      }
    }
  }

  std::string CribbageCountNode::to_string() {
    char buf[128];
    std::string outstr;
    sprintf(buf,"id: %u max: %s dep: %d trm: %s\nstot: %2d val: %2d mxs: %3d mns: %3d\n", 
            uint32_t(node_id), max_node?"Y":"N", depth, is_terminal()?"Y":"N", stackTotal, 
            heuristic_value(), max_player_gamescore, min_player_gamescore);
    outstr.append(buf);

    //hand and stack
    outstr.append("hand: ");
    if(handcards.empty()) 
      outstr.append("empty");
    else
      for(auto j = 0; j < handcards.size(); j++) {
        outstr.append(cu.cardstring(handcards[j]));
        outstr += ' ';
      }

    outstr.append(" stack: ");
    if(stackcards.empty())
      outstr.append("empty");
    else
      for(auto j = 0; j < stackcards.size(); j++) {
        outstr.append(cu.cardstring(stackcards[j]));
        outstr += ' ';
      }
    outstr += '\n';

    //remaining rank counts
    std::string ranks = "A234567890JQK";
    for(auto j = 0; j < 13; j++) {
      outstr += ranks[j];
      outstr += '0' + remainingRankCounts[j];
      outstr += ' ';
    }

    return outstr;
  }

  void CribbageCountNode::print_node(index_t max_depth) {
    std::string indent((max_depth - depth)*2,' ');
    bool term = is_terminal();      //ugh, side effect sets cumulative value - let's fix that
    node_value_t val = heuristic_value();
    //print rank headings and some vitals
    plprintf("%sA234567890JQK id: %lu max: %s dep: %d trm: %s stot: %2d val: %2d mxs: %3d mns: %3d\n", 
            indent.c_str(), node_id, max_node?"Y":"N", depth, term?"Y":"N", stackTotal, val, max_player_gamescore, min_player_gamescore);
    //print rank counts, hand, stack
    plprintf("%s",indent.c_str());
    for(auto j = 0; j < 13; j++) plprintf("%d",remainingRankCounts[j]);
    plprintf(" hand: ");
    if(handcards.empty()) 
      plprintf("empty");
    else
      for(auto j = 0; j < handcards.size(); j++) plprintf("%s ", (cu.cardstring(handcards[j])).c_str());
    plprintf(" stack: ",indent.c_str());
    if(stackcards.empty())
      plprintf("empty");
    else
      for(auto j = 0; j < stackcards.size(); j++) plprintf("%s ", (cu.cardstring(stackcards[j])).c_str());
    plprintf("\n%s-----------------------------------------------------------------------\n",indent.c_str());
  }
}

using namespace minimax;

//builds a CribbageCountNode to act as a root in tests
//returns false if the scenario is inconsistent, like there are five kings among the hand and stack
//also more subtle illegalities like too long a hand or stack
//...this should be a method in one of the classes maybe
bool build_scenario(bool max, index_t depth, node_value_t cumuscore, index_t maxplayer_gscore, index_t minplayer_gscore,
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

  //check that player positions are legal
  if(maxplayer_gscore > 120) {
    //...is this wrong?
    return false;
  }

  if(minplayer_gscore > 120) {
    //...is this wrong?
    return false;
  }

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

  // calculate initial stack total
  index_t curtotal = std::accumulate(
    stackcards.begin(), stackcards.end(), 0,
    [](index_t a, card_t b) { return a + index_t(cu.val(b)); });
  
  //if it's > 31, this is an illegal node
  if(curtotal > 31) {
    //total over 31
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
  node = CribbageCountNode(depth, max, cumuscore, curtotal, maxplayer_gscore, minplayer_gscore, stackcards, handcards, start_rrc);  


  return true;

}

// main method ------------------------------------------------------------------------------------------
void run() {
  plprintf("Hello and welcome to MINIMAX_MAIN, the minty minimax fiddlement utility.\n");

  //try a max depth of 9 - should never reach that far
  index_t max_depth = 3; //try a 3 for easier reading - was 9;
  //also the "true" means emit DOT graph
  MinimaxRunner mr = MinimaxRunner(max_depth, true);

  //OK! So set up a cribbage hand in this thing's terms and see what it thinks the best play is!
  //let's start with our hand is ... and we only care about ranks ...
  //5, 6, Q, J, any rank is fine
  std::vector<std::string> player_cardstrings = { "5d", "6s", "Qh", "Jc" };
  std::vector<std::string> stack_cardstrings;   //empty
  CribbageCountNode root;
  //say that the max player is at 50 points, min player at 117
  bool res = build_scenario(true, mr.get_max_depth(), 0, 50, 117, player_cardstrings, stack_cardstrings, root); 
  if(res == true) {

    plprintf("Root node string:\n%s\n",root.to_string().c_str());

    plprintf("Root node:\n");
    root.print_node(max_depth);

    //so let's do an actual minimax! What happens?
    // (* Initial call *)
    // alphabeta(origin, depth, −∞, +∞, TRUE)
    bool build_graph = true;      //for debugging with DOT/svg graph

    mr.set_building_graph(build_graph);     //need to set that before running
    node_value_t bestscore = mr.alphabeta(root, max_depth, min_node_value, max_node_value, true);

    plprintf("Best minimax score found: %d\n",int(bestscore));

    /* old noodle tests
    std::vector<std::unique_ptr<MinimaxNode>> kids;
    plprintf("Children of root:\n");
    root.find_legal_countermoves(root.is_max_node(),kids);
    for(auto j = 0; j < kids.size(); j++) {
      kids[j]->print_node(max_depth);
    }

    //let's look at children of first kid
    std::vector<std::unique_ptr<MinimaxNode>> kids2;
    plprintf("Children of first child of root:\n");
    kids[0]->find_legal_countermoves(kids[0]->is_max_node(),kids2);
    for(auto j = 0; j < kids2.size(); j++) {
      kids2[j]->print_node(max_depth);
    }

    //let's look at children of 2nd to last grandkid
    std::vector<std::unique_ptr<MinimaxNode>> kids3;
    plprintf("Children of 2nd to last child of first child of root:\n");
    kids2[kids2.size()-2]->find_legal_countermoves(kids2[kids2.size()-2]->is_max_node(),kids3);
    for(auto j = 0; j < kids3.size(); j++) {
      kids3[j]->print_node(max_depth);
    }

    //let's look at children of last great grandkid
    std::vector<std::unique_ptr<MinimaxNode>> kids4;
    plprintf("Children of last child of 2nd to last child of first child of root:\n");
    kids3[kids3.size()-1]->find_legal_countermoves(kids3[kids3.size()-1]->is_max_node(),kids4);
    for(auto j = 0; j < kids4.size(); j++) {
      kids4[j]->print_node(max_depth);
    }
    */
  } else {
    plprintf("Failed to set up root scenario!");
  }
}