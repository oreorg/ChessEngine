#include <chess/core/game.h>
#include <random>

void Game::start() {
  std::mt19937 rng{std::random_device{}()};
  std::uniform_int_distribution<int> choose_color(0, 100);
  position = parse_fen(fen);
  player_color = choose_color(rng) >= 50 ? WHITE : BLACK;
}