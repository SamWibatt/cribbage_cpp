// let's try this with google test
// includes and such are in /usr/

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <stdio.h>
#include <iostream>
#include <string>
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "gtest/gtest.h"
#include "plat_io.h"

using namespace cardutils;
using namespace cribbage_core;

// need some globals in order to use the classes - gross but there it is
CardUtils cu;
Cribbage cr;

int main(int argc, char *argv[]) {
  // silly noodles
  /*
  std::vector <card_t> stacky;
  stacky.push_back(cu.stringcard("9c"));
  stacky.push_back(cu.stringcard("2h"));
  stacky.push_back(cu.stringcard("7d"));
  stacky.push_back(cu.stringcard("4c"));
  cr.play_card(stacky, cu.stringcard("6c"), nullptr, false);
  */

  /*
  std::vector<card_t> hand;
  hand.push_back(31);
  hand.push_back(17);
  hand.push_back(41);
  hand.push_back(51);
  card_t starter = 44;
  plprintf("Hand: ");
  std::for_each(hand.begin(),hand.end(),[](card_t c){plprintf("%d ",c);});
  plprintf("Starter: %d\n",starter);
  std::array<card_t,5> whole_hand;
  std::array<card_t,5> whole_vals;
  std::array<card_t,5> sorthand_nranks;
  std::array<card_t,5> whole_suits;
  std::array<index_t,5> sort_map;
  //try a bunch of reps to see how long it takes - v. roughly 3 sec for ten
  million iterations, not great
  //BUT in the context of a giant AI search or whatever, not terrible; minimax
  is unlikely to be doing that many
  //at least if I limit the deepening
  //oh wait that was on debug, how about release? pretty much insty
  //plprintf("TEN MILLION!!!!\n");
  //for(auto j = 0; j < 10000000; j++)
      c.prep_score_hand(hand, starter, whole_hand, whole_vals, sorthand_nranks,
  whole_suits, sort_map, true ); plprintf("Whole Hand: ");
  std::for_each(whole_hand.begin(),whole_hand.end(),[](card_t c){plprintf("%d
  ",c);}); plprintf("\nWhole Vals: ");
  std::for_each(whole_vals.begin(),whole_vals.end(),[](card_t c){plprintf("%d
  ",c);}); plprintf("\nWhole Hand Suits: ");
  std::for_each(whole_suits.begin(),whole_suits.end(),[](card_t c){plprintf("%d
  ",c);}); plprintf("\nSortHand NormRanks: ");
  std::for_each(sorthand_nranks.begin(),sorthand_nranks.end(),[](card_t
  c){plprintf("%d ",c);});
  //HEY PRINT SORT MAPPING HERE IF WE DID IT
  plprintf("\nSort Map: ");
  std::for_each(sort_map.begin(),sort_map.end(),[](index_t c){plprintf("%d
  ",c);}); plprintf("\n");

  //4-carders
  std::array<card_t,4> first4;
  std::array<card_t,4> last4;
  card_t firstlastrank = sorthand_nranks[1];     //first rank of last 4 cards
  std::copy_n(sorthand_nranks.begin(),4,first4.begin());
  std::transform(sorthand_nranks.begin() +
  1,sorthand_nranks.end(),last4.begin(), [firstlastrank](card_t rank) -> card_t
  { return rank - firstlastrank; }); plprintf("First 4 nranks: ");
  std::for_each(first4.begin(),first4.end(),[](card_t c){plprintf("%d ",c);});
  plprintf("\nLast 4 nranks, firstlastrank %d: ",firstlastrank);
  std::for_each(last4.begin(),last4.end(),[](card_t c){plprintf("%d ",c);});
  plprintf("\n");
  */

  // real testing main starts here
  ::testing::InitGoogleTest();
  auto testresult = RUN_ALL_TESTS();
}
