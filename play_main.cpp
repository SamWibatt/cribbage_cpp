#include <stdio.h>
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include "plat_play_main.h"

//what if we just had globals
CardUtils cu;
Cribbage cr;
CribbagePlayer cp[2];

//platform-specific main will call this after doing platspec setup
void run() {
    std::string gamename = "Cribbulationzzz";
    plprintf("Hello and welcome to %s, the crinkly cribbage playing utility.\n",gamename.c_str());
}
