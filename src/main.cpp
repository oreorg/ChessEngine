#include <iostream>
#include <chess/core/position.h>
#include <chess/gui/board_graphics.h>
#include <chess/engine/attack_generator.h>

int main(){
    precompute_all_attacks();
    Position currentPosition;
    BoardGraphics boardGraphics;
    currentPosition.init_start_position();
    boardGraphics.run_loop(currentPosition);
    return 0;
}