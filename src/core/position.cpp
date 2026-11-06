#include <algorithm>
#include <chess/core/helper.h>
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

bool Position::make_move(Move move) {
  int start_square = move.square();
  int target_square = move.target_square();
  if (start_square < SQ_A1 || start_square > SQ_H8 || target_square < SQ_A1 || target_square > SQ_H8) return false;
  if (start_square == target_square) return false;
  std::vector<Move> legalMoves = generatePsuedoLegalMoves(*this);
  const int index = board[start_square];
  if (index == NO_PIECE_TYPE) return false;
  PieceColor pieceColor = index >= 6 ? BLACK : WHITE;
  if (side_to_move != pieceColor) return false;
  const bool containsMove = std::ranges::any_of(legalMoves, [=](const Move &move) {
    return move.square() == start_square && move.target_square() == target_square;
  });
  if (!containsMove) return false;

  bool isWhite = pieceColor == WHITE;

  PieceType pieceType = static_cast<PieceType>(index % 6);
  UndoInfo info{start_square, target_square, static_cast<PieceType>(board[target_square]), castling_right,
                enpassant_square};

  // Updating Pawn Special Moves
  if (pieceType == PAWN) {
    int direction = isWhite ? 8 : -8;
    if (std::abs(start_square - target_square) == 16) {
      enpassant_square = target_square - direction;
    } else {
      enpassant_square = -1;
    }
    if (move.type() == ENPASSANT) {
      info.capturedPiece = static_cast<PieceType>(board[target_square - direction]);
      remove_piece(target_square - direction);
    }
  } else {
    enpassant_square = -1;
  }

  // Checking for King Special Moves
  if (pieceType == KING) {
    if (pieceColor == WHITE) {
      castling_right &= ~WHITE_KING_SIDE;
      castling_right &= ~WHITE_QUEEN_SIDE;
    } else {
      castling_right &= ~BLACK_KING_SIDE;
      castling_right &= ~BLACK_QUEEN_SIDE;
    }

    if (move.type() == CASTLE) {
      const int king_side_castle = isWhite ? SQ_G1 : SQ_G8;
      const int queen_side_castle = isWhite ? SQ_C1 : SQ_C8;
      const int rook_king_side = isWhite ? SQ_H1 : SQ_H8;
      const int rook_queen_side = isWhite ? SQ_A1 : SQ_A8;

      if (target_square == king_side_castle) {
        info.extra = KING_SIDE;
        place_piece(ROOK, pieceColor, target_square - 1);
        remove_piece(rook_king_side);
      } else if (target_square == queen_side_castle) {
        info.extra = QUEEN_SIDE;
        place_piece(ROOK, pieceColor, target_square + 1);
        remove_piece(rook_queen_side);
      }
    }
  }

  if (pieceType == ROOK) {
    if (pieceColor == WHITE) {
      if (move.extra() == KING_SIDE) castling_right &= ~WHITE_KING_SIDE;
      if (move.extra() == QUEEN_SIDE) castling_right &= ~WHITE_QUEEN_SIDE;
    } else {
      if (move.extra() == KING_SIDE) castling_right &= ~BLACK_KING_SIDE;
      if (move.extra() == QUEEN_SIDE) castling_right &= ~BLACK_QUEEN_SIDE;
    }
  }

  if (move.type() == PROMOTION) {
    pieceType = static_cast<PieceType>(move.extra());
  }

  remove_piece(target_square);
  remove_piece(start_square);
  place_piece(pieceType, pieceColor, target_square);
  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  info.move_type = static_cast<MoveType>(move.type());
  history.push_back(info);
  return true;
}

bool Position::undo_move() {
  if (history.empty()) return false;
  UndoInfo info = history.back();
  history.pop_back();
  int startSquare = info.startSquare;
  int targetSquare = info.targetSquare;

  PieceColor pieceColor = board[info.targetSquare] >= 6 ? BLACK : WHITE;
  PieceType pieceType = static_cast<PieceType>(board[info.targetSquare] % 6);

  if (info.move_type == PROMOTION) pieceType = PAWN;
  if (info.move_type == CASTLE) {
    if (info.extra == KING_SIDE) {
      remove_piece(info.startSquare + 1);
      place_piece(ROOK, pieceColor, info.startSquare + 3);
    } else if (info.extra == QUEEN_SIDE) {
      remove_piece(info.startSquare - 1);
      place_piece(ROOK, pieceColor, info.startSquare - 4);
    }
  }
  if (info.move_type == ENPASSANT) {
    PieceColor capturedColor = info.capturedPiece >= 6 ? BLACK : WHITE;
    int direction = capturedColor ? -8 : 8;
    place_piece(PAWN, capturedColor, info.targetSquare + direction);
  }
  remove_piece(targetSquare);
  place_piece(pieceType, pieceColor, startSquare);
  if (info.capturedPiece != NO_PIECE_TYPE && info.move_type != ENPASSANT) {
    PieceColor capturedColor = info.capturedPiece >= 6 ? BLACK : WHITE;
    PieceType capturedType = static_cast<PieceType>(info.capturedPiece % 6);
    place_piece(capturedType, capturedColor, info.targetSquare);
  }

  castling_right = info.castling_rights;
  enpassant_square = info.enpassant_square;
  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  return true;
}

std::vector<Move> Position::getLegalMoves() { return generateLegalMoves(*this); }