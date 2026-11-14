#pragma once

#include <chess/core/helper.h>
#include <chess/core/move.h>
#include <chess/core/types.h>
#include <cstdint>
#include <vector>

enum PositionStatus { ONGOING, STALEMATE, CHECKMATE };

inline constexpr uint8_t WHITE_KING_SIDE = 1ULL << 0;
inline constexpr uint8_t WHITE_QUEEN_SIDE = 1ULL << 1;
inline constexpr uint8_t BLACK_KING_SIDE = 1ULL << 2;
inline constexpr uint8_t BLACK_QUEEN_SIDE = 1ULL << 3;

inline constexpr int DISTANCE_TO_KING_ROOK = 3;
inline constexpr int DISTANCE_TO_QUEEN_ROOK = 4;

class Position {
public:
  uint8_t castling_rights = WHITE_KING_SIDE | WHITE_QUEEN_SIDE | BLACK_KING_SIDE | BLACK_QUEEN_SIDE;
  int half_moves = 0;
  int en_passant_square = -1;
  PositionStatus status = ONGOING;

  int board[64] = {0};
  U64 piece_boards[12] = {0};
  U64 occupancies[3] = {0};
  U64 hash_key = 0;
  std::vector<U64> hash_history;
  std::vector<UndoInfo> history;
  PieceColor side_to_move = WHITE;
  bool make_move(Move move);
  bool undo_move();
  PositionStatus check_position();

private:
  friend MoveList generate_legal_moves(Position &position);
  friend Position parse_fen(std::string_view fen);

  void clear();
  void place_piece(PieceType piece_type, PieceColor color, int square);
  void remove_piece(int square);
  bool make_move_internal(Move move);
  bool undo_move_internal();
  void update_hash(bool is_white);
  bool validate_move(Move &move, PieceColor piece_color, int index);
  void make_special_move(Move &move, int index, UndoInfo &info);
};
