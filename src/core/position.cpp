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

void Position::place_piece(PieceType piece_type, PieceColor color, int square) {
  int index = piece_type + (color * 6);
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
  std::vector<Move> legal_moves = generate_pseudo_legal_moves(*this);
  const int index = board[start_square];
  if (index == NO_PIECE_TYPE) return false;
  PieceColor piece_color = index >= 6 ? BLACK : WHITE;
  if (side_to_move != piece_color) return false;
  const bool contains_move = std::ranges::any_of(legal_moves, [=](const Move &move) {
    return move.square() == start_square && move.target_square() == target_square;
  });
  if (!contains_move) return false;

  bool is_white = piece_color == WHITE;

  PieceType piece_type = static_cast<PieceType>(index % 6);
  UndoInfo info{start_square, target_square, static_cast<PieceType>(board[target_square]), castling_rights,
                en_passant_square};

  // Updating Pawn Special Moves
  if (piece_type == PAWN) {
    int direction = is_white ? 8 : -8;
    if (std::abs(start_square - target_square) == 16) {
      en_passant_square = target_square - direction;
    } else {
      en_passant_square = -1;
    }
    if (move.type() == EN_PASSANT) {
      info.captured_piece = static_cast<PieceType>(board[target_square - direction]);
      remove_piece(target_square - direction);
    }
  } else {
    en_passant_square = -1;
  }

  // Checking for King Special Moves
  if (piece_type == KING) {
    if (piece_color == WHITE) {
      castling_rights &= ~WHITE_KING_SIDE;
      castling_rights &= ~WHITE_QUEEN_SIDE;
    } else {
      castling_rights &= ~BLACK_KING_SIDE;
      castling_rights &= ~BLACK_QUEEN_SIDE;
    }

    if (move.type() == CASTLE) {
      const int king_side_castle = is_white ? SQ_G1 : SQ_G8;
      const int queen_side_castle = is_white ? SQ_C1 : SQ_C8;
      const int rook_king_side = is_white ? SQ_H1 : SQ_H8;
      const int rook_queen_side = is_white ? SQ_A1 : SQ_A8;

      if (target_square == king_side_castle) {
        info.extra = KING_SIDE;
        place_piece(ROOK, piece_color, target_square - 1);
        remove_piece(rook_king_side);
      } else if (target_square == queen_side_castle) {
        info.extra = QUEEN_SIDE;
        place_piece(ROOK, piece_color, target_square + 1);
        remove_piece(rook_queen_side);
      }
    }
  }

  if (piece_type == ROOK) {
    if (piece_color == WHITE) {
      if (move.extra() == KING_SIDE) castling_rights &= ~WHITE_KING_SIDE;
      if (move.extra() == QUEEN_SIDE) castling_rights &= ~WHITE_QUEEN_SIDE;
    } else {
      if (move.extra() == KING_SIDE) castling_rights &= ~BLACK_KING_SIDE;
      if (move.extra() == QUEEN_SIDE) castling_rights &= ~BLACK_QUEEN_SIDE;
    }
  }

  if (move.type() == PROMOTION) {
    piece_type = static_cast<PieceType>(move.extra());
  }

  remove_piece(target_square);
  remove_piece(start_square);
  place_piece(piece_type, piece_color, target_square);
  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  info.move_type = static_cast<MoveType>(move.type());
  history.push_back(info);
  return true;
}

bool Position::undo_move() {
  if (history.empty()) return false;
  UndoInfo info = history.back();
  history.pop_back();
  int start_square = info.start_square;
  int target_square = info.target_square;

  PieceColor piece_color = board[info.target_square] >= 6 ? BLACK : WHITE;
  PieceType piece_type = static_cast<PieceType>(board[info.target_square] % 6);

  if (info.move_type == PROMOTION) piece_type = PAWN;
  if (info.move_type == CASTLE) {
    if (info.extra == KING_SIDE) {
      remove_piece(info.start_square + 1);
      place_piece(ROOK, piece_color, info.start_square + 3);
    } else if (info.extra == QUEEN_SIDE) {
      remove_piece(info.start_square - 1);
      place_piece(ROOK, piece_color, info.start_square - 4);
    }
  }
  if (info.move_type == EN_PASSANT) {
    PieceColor captured_color = info.captured_piece >= 6 ? BLACK : WHITE;
    int direction = captured_color ? -8 : 8;
    place_piece(PAWN, captured_color, info.target_square + direction);
  }
  remove_piece(target_square);
  place_piece(piece_type, piece_color, start_square);
  if (info.captured_piece != NO_PIECE_TYPE && info.move_type != EN_PASSANT) {
    PieceColor captured_color = info.captured_piece >= 6 ? BLACK : WHITE;
    PieceType captured_type = static_cast<PieceType>(info.captured_piece % 6);
    place_piece(captured_type, captured_color, info.target_square);
  }

  castling_rights = info.castling_rights;
  en_passant_square = info.en_passant_square;
  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  return true;
}

std::vector<Move> Position::get_legal_moves() { return generate_legal_moves(*this); }