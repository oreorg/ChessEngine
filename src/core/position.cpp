#include <chess/core/position.h>
#include <chess/engine/move_generator.h>

void Position::clear() {
  for (int i = SQ_A1; i <= SQ_H8; i++) {
    board[i] = NO_PIECE_TYPE;
    if (i < 12) {
      piece_boards[i] = 0ULL;
    }
    if (i < 3) {
      occupancies[i] = 0ULL;
    }
  }
}

void Position::place_piece(PieceType pt, PieceColor color, int square) {
  int index = pt + (color * 6);
  set_bit(piece_boards[index], square);
  set_bit(occupancies[color], square);
  set_bit(occupancies[BOTH], square);
  board[square] = index;
}

void Position::remove_piece(int square) {
  int index = board[square];

  if (index == NO_PIECE_TYPE) return;

  PieceColor color = (index >= 6) ? BLACK : WHITE;
  pop_bit(piece_boards[index], square);
  pop_bit(occupancies[color], square);
  pop_bit(occupancies[BOTH], square);
  board[square] = NO_PIECE_TYPE;
}

void Position::init_start_position() {
  clear();
  for (int i = SQ_A2; i <= SQ_H2; i++) {
    place_piece(PAWN, WHITE, i);
  }
  place_piece(ROOK, WHITE, SQ_A1);
  place_piece(ROOK, WHITE, SQ_H1);

  place_piece(KNIGHT, WHITE, SQ_B1);
  place_piece(KNIGHT, WHITE, SQ_G1);

  place_piece(BISHOP, WHITE, SQ_C1);
  place_piece(BISHOP, WHITE, SQ_F1);

  place_piece(QUEEN, WHITE, SQ_D1);
  place_piece(KING, WHITE, SQ_E1);

  for (int i = SQ_A7; i <= SQ_H7; i++) {
    place_piece(PAWN, BLACK, i);
  }

  place_piece(ROOK, BLACK, SQ_A8);
  place_piece(ROOK, BLACK, SQ_H8);

  place_piece(KNIGHT, BLACK, SQ_B8);
  place_piece(KNIGHT, BLACK, SQ_G8);

  place_piece(BISHOP, BLACK, SQ_C8);
  place_piece(BISHOP, BLACK, SQ_F8);

  place_piece(QUEEN, BLACK, SQ_D8);
  place_piece(KING, BLACK, SQ_E8);
}

bool Position::make_move(int startSquare, int targetSquare) {
  if (startSquare < SQ_A1 || startSquare > SQ_H8 || targetSquare < SQ_A1 || targetSquare > SQ_H8) return false;
  if (startSquare == targetSquare) return false;
  std::vector<Move> legalMoves = generatePsuedoLegalMoves(*this);
  const int index = board[startSquare];
  if (index == NO_PIECE_TYPE) return false;
  PieceColor pieceColor = index >= 6 ? BLACK : WHITE;
  if (side_to_move != pieceColor) return false;
  const bool containsMove = std::ranges::any_of(legalMoves, [startSquare, targetSquare](const Move &move) {
    return move.startSquare == startSquare && move.targetSquare == targetSquare;
  });

  if (!containsMove) return false;
  PieceType pieceType = static_cast<PieceType>(index % 6);
  UndoInfo info{startSquare, targetSquare,
                board[targetSquare] == NO_PIECE_TYPE ? NO_PIECE_TYPE : static_cast<PieceType>(board[targetSquare]),
                false};
  history.push_back(info);

  remove_piece(targetSquare);
  remove_piece(startSquare);
  place_piece(pieceType, pieceColor, targetSquare);
  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  return true;
}

bool Position::undo_move() {
  if (history.empty()) return false;
  UndoInfo info = history.back();
  history.pop_back();

  PieceColor pieceColor = board[info.targetSquare] >= 6 ? BLACK : WHITE;
  PieceType pieceType = static_cast<PieceType>(board[info.targetSquare] % 6);

  remove_piece(info.targetSquare);
  place_piece(pieceType, pieceColor, info.startSquare);
  if (!(info.capturedPiece == NO_PIECE_TYPE)) {
    PieceColor capturedColor = info.capturedPiece >= 6 ? BLACK : WHITE;
    PieceType capturedType = static_cast<PieceType>(info.capturedPiece % 6);
    place_piece(capturedType, capturedColor, info.targetSquare);
  }

  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  return true;
}

std::vector<Move> Position::getLegalMoves() { return generateLegalMoves(*this); }