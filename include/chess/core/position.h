#pragma once

#include "types.h"
#include <chess/core/helper.h>
#include <chess/engine/move.h>
#include <cstdint>
#include <iterator>
#include <vector>

using U64 = std::uint64_t;

inline constexpr uint8_t WHITE_KING_SIDE = 1ULL << 0;
inline constexpr uint8_t WHITE_QUEEN_SIDE = 1ULL << 1;
inline constexpr uint8_t BLACK_KING_SIDE = 1ULL << 2;
inline constexpr uint8_t BLACK_QUEEN_SIDE = 1ULL << 3;

inline constexpr int DISTANCE_TO_KING_ROOK = 3;
inline constexpr int DISTANCE_TO_QUEEN_ROOK = 4;

struct Position {
  uint8_t castling_right = WHITE_KING_SIDE | WHITE_QUEEN_SIDE | BLACK_KING_SIDE | BLACK_QUEEN_SIDE;

  int board[64] = {0};
  U64 piece_boards[12] = {0};
  U64 occupancies[3] = {0};
  std::vector<UndoInfo> history;

  PieceColor side_to_move = WHITE;

  int enpassant_square;
  void clear();
  void place_piece(PieceType pt, PieceColor color, int sqaure);
  void remove_piece(int square);
  void init_start_position();
  std::vector<Move> getLegalMoves();
  bool make_move(Move move);
  bool undo_move();
};
