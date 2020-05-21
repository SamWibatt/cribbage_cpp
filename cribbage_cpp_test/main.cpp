// let's try this with google test
// includes and such are in /usr/

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include <stdio.h>
#include "cribbage_cpp.h"
#include "gtest/gtest.h"

using namespace cribbage_cpp;

namespace {

    // Step 2. Use the TEST macro to define your tests.
    //
    // TEST has two parameters: the test case name and the test name.
    // After using the macro, you should define your test logic between a
    // pair of braces.  You can use a bunch of macros to indicate the
    // success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
    // examples of such macros.  For a complete list, see gtest.h.

    // TODO: init an array of these uint32s, srand with 9999, expect this sequence from my_random
    //
    // testing random first 10! seeded with 9999
    TEST(CribbageTest,RandomFirst10From9999) {
        my_srandom(9999);

        //dumb unrolled way
        EXPECT_EQ(168053193,my_random());
        EXPECT_EQ(529018946,my_random());
        EXPECT_EQ(639126842,my_random());
        EXPECT_EQ(91631200,my_random());
        EXPECT_EQ(299803501,my_random());
        EXPECT_EQ(800805445,my_random());
        EXPECT_EQ(857098366,my_random());
        EXPECT_EQ(2079416933,my_random());
        EXPECT_EQ(611521653,my_random());
        EXPECT_EQ(2135171076,my_random());
    }


}

int main(int argc, char *argv[]) {
    //first some dumb noodles
    char cardstr[3] = { 0,0,0 };

    for(uint8_t card = 0; card < 52; card++) {
        cardstring(card,cardstr,0);
        printf("Card is %02d - rank: %2d suit: %d val: %2d str: %s\n",card,rank(card),suit(card),val(card),cardstr);
    }

    //try a shuffle!
    uint8_t deck[52];
    uint8_t decklen = 52;
    my_srandom(9999);
    shuffle(deck);
    printf("Shuffled deck:\n");
    for(auto i = 0; i< decklen; i++) {
        cardstring(deck[i],cardstr,0);
        printf("%2d: %2d = %s\n",i,deck[i],cardstr);
    }

    //try a cut!
    for(auto j=0;j<10;j++) deck[j] = 10+j;
    decklen = 10;
    printf("Pre-cut deck: ");
    for(auto j=0;j<10;j++) printf("%d, ",deck[j]);
    printf("\n");
    cut(deck,decklen,6);
    printf("Post-cut deck: ");
    for(auto j=0;j<10;j++) printf("%d, ",deck[j]);
    printf("\n");


    // real testing main starts here
    ::testing::InitGoogleTest();
    RUN_ALL_TESTS();
}
