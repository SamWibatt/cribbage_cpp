#include <stdio.h>
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"

//what if we just had globals
CardUtils cu;
Cribbage cr;
CribbagePlayer cp[2];

int main(int argc, char *argv[]) {
    plprintf("Hello and welcome to Cribbulation, the crinkly cribbage playing utility.\n");
    return 0;
}