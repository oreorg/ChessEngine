#include <iostream>
#include <chess/core/position.h>
#include <chess/gui/board_graphics.h>
#include <chess/engine/attack_generator.h>

int main(){
    precompute_all_attacks();
    Position current_position;
    BoardGraphics board_graphics;
    current_position.init_start_position();
    board_graphics.run_loop(current_position);
    return 0;
}