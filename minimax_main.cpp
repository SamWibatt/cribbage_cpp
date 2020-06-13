//minimax.cpp! Let's try doing some AI stuff
// for now, let's just do it as a standalone main, and we'll move it around as needed
#include "card_utils.h"
#include "cribbage_core.h"
#include "cribbage_player.h"
#include "plat_io.h"
#include "minimax_main.h"
#include "plat_minimax_main.h"

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


*/

//CribbageCountNode implementations
node_value_t CribbageCountNode::heuristicValue() {
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //WRITE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  return 0;
}


// main method ------------------------------------------------------------------------------------------
void run() {
  plprintf("Hello and welcome to MINIMAX_MAIN, the minty minimax fiddlement utility.\n");
}