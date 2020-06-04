
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include <vector>

using namespace cardutils;
using namespace cribbage_core;


//
//# Default computer player --------------------------------------------------------------------------------------------
//# K so I have a player object. What do we know about players?
//# - what cards they hold and which are "used"
//# - whether they're the dealer
//# - their score
//# parent pybbage
//class Player:
//    def __init__(self, parent, cards = [], used_cards = [], crib = [], dealer = False, score = 0, name = "Player"):
//        self.parent = parent
//        self.cards = cards
//        self.used_cards = used_cards
//        self.crib = crib
//        self.dealer = dealer
//        self.score = score
//        self.name = name

// CTOR AND DTOR ====================================================================================
CribbagePlayer::CribbagePlayer() {
    //TODO: FIGURE OUT HOW TO DO DEFAULTS and what good defaults are

}

CribbagePlayer::~CribbagePlayer() {
}


//
//    def set_score(self,points):
//        self.score = points
//
//

//  HERE IS AN EXAMPLE of where I need to reorganize. the player class shouldn't be responsible for UI.
//    def advance_peg_by_score_callback(self,score):
//        # OVERRIDE to do graphics of peg advancement, given a total score (for shew)
//        print("Advance peg by score callback",score)
//        pass
//
//    # this is used by play, and every scoring combination of cards in play is a contiguous set including the
//    # newly played card. so if num_cards is 1, the top card is highlighted, on up to e.g. run of 4, highlight
//    # 4 cards, top and 3 preceding.
//    def advance_peg_by_index_callback(self,score_index,num_cards):
//        # OVERRIDE to do graphics of peg advancement, given a score index (for play)
//        print("Advance peg by index callback",self.parent.scoreStringsNPoints[score_index][0],"for",
//              self.parent.scoreStringsNPoints[score_index][1],"including this many cards:",num_cards)
//        pass
//

//  THESE LOOK LIKE THEY NEED REORGANIZING TOO
//    # add_score_by_* is a pegging - if the score goes past 120, force it to 121 (game hole) and return True.
//    # otherwise return False.
//    # add by index, so can show name (in play) or by total score (in shew)
//    # this is used by play, and every scoring combination of cards in play is a contiguous set including the
//    # newly played card. so if num_cards is 1, the top card is highlighted, on up to e.g. run of 4, highlight
//    # 4 cards, top and 3 preceding.
//    def add_score_by_index(self,score_index,num_cards):
//        self.score += self.parent.scoreStringsNPoints[score_index][1]
//        self.advance_peg_by_index_callback(score_index,num_cards)
//        if self.score > 120:
//            self.score = 121
//            return True
//        return False
//
//    def add_score_by_score(self,score):
//        self.score += score
//        self.advance_peg_by_score_callback(score)
//        if self.score > 120:
//            self.score = 121
//            return True
//        return False
//
//    def get_score(self):
//        return self.score
//
//    def set_dealer(self,isdealer):
//        self.dealer = isdealer
//
//    def is_dealer(self):
//        return self.dealer
//
//    def get_cards(self):
//        return self.cards
//
//    def set_cards(self,cards):
//        self.cards = cards
//
//    def add_card(self,card):
//        self.cards.append(card)
//
//    def add_crib(self,card):
//        self.crib.append(card)
//
//    def get_used_cards(self):
//        return self.used_cards
//
//    def set_used_cards(self,used_cards):
//        self.used_cards = used_cards
//
//    def get_crib(self):
//        return self.crib
//
//    def set_crib(self,crib):
//        self.crib = crib
//
//    def get_name(self):
//        return self.name
//
//    def set_name(self,name):
//        self.name = name
//
//    def get_parent(self):
//        return parent
//
//    def set_parent(self,parent):
//        self.parent = parent
//
//    # =================================================================================================================
//    # HERE ARE OVERRIDEABLE STRATEGY METHODS like playing a card in the play or shew
//    # easy ones are using human input and choosing cards at random
//    # might also have one for initial cut or subsequent cuts
//    def cut(self,deck):
//        # default: assume computer, and that deck is big enough to do this. Should be called right after a shuffle
//        cutspot = self.parent.get_computer_input(4,len(deck)-4)
//        print("*** cutting.... at",cutspot)
//        return self.parent.cut(deck,cutspot)
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
