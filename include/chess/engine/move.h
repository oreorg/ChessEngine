#pragma once

#include <chess/core/types.h>
#include <cstdint>

enum MoveType { NORMAL, PROMOTION, CASTLE, EN_PASSANT };

enum CastleSide { KING_SIDE, QUEEN_SIDE };

struct Move {
private:
  uint16_t data;

public:
  constexpr Move() = default;

  constexpr Move(int square, int target, MoveType type = NORMAL, int extra = -1) {
    data = (square & 0x3F) | (target & 0x3F) << 6 | (static_cast<int>(type) & 0x3) << 12 | (extra & 0x3) << 14;
  }
  constexpr int square() const { return data & (0x3f); }

  constexpr int target_square() const { return (data >> 6) & 0x3f; }

  constexpr MoveType type() const { return static_cast<MoveType>((data >> 12) & 0x3); }

  constexpr int extra() const { return (data >> 14) & 0x3; }
};

struct UndoInfo {
  int start_square;
  int target_square;
  PieceType captured_piece = NO_PIECE_TYPE;
  uint8_t castling_rights = 0;
  int en_passant_square = -1;
  MoveType move_type = NORMAL;
  int extra = -1;

  int half_moves = -1;
};