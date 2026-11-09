#include <chess/core/fen.h>
#include <chess/core/position.h>
#include <chess/debug/perft.h>
#include <chess/engine/attack_generator.h>
#include <chess/gui/board_graphics.h>
#include <iostream>
#include <string_view>

int main() {
  precompute_all_attacks();
  const int MAX_DEPTH = 5;
  const bool DIVIDE = false;
  std::string_view fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 ";
  Position current_position = parse_fen(fen);
  BoardGraphics board_graphics;
  for (int i = 1; i <= MAX_DEPTH; i++) {
    Debugging::perft(current_position, i, DIVIDE);
  }
  board_graphics.run_loop(current_position);
  return 0;
}