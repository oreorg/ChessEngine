#pragma once
#include <chess/core/game.h>
#include <optional>

struct Engine {
  PieceColor ai_color;
  std::optional<Move> find_best_move(Position &position);
};