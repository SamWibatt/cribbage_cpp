
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include <vector>

using namespace cardutils;
using namespace cribbage_core;

//gross, might refactor all to hell and gone
extern CardUtils cu;
extern Cribbage cr;

//# Default player ------------------------------------------------------------------------------------
// Default player is computer player that does random cuts and first legal card for discard or play.

// CTOR AND DTOR ====================================================================================
CribbagePlayer::CribbagePlayer()
{ 
    cards.reserve(6);           //allocate space in cards vector for 6 cards, which is the max
    used_cards.reserve(4);      //similar for used_cards, cards that have been put down in the play, at most 4
    crib.reserve(4);            //and crib
    score = 0;
    dealer = false;
    name = "";
}

//delegate to default ctor
CribbagePlayer::CribbagePlayer(std::string nuname, bool nudealer)  : CribbagePlayer()
{ 
    dealer = nudealer;
    name = nuname;
}

CribbagePlayer::~CribbagePlayer() {
}

// =================================================================================================================
// HERE ARE OVERRIDEABLE STRATEGY METHODS like playing a card in the play or shew

// 
index_t CribbagePlayer::get_cut_index(index_t deck_len) {
    if(deck_len < 9) return 0;                          //no legal cut possible
    return 4 + cu.random_at_most(uint32_t(deck_len)-4); //hardcode 4s bc you can't cut anywhere 
                                                        //within 4 cards of an end of the deck
}

//
//    def discard(self,otherplayer):
//        # here, choose two cards from self.hand and into the dealer's crib, either own if self.is_dealer
//        # or otherplayer if not.
//        # default implementation, just choose two at random. Can use deal to pull a card off.
//        # or, just yank the card out.
//        for j in range(0,2):
//            cardind = self.parent.random_at_most(len(self.cards)-1)
//            card = self.cards[cardind]              # split out the card
//            self.cards = self.cards[:cardind] + self.cards[cardind + 1:]  # remove card from hand
//            if self.is_dealer():
//                self.add_crib(card)
//            else:
//                otherplayer.add_crib(card)

// factoring parent out here is again moving random_at_most out of parent

//
//    # in the play, we get the stack of cards to date
//    # choose a card and play it, if one is legal, returning the new stack of cards and the score for this round
//    # (which also gets added on here.) can be 0. If -1, no legal play was available, and card stack unchanged,
//    # which means "go"
//    # card that gets played, if any, is appended to used_cards so hand can be restored for show.
//    # THIS VERSION SHOULD STAY AS THE DEFAULT, OR AT LEAST BE AVAILABLE THROUGH A PlayFirstLegalPlayer
//    # SUBCLASS, BC THAT WILL MAKE FOR EASY UNIT TESTING!!!!!!!
//    # refactoring to return score index e.g. instead of 5 points for a run of 5, return self.SCORE_RUN5
//    # as curscore. Which means play_card has to do that.
//    # + actually it's a list or None - None if it was illegal like going over 31, [] if legal but no score,
//    # list of score indices if legal and gets scores.
//    # and now it's a list of (score index, # cards) so we can highlight! See above in player.add_score_by_index
//    # this is used by play, and every scoring combination of cards in play is a contiguous set including the
//    # newly played card. so if num_cards is 1, the top card is highlighted, on up to e.g. run of 4, highlight
//    # 4 cards, top and 3 preceding.
//    def play(self,curcards):
//        # choose a card for the play, if there is one that works
//        # let's just go with the first one
//        # def play_card(curcards, newcard):
//        # return (newcards, curtotal, scorelist)
//        for i in range(0,len(self.cards)):
//            card = self.cards[i]
//            (newcards,curtotal,scorelist) = self.parent.play_card(curcards,card)
//            if scorelist is not None:
//                # play this one!
//                print("playing",self.parent.cardstring(card),"on",[self.parent.cardstring(x) for x in curcards])
//                self.cards = self.cards[:i] + self.cards[i + 1:]  # remove card from hand
//                self.used_cards.append(card)        # memorize it so can be restored
//                return (newcards,curtotal,scorelist)
//                pass
//        # if we get here, it's a go, I guess. -1 works for score index as well as actual score.
//        print("Go!")
//        return (curcards,sum([self.parent.val(x) for x in curcards]),None)
//
//    # =================================================================================================================
//
//    def print_hand(self):
//        print("Hand:",[self.parent.cardstring(x) for x in self.cards],"used",
//              [self.parent.cardstring(x) for x in self.used_cards])
//
//    def print_crib(self):
//        if self.is_dealer():
//            print("Crib:", [self.parent.cardstring(x) for x in self.crib])
//        else:
//            print("Crib: (not dealer)")
//
//    def print_all(self):
//        print("Name:",self.name,"Dealer:",self.is_dealer(),"score:",self.score)
//        self.print_hand()
//        if self.is_dealer():
//            self.print_crib()
//
//
//# Default human player ------------------------------------------------------------------------------------------------
//class HumanPlayer(Player):
//
//    def __init__(self, parent, cards = [], used_cards = [], crib = [], dealer = False, score = 0, name = "Player"):
//        super().__init__(parent, cards, used_cards, crib, dealer, score, name)
//
//    # TEMP just do random cut like computer
//    # TODO restore this later but cuts are annoying
//    # def cut(self,deck):
//    #     # humqn version!
//    #     print("*** enter cut!")
//    #     return cut(deck,get_input(4,len(deck)-4))
//
//
//    def discard(self, otherplayer):
//        # here, choose two cards from self.hand and into the dealer's crib, either own if self.is_dealer
//        # or otherplayer if not.
//        # default implementation, let user input
//        # or, just yank the card out.
//        for j in range(0, 2):
//            print("Cards:")
//            for k in range(0,len(self.cards)):
//                print("{}) {}".format(k,self.parent.cardstring(self.cards[k])))
//            print("Enter a card to discard")
//            cardind = self.parent.get_input(0,len(self.cards) - 1)
//            card = self.cards[cardind]  # split out the card
//            self.cards = self.cards[:cardind] + self.cards[cardind + 1:]  # remove card from hand
//            if self.is_dealer():
//                self.add_crib(card)
//            else:
//                otherplayer.add_crib(card)
//
//    # in the play, we get the stack of cards to date
//    # choose a card and play it, if one is legal, returning the new stack of cards and the score for this round
//    # (which also gets added on here.) can be 0. If -1, no legal play was available, and card stack unchanged,
//    # which means "go"
//    # card that gets played, if any, is appended to used_cards so hand can be restored for show.
//    # THIS VERSION SHOULD STAY AS THE DEFAULT, OR AT LEAST BE AVAILABLE THROUGH A PlayFirstLegalPlayer
//    # SUBCLASS, BC THAT WILL MAKE FOR EASY UNIT TESTING!!!!!!!
//    def play(self,curcards):
//        # choose a card for the play, if there is one that works
//        # so scan through and make sure there is a legal play. If not,
//        legalcards = []
//        for i in range(0,len(self.cards)):
//            card = self.cards[i]
//            (newcards,curtotal,scorelist) = self.parent.play_card(curcards,card)
//            if scorelist is not None:
//                legalcards.append(card)
//        if legalcards == []:
//            print("No legal card to play! you say go")
//            return (curcards,sum([self.parent.val(x) for x in curcards]),None)
//
//        # limit choices to legal cards
//        print("Legal Cards:")
//        for k in range(0, len(legalcards)):
//            print("{}) {}".format(k, self.parent.cardstring(legalcards[k])))
//        print("Enter a card to play:")
//        cardind = self.parent.get_input(0,len(legalcards) - 1)
//        card = legalcards[cardind]  # split out the card
//        self.cards = self.cards[:cardind] + self.cards[cardind + 1:]  # remove card from hand
//        self.used_cards.append(card)  # memorize it so can be restored
//        (newcards, curtotal, scorelist) = self.parent.play_card(curcards, card)
//        return (newcards,curtotal,scorelist)
//
//# Player for Unit Testing that plays the first card in their hand that is legal.
//class PlayFirstLegalCardPlayer(Player):
//    def __init__(self, parent, cards = [], used_cards = [], crib = [], dealer = False, score = 0, name = "Player"):
//        super().__init__(parent, cards, used_cards, crib, dealer, score, name)
//
//    def play(self,curcards):
//        # choose a card for the play, if there is one that works
//        # let's just go with the first one
//        # def play_card(curcards, newcard):
//        # return (newcards, curtotal, scorelist)
//        for i in range(0,len(self.cards)):
//            card = self.cards[i]
//            (newcards,curtotal,scorelist) = self.parent.play_card(curcards,card)
//            if scorelist is not None:
//                # play this one!
//                print("playing",self.parent.cardstring(card),"on",[self.parent.cardstring(x) for x in curcards])
//                self.cards = self.cards[:i] + self.cards[i + 1:]  # remove card from hand
//                self.used_cards.append(card)        # memorize it so can be restored
//                # TODO: wait, should this be here? self.score += curscore;
//                return (newcards,curtotal,scorelist)
//                pass
//        # if we get here, it's a go, I guess
//        print("Go!")
//        return (curcards,sum([self.parent.val(x) for x in curcards]),None)
//
