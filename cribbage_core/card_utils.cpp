
#include "card_utils.h"
#include <stdio.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <vector>

namespace cardutils {

// Random number stuff
// ----------------------------------------------------------------------------------------------

// for the vpok random
// VPOK VERSION:
// ; implements the classic 32-bit linear congruential random number generator
// (LCRNG) where ; Xn+1 =  (a Xn + c) mod m,     n > 0 ; in this case a =
// 1664525, m = 2^32, c = 1013904223, period = 2^32.
uint32_t nextvpok = 1;
void CardUtils::v_srandom(uint32_t n) { nextvpok = n; }
uint32_t CardUtils::v_random() {
  nextvpok = (1664525ul * nextvpok) + 1013904223ul;
  return nextvpok;
}

// generating a number from 0..n, inclusive:
// this might be it. from @Ryan Reich:
// https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
// which has this commentary and C code:
// All the answers so far are mathematically wrong. Returning rand() % N does
// not uniformly give a number in the range [0, N) unless N divides the length
// of the interval into which rand() returns (i.e. is a power of 2).
// Furthermore, one has no idea whether the moduli of rand() are independent:
// it's possible that they go 0, 1, 2, ..., which is uniform but not very
// random. The only assumption it seems reasonable to make is that rand() puts
// out a Poisson distribution: any two nonoverlapping subintervals of the same
// size are equally likely and independent. For a finite set of values, this
// implies a uniform distribution and also ensures that the values of rand() are
// nicely scattered.
//
// This means that the only correct way of changing the range of rand() is to
// divide it into boxes; for example, if RAND_MAX == 11 and you want a range
// of 1..6, you should assign {0,1} to 1, {2,3} to 2, and so on. These are
// disjoint, equally-sized intervals and thus are uniformly and independently
// distributed.
//
// The correct way is to use integer arithmetic.
// ---
// so, my randmax+1 DOES overflow the container, so we need a tweak.
// easy way is to make it so it doesn't, can just make it so the biggest value
// you can ask for is 0x7FFF FFFF, and shift the random numbers down by 1. Then
// it's fine.

// Returns uniformly distributed uint32_t in the closed interval [0, max]
// Assumes 0 <= max <= RAND_MAX
uint32_t CardUtils::random_at_most(uint32_t max) {
  // coupla special cases: 0 always gets 0
  if (max == 0) return 0;
  // anything bigger than random_max is an error, return all Fs
  if (max > RANDOM_MAX) return RANDOM_ERROR;

  // max <= RAND_MAX < ULONG_MAX, so max+1 is okay.
  uint32_t num_bins = max + 1, num_rand = RANDOM_MAX + 1,
           bin_size = num_rand / num_bins, defect = num_rand % num_bins;
  uint32_t x;
  // This is carefully written not to overflow
  do {
    x = v_random() >> 1;  // hardcoded to assume RANDOM_MAX of 0x7FFF FFFF
  } while (num_rand - defect <= x);
  // Truncated division is intentional
  return x / bin_size;
}

// card related stuff
// ------------------------------------------------------------------------------------------------

// these two functions are mostly for debugging and unit testing
// card is a number from 0-51; card %4 is rank, where 0 = hearts, 1 = diamonds,
// 2 = clubs, 3 = spades. card / 4 is rank, 0 = ace .. 12 = king
const std::string rankstr = "A234567890JQK";
const std::string suitstr =
    "hdcs";  // lower case to stand out from values better

// given a card, return the string e.g. 0 => Ah = ace of hearts
std::string CardUtils::cardstring(card_t card) {
  if (card > 51) return std::string("ERROR");
  std::string s(
      2, rankstr[rank(card)]);  // length 2 string, both chars are rank (dumb,
                                // but allocates and takes care of 1st char)
  s[1] = suitstr[suit(card)];
  return s;
}

// given a 2-character string, return corresponding card (or error if it doesn't
// conform to rank/suit possibilities
card_t CardUtils::stringcard(std::string srcstr) {
  card_t card = 0;
  index_t i;
  for (i = 0; i < rankstr.size(); i++) {
    if (srcstr[0] == rankstr[i]) {
      card += card_t(i << 2);
      break;
    }
  }

  // if at this point is strlen(rankstr), the rank wasn't found
  if (i == rankstr.size()) {
    return ERROR_CARD_VAL;
  }

  // then account for the suit
  for (i = 0; i < suitstr.size(); i++) {
    if (srcstr[1] == suitstr[i]) {
      card += i;
      break;
    }
  }
  // if at this point is strlen(suitstr), the rank wasn't found
  if (i == suitstr.size()) {
    return ERROR_CARD_VAL;
  }
  return card;
}

// deck-level functions!
// =============================================================================================

// support class for shuffling, see below
class CardOrder {
 public:
  uint32_t order;  // random number by which deck is sorted, see shuffle below
  card_t card;     // card value 0..51
};

// shuffle turns the handed-in deck vector into a 52-card deck, randomized by
// means of constructing an array of CardOrder objects s.t. their "order" member
// is random and the "card" member is consecutive, then sorting by "order." The
// resulting deck is the card members copied onto the end of the deck. deck is
// not assumed to have any particular capacity but for best results, reserve 52.
void CardUtils::shuffle(std::vector<card_t> &deck) {
  std::array<CardOrder, 52> shufdeck;
  deck.clear();
  index_t j = 0;
  std::for_each(shufdeck.begin(), shufdeck.end(), [&j, this](CardOrder &c) {
    c.order = this->v_random();
    c.card = card_t(j++);
  });
  std::sort(shufdeck.begin(), shufdeck.end(),
            [](CardOrder &a, CardOrder &b) { return a.order < b.order; });
  std::for_each(shufdeck.begin(), shufdeck.end(),
                [&j, &deck](CardOrder &c) { deck.push_back(c.card); });
}

// deck is a vector, deal off the end of the array for fastness
card_t CardUtils::deal_card(std::vector<card_t> &deck) {
  if (deck.empty() || deck.size() > 52) {
    return ERROR_CARD_VAL;
  }
  auto card = deck.back();  // memorize the "rightmost" card
  deck.pop_back();          // and get rid of it, shrinking the deck
  return card;
}

// this is a "global" to support the cut function
std::array<card_t, 52> tempdeck;

// kind of weird bc I think of "index" as being from the "left" (beginning of
// array) and the deck deals from the "right" (end of array)
void CardUtils::cut(std::vector<card_t> &deck, card_t index) {
  if (deck.size() < 2 || index < 1 || index > deck.size() - 1) {
    printf("Hey illegal deck or index\n");
    return;  // no effect if illegal index or degenerate deck
  }
  // so ok - is this compatible with the idea that we deal from the "end" of the
  // deck for efficiency? I think it is, so all we have to do is go back to
  // using deck.size() - index instead of index
  auto numpop = deck.size() - index;
  std::copy_n(deck.begin(), numpop, tempdeck.begin());
  deck.erase(deck.begin(), deck.begin() + numpop);
  deck.insert(deck.end(), tempdeck.begin(), tempdeck.begin() + numpop);
}
}  // namespace cardutils
