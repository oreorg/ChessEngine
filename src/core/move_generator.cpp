#include <bit>
#include <chess/core/helper.h>
#include <chess/core/attack_generator.h>
#include <chess/core/move_generator.h>
#include <iterator>
#include <vector>

using namespace std;

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_B = 0x0202020202020202ULL;
constexpr U64 FILE_G = 0x4040404040404040ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;

constexpr U64 NOT_A = ~FILE_A;
constexpr U64 NOT_AB = ~(FILE_A | FILE_B);
constexpr U64 NOT_H = ~FILE_H;
constexpr U64 NOT_GH = ~(FILE_G | FILE_H);

constexpr U64 WHITE_START_RANK = 0x000000000000FF00ULL;
constexpr U64 BLACK_START_RANK = 0x00FF000000000000ULL;

MoveList generate_legal_moves(Position &position) {
  MoveList legal_moves;
  MoveList psuedo_moves;
  generate_pseudo_legal_moves(position, psuedo_moves);

  const PieceColor king_color = position.side_to_move;
  const PieceColor opponent = king_color == WHITE ? BLACK : WHITE;

  for (const Move move : psuedo_moves) {
    if (!position.make_move_internal(move)) continue;

    U64 king = king_color == WHITE ? position.piece_boards[KING] : position.piece_boards[KING + 6];
    const int square = pop_lsb(king);
    if (!is_square_attacked(position, square, opponent)) legal_moves.push_back(move);

    position.undo_move_internal();
  }
  return legal_moves;
}

void generate_pawn_moves(Position &position, MoveList &move_list) {
  const bool is_white = position.side_to_move == WHITE;
  const int index = position.side_to_move * 6;
  const int direction = !index ? 8 : -8;
  const U64 pawns = position.piece_boards[PAWN + index];
  const PieceColor opponent_color = is_white ? BLACK : WHITE;
  const U64 opponent = position.occupancies[opponent_color];
  const U64 occupied = position.occupancies[BOTH];

  auto add_moves = [&move_list, direction](U64 targets, int offset, MoveType type = NORMAL) {
    while (targets != 0) {
      int promotion_rank = direction == 8 ? 7 : 0;
      const int target = pop_lsb(targets);
      if (target / 8 == promotion_rank) {
        move_list.push_back({target - offset, target, PROMOTION, KNIGHT});
        move_list.push_back({target - offset, target, PROMOTION, BISHOP});
        move_list.push_back({target - offset, target, PROMOTION, ROOK});
        move_list.push_back({target - offset, target, PROMOTION, QUEEN});
      } else {
        move_list.push_back({target - offset, target, type});
      }
    }
  };

  auto shift_forward = [is_white](U64 board) { return is_white ? board << 8 : board >> 8; };

  add_moves(shift_forward(pawns) & ~occupied, direction);

  const U64 starting_pawns = pawns & (is_white ? WHITE_START_RANK : BLACK_START_RANK);
  const U64 clear_first_step = shift_forward(starting_pawns) & ~occupied;

  add_moves(shift_forward(clear_first_step) & ~occupied, 2 * direction);

  const U64 en_passant_square = 1ULL << position.en_passant_square;

  U64 capture_one = is_white ? ((pawns & ~FILE_A) << 7) : ((pawns & ~FILE_H) >> 7);
  U64 capture_two = is_white ? ((pawns & ~FILE_H) << 9) : ((pawns & ~FILE_A) >> 9);

  // generating enpassant moves
  if (capture_one & en_passant_square) {
    add_moves(capture_one & en_passant_square, is_white ? 7 : -7, EN_PASSANT);
  }
  if (capture_two & en_passant_square) {
    add_moves(capture_two & en_passant_square, is_white ? 9 : -9, EN_PASSANT);
  }
  // filter captures after generating enpassant move
  capture_one &= opponent;
  capture_two &= opponent;
  add_moves(capture_one, is_white ? 7 : -7, NORMAL);
  add_moves(capture_two, is_white ? 9 : -9, NORMAL);
}

void generate_knight_moves(Position &position, MoveList &move_list) {
  const int index = position.side_to_move * 6;
  const U64 player_pieces = position.occupancies[position.side_to_move];

  U64 knights = position.piece_boards[KNIGHT + index];
  U64 attacks;

  while (knights) {
    const int square = pop_lsb(knights);
    attacks = get_knight_attacks(square);
    attacks &= ~player_pieces;
    while (attacks) {
      const int target = pop_lsb(attacks);
      move_list.push_back({square, target});
    }
  }
}

void generate_king_moves(Position &position, MoveList &move_list) {
  const int index = position.side_to_move * 6;
  const U64 player_pieces = position.occupancies[position.side_to_move];
  const PieceColor opponent = position.side_to_move == WHITE ? BLACK : WHITE;

  U64 king = position.piece_boards[KING + index];
  int square = pop_lsb(king);

  U64 attacks = get_king_attacks(square);
  attacks &= ~player_pieces;
  while (attacks) {
    const int target = pop_lsb(attacks);
    move_list.push_back({square, target});
  }
  bool is_white = position.side_to_move == WHITE;
  const uint8_t king_side = is_white ? WHITE_KING_SIDE : BLACK_KING_SIDE;
  const uint8_t queen_side = is_white ? WHITE_QUEEN_SIDE : BLACK_QUEEN_SIDE;
  const int rank_offset = is_white ? 0 : 56;
  const int king_side_square = SQ_G1 + rank_offset;
  const int queen_side_square = SQ_C1 + rank_offset;
  const U64 king_side_path = 0x60ULL << rank_offset;
  const U64 queen_side_path = 0x0EULL << rank_offset;

  const bool king_side_clear = !is_square_attacked(position, SQ_F1 + rank_offset, opponent) &&
                               !is_square_attacked(position, SQ_G1 + rank_offset, opponent) &&
                               !is_square_attacked(position, SQ_E1 + rank_offset, opponent);

  const bool queen_side_clear = !is_square_attacked(position, SQ_D1 + rank_offset, opponent) &&
                                !is_square_attacked(position, SQ_C1 + rank_offset, opponent) &&
                                !is_square_attacked(position, SQ_E1 + rank_offset, opponent);

  if ((position.castling_rights & king_side) && king_side_clear) {
    if (!(position.occupancies[BOTH] & king_side_path)) move_list.push_back({square, king_side_square, CASTLE, KING_SIDE});
  }

  if ((position.castling_rights & queen_side) && queen_side_clear) {
    if (!(position.occupancies[BOTH] & queen_side_path)) move_list.push_back({square, queen_side_square, CASTLE, QUEEN_SIDE});
  }
}

template <std::size_t TableSize>
void generate_sliding_moves_impl(Position &position, MoveList &move_list, U64 pieces, const MagicTable<TableSize> &magic) {
  const U64 player_pieces = position.occupancies[position.side_to_move];
  U64 attacks;
  while (pieces) {
    const int square = pop_lsb(pieces);
    U64 mask = magic.mask[square];
    U64 number = magic.number[square];
    int bits = count_bits(mask);
    std::size_t index = ((mask & position.occupancies[BOTH]) * number) >> (64 - bits);
    attacks = magic.attacks[square][index];
    attacks &= ~player_pieces;
    while (attacks) {
      const int target = pop_lsb(attacks);
      move_list.push_back({square, target});
    }
  }
}

void generate_sliding_moves(Position &position, MoveList &move_list, PieceType type) {
  const U64 queens = position.piece_boards[QUEEN + position.side_to_move * 6];

  if (type == BISHOP) {
    const U64 bishops = position.piece_boards[BISHOP + position.side_to_move * 6];
    generate_sliding_moves_impl(position, move_list, bishops | queens, get_bishop_magic());
  } else if (type == ROOK) {
    const U64 rooks = position.piece_boards[ROOK + position.side_to_move * 6];
    generate_sliding_moves_impl(position, move_list, rooks | queens, get_rook_magic());
  }
}

void generate_pseudo_legal_moves(Position &position, MoveList &move_list) {
  generate_pawn_moves(position, move_list);
  generate_knight_moves(position, move_list);
  generate_king_moves(position, move_list);
  generate_sliding_moves(position, move_list, BISHOP);
  generate_sliding_moves(position, move_list, ROOK);
}
