#include <chess/core/attack_generator.h>
#include <chess/core/game.h>
#include <chess/debug/perft.h>
#include <chess/engine/engine.h>
#include <chess/gui/board_graphics.h>
#include <thread>

int main() {
  precompute_all_attacks();
  Game game;
  Engine engine;
  BoardGraphics board_graphics;

  game.start();
  engine.ai_color = game.player_color == WHITE ? BLACK : WHITE;
  board_graphics.inverse = game.player_color == WHITE ? false : true;
  Debugging::perft(game.position, 5);
  std::thread board_thread([&]() { board_graphics.run_loop(game, engine); });
  board_thread.join();
  return 0;
}