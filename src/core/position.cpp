#include <algorithm>
#include <chess/core/helper.h>
#include <chess/core/position.h>
#include <chess/core/zobrist_hash.h>
#include <chess/engine/attack_generator.h>
#include <chess/engine/move_generator.h>

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

  update_hash(true);
  hash_history.push_back(hash_key);
  generate_legal_moves();
}

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
  hash_key ^= ZOBRIST_KEYS.piece_keys[index][square];

  set_bit(piece_boards[index], square);
  set_bit(occupancies[color], square);
  set_bit(occupancies[BOTH], square);
  board[square] = index;
}

void Position::remove_piece(int square) {
  int index = board[square];

  if (index == NO_PIECE_TYPE) return;

  PieceColor color = (index >= 6) ? BLACK : WHITE;
  hash_key ^= ZOBRIST_KEYS.piece_keys[index][square];

  pop_bit(piece_boards[index], square);
  pop_bit(occupancies[color], square);
  pop_bit(occupancies[BOTH], square);
  board[square] = NO_PIECE_TYPE;
}

void Position::update_hash(bool is_white) {
  // editing the zobrist hash key
  if (!is_white) {
    hash_key ^= ZOBRIST_KEYS.side_key;
  }
  if (en_passant_square != -1) {
    hash_key ^= ZOBRIST_KEYS.ep_keys[en_passant_square % 8];
  }
}

void Position::make_special_move(Move &move, int index, UndoInfo &info) {
  PieceColor piece_color = index >= 6 ? BLACK : WHITE;
  PieceType piece_type = static_cast<PieceType>(index % 6);

  bool is_white = piece_color == WHITE;
  int start_square = move.square();
  int target_square = move.target_square();

  if (piece_type == PAWN) {
    half_moves = 0;
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
    if (is_white) {
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
    if (is_white) {
      if (move.extra() == KING_SIDE) castling_rights &= ~WHITE_KING_SIDE;
      if (move.extra() == QUEEN_SIDE) castling_rights &= ~WHITE_QUEEN_SIDE;
    } else {
      if (move.extra() == KING_SIDE) castling_rights &= ~BLACK_KING_SIDE;
      if (move.extra() == QUEEN_SIDE) castling_rights &= ~BLACK_QUEEN_SIDE;
    }
  }
}

bool Position::validate_move(Move &move, PieceColor piece_color, int index) {
  if (move.square() < SQ_A1 || move.square() > SQ_H8 || move.target_square() < SQ_A1 || move.target_square() > SQ_H8)
    return false;
  if (move.square() == move.target_square()) return false;
  std::vector<Move> legal_moves = generate_pseudo_legal_moves(*this);
  if (index == NO_PIECE_TYPE) return false;
  if (side_to_move != piece_color) return false;
  const bool contains_move = std::ranges::any_of(legal_moves, [=](const Move &move) {
    return move.square() == move.square() && move.target_square() == move.target_square();
  });
  if (!contains_move) return false;
  return true;
}

bool Position::make_move(Move move) {
  if (!make_move_internal(move)) return false;
  generate_legal_moves();
  status = check_position();

  if (status == CHECKMATE) {
    std::string text = side_to_move == WHITE ? "Black " : "White ";
    std::cout << text << " has won" << std::endl;
  } else if (status == STALEMATE) {
    std::cout << "Stalemate" << std::endl;
  }
  return true;
}

bool Position::undo_move() {
  if (!undo_move_internal()) return false;
  generate_legal_moves();
  status = check_position();
  return true;
}

PositionStatus Position::check_position() {
  if (legal_moves.empty()) {
    if (is_king_attacked(*this, side_to_move)) {
      return CHECKMATE;
    } else {
      return STALEMATE;
    }
  }

  const auto repetitions = std::ranges::count(hash_history, hash_key);
  if (repetitions >= 3) {
    return STALEMATE;
  }

  if (half_moves == 100) {
    return STALEMATE;
  }
  return ONGOING;
}

bool Position::make_move_internal(Move move) {
  const int start_square = move.square();
  const int target_square = move.target_square();
  const int index = board[move.square()];
  PieceColor piece_color = static_cast<PieceColor>(index >= 6 ? BLACK : WHITE);
  if (!validate_move(move, piece_color, index)) return false;
  bool is_white = piece_color == WHITE;

  half_moves += 1;
  if (board[target_square] != NO_PIECE_TYPE) {
    half_moves = 0;
  }

  PieceType piece_type = static_cast<PieceType>(index % 6);
  UndoInfo info{start_square, target_square, static_cast<PieceType>(board[target_square]), castling_rights,
                en_passant_square};

  if (move.type() == PROMOTION) {
    piece_type = static_cast<PieceType>(move.extra());
  }
  make_special_move(move, index, info);

  remove_piece(target_square);
  remove_piece(start_square);
  place_piece(piece_type, piece_color, target_square);

  update_hash(is_white);
  hash_history.push_back(hash_key);

  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  info.move_type = static_cast<MoveType>(move.type());
  info.half_moves = half_moves;
  history.push_back(info);

  status = check_position();

  return true;
}

bool Position::undo_move_internal() {
  if (history.empty()) return false;
  hash_history.pop_back();
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
  half_moves = info.half_moves;

  hash_key = hash_history.back();
  side_to_move = side_to_move == WHITE ? BLACK : WHITE;
  return true;
}

void Position::generate_legal_moves() {
  legal_moves = {};
  if (status != ONGOING) return;

  Position position = *this;
  std::vector<Move> psudo_move_list = generate_pseudo_legal_moves(*this);
  PieceColor king_color = position.side_to_move;

  for (const auto move : psudo_move_list) {
    position.make_move_internal(move);
    if (!is_king_attacked(position, king_color)) legal_moves.push_back(move);

    position.undo_move_internal();
  }
}