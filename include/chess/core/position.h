#pragma once

#include "types.h"
#include <algorithm>
#include <chess/engine/move.h>
#include <cstdint>
#include <vector>

using U64 = std::uint64_t;

struct Position {
  int board[64] = {0};
  U64 piece_boards[12] = {0};
  U64 occupancies[3] = {0};
  std::vector<UndoInfo> history;

  PieceColor side_to_move = WHITE;
  void clear();
  void place_piece(PieceType pt, PieceColor color, int sqaure);
  void remove_piece(int square);
  void init_start_position();
  std::vector<Move> getLegalMoves();
  bool make_move(int startSquare, int targetSquare);
  bool undo_move();
};

inline int get_rank(int square) { return square / 8; }
inline int get_file(int square) { return square % 8; }

inline int get_square(int col, int rank) { return rank * 8 + col; }

inline void set_bit(U64 &bb, int square) { bb |= 1ULL << square; }

inline void pop_bit(U64 &bb, int square) { bb &= ~(1ULL << square); }

inline bool check_bit(U64 bb, int square) {
  return (bb & (1ULL << square)) != 0;
}

inline int count_bits(U64 bb) {
  int counter = 0;
  while (bb) {
    counter += 1;
    bb &= bb - 1;
  }
  return counter;
}

inline int popLSB(U64 &bb) {
  int index = std::countr_zero(bb);
  bb &= bb - 1;
  return index;
}

inline uint64_t vertical_flip(U64 bb) {
  return ((bb >> 56) & 0x00000000000000FFULL) |
         ((bb >> 40) & 0x000000000000FF00ULL) |
         ((bb >> 24) & 0x0000000000FF0000ULL) |
         ((bb >> 8) & 0x00000000FF000000ULL) |
         ((bb << 8) & 0x000000FF00000000ULL) |
         ((bb << 24) & 0x0000FF0000000000ULL) |
         ((bb << 40) & 0x00FF000000000000ULL) |
         ((bb << 56) & 0xFF00000000000000ULL);
}
