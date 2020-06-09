#ifndef CRIBBAGE_PLAYER_H_INCLUDED
#define CRIBBAGE_PLAYER_H_INCLUDED

#include "card_utils.h"
#include "cribbage_core.h"

#include <utility>
#include <vector>

using namespace cardutils;
using namespace cribbage_core;

class CribbagePlayer {
  // data members
  // =======================================================================================================
 protected:
  // def __init__(self, parent, cards = [], used_cards = [], crib = [], dealer =
  // False, score = 0, name = "Player"):
  //    self.parent = parent
  //    self.cards = cards
  //    self.used_cards = used_cards
  //    self.crib = crib
  //    self.dealer = dealer
  //    self.score = score
  //    self.name = name
  // so - for C++ version need to think over what a player should really have.
  // Most of this is good, I think. try to refactor away whatever "parent" was
  // needed for "name" might also not be necessary in arduino version - but keep
  // it, it's a tiny memory use

  // initialized values here can be overridden by
  bool dealer = false;
  std::vector<card_t> cards;
  std::vector<card_t> used_cards;
  std::vector<card_t> crib;
  index_t score = 0;
  std::string name = "";

 public:
  inline index_t get_score() { return score; }
  inline void set_score(index_t nuscore) { score = nuscore; }

  inline bool is_dealer() { return dealer; }
  inline void set_dealer(bool nudeal) { dealer = nudeal; }

  inline std::vector<card_t> &get_cards() { return cards; }
  inline void set_cards(std::vector<card_t> nucards) { cards = nucards; }
  inline void add_card(card_t nucard) { if(cards.size() < 6) cards.push_back(nucard); }

  inline std::vector<card_t> &get_crib() { return crib; }
  inline void set_crib(std::vector<card_t> nucrib) { crib = nucrib; }
  inline void add_crib(card_t nucard) { if(crib.size() < 4)  crib.push_back(nucard); }

  inline std::vector<card_t> &get_used_cards() { return used_cards; }
  inline void set_used_cards(std::vector<card_t> nuused) {
    used_cards = nuused;
  }
  inline void add_used_cards(card_t nucard) {  if(used_cards.size() < 4) used_cards.push_back(nucard); }

  inline std::string get_name() { return name; }
  inline void set_name(std::string nuname) { name = nuname; }

 public:
  CribbagePlayer();
  CribbagePlayer(std::string nuname, bool nudealer);
  virtual ~CribbagePlayer();

  // overrideables for player subclasses
  virtual index_t get_cut_index(index_t deck_len);
  std::pair<card_t, card_t> get_discards(std::vector<card_t> &cardvec);
  card_t get_play_card(std::vector<card_t> &cardvec,
                       std::vector<card_t> &cardstack);
};

#endif  // CRIBBAGE_PLAYER_H_INCLUDED
