#include <chess/core/position.h>
#include <chess/engine/evaluation.h>

int get_piece_value(PieceType type) {
  switch (type) {
  case PieceType::PAWN: return 1;
  case PieceType::KNIGHT: return 3;
  case PieceType::BISHOP: return 3;
  case PieceType::ROOK: return 5;
  case PieceType::QUEEN: return 9;
  default: return 0;
  }
}
int evaluate(Position &position) {
  int white_pieces = 0;
  int black_pieces = 0;
  for (int i = 0; i < 5; i++) {
    PieceType piece_type = static_cast<PieceType>(i % 6);
    if (piece_type == KING) continue;
    white_pieces += get_piece_value(piece_type) * count_bits(position.piece_boards[i]);
    black_pieces += get_piece_value(piece_type) * count_bits(position.piece_boards[i + 6]);
  }
  return white_pieces - black_pieces;
}
