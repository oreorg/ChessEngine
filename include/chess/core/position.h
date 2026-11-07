#pragma once

#include <chess/core/helper.h>
#include <chess/core/transposition_table.h>
#include <chess/core/types.h>
#include <chess/engine/move.h>
#include <cstdint>
#include <vector>

using Bitboard = std::uint64_t;

inline constexpr uint8_t WHITE_KING_SIDE = 1ULL << 0;
inline constexpr uint8_t WHITE_QUEEN_SIDE = 1ULL << 1;
inline constexpr uint8_t BLACK_KING_SIDE = 1ULL << 2;
inline constexpr uint8_t BLACK_QUEEN_SIDE = 1ULL << 3;

inline constexpr int DISTANCE_TO_KING_ROOK = 3;
inline constexpr int DISTANCE_TO_QUEEN_ROOK = 4;

struct Position {
  uint8_t castling_rights = WHITE_KING_SIDE | WHITE_QUEEN_SIDE | BLACK_KING_SIDE | BLACK_QUEEN_SIDE;

  int board[64] = {0};
  Bitboard piece_boards[12] = {0};
  Bitboard occupancies[3] = {0};
  std::vector<UndoInfo> history;
  std::vector<TranspositionTableEntry> transposition_table;

  PieceColor side_to_move = WHITE;

  int en_passant_square;
  void clear();
  void place_piece(PieceType piece_type, PieceColor color, int square);
  void remove_piece(int square);
  void init_start_position();
  std::vector<Move> get_legal_moves();
  bool make_move(Move move);
  bool undo_move();
};
