#pragma once

#include <chess/core/fen.h>
#include <chess/core/position.h>

struct Game {
  std::string_view fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  Position position;
  PieceColor player_color;

  void start();
};