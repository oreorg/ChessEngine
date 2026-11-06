#include <bit>
#include <chess/core/helper.h>
#include <chess/engine/attack_generator.h>
#include <chess/engine/move_generator.h>
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

vector<Move> generatePsuedoLegalMoves(const Position &position) {
  vector<Move> moveList = {};
  generatePawnMoves(position, moveList);
  generateKnightMoves(position, moveList);
  generateKingMoves(position, moveList);
  generateSlidingMoves(position, moveList, BISHOP);
  generateSlidingMoves(position, moveList, ROOK);
  return moveList;
}

vector<Move> generateLegalMoves(const Position &position) {
  Position current_position = position;
  vector<Move> legal_moves = {};
  vector<Move> move_list = generatePsuedoLegalMoves(current_position);
  PieceColor king_color = current_position.side_to_move;

  for (const auto move : move_list) {
    current_position.make_move(move);
    if (!is_king_attacked(current_position, king_color)) legal_moves.push_back(move);

    current_position.undo_move();
  }
  return legal_moves;
}

void generatePawnMoves(const Position &position, vector<Move> &moveList) {
  const bool isWhite = position.side_to_move == WHITE;
  const int index = position.side_to_move * 6;
  const int direction = !index ? 8 : -8;
  const U64 pawns = position.piece_boards[PAWN + index];
  const PieceColor opponentColor = isWhite ? BLACK : WHITE;
  const U64 opponent = position.occupancies[opponentColor];
  const U64 occupied = position.occupancies[BOTH];

  auto addMoves = [&moveList, direction](U64 targets, int offset, MoveType type = NORMAL) {
    while (targets != 0) {
      int protomotionRank = direction == 8 ? 7 : 0;
      const int target = popLSB(targets);
      if (target / 8 == protomotionRank) {
        moveList.push_back({target - offset, target, PROMOTION, KNIGHT});
        moveList.push_back({target - offset, target, PROMOTION, BISHOP});
        moveList.push_back({target - offset, target, PROMOTION, ROOK});
        moveList.push_back({target - offset, target, PROMOTION, QUEEN});
      }
      moveList.push_back({target - offset, target, type});
    }
  };

  auto shiftForward = [isWhite](U64 board) { return isWhite ? board << 8 : board >> 8; };

  addMoves(shiftForward(pawns) & ~occupied, direction);

  const U64 startingPawns = pawns & (isWhite ? WHITE_START_RANK : BLACK_START_RANK);
  const U64 clearFirstStep = shiftForward(startingPawns) & ~occupied;

  addMoves(shiftForward(clearFirstStep) & ~occupied, 2 * direction);

  const U64 enpassant_square = 1ULL << position.enpassant_square;

  U64 captureOne = isWhite ? ((pawns & ~FILE_A) << 7) : ((pawns & ~FILE_H) >> 7);
  U64 captureTwo = isWhite ? ((pawns & ~FILE_H) << 9) : ((pawns & ~FILE_A) >> 9);

  // generating enpassant moves
  if (captureOne & enpassant_square) {
    addMoves(captureOne & enpassant_square, isWhite ? 7 : -7, ENPASSANT);
  }
  if (captureTwo & enpassant_square) {
    addMoves(captureTwo & enpassant_square, isWhite ? 9 : -9, ENPASSANT);
  }
  // filter captures after generating enpassant move
  captureOne &= opponent;
  captureTwo &= opponent;
  addMoves(captureOne, isWhite ? 7 : -7, NORMAL);
  addMoves(captureTwo, isWhite ? 9 : -9, NORMAL);
}

void generateKnightMoves(const Position &position, vector<Move> &moveList) {
  const int index = position.side_to_move * 6;
  const U64 player_pieces = position.occupancies[position.side_to_move];

  U64 knights = position.piece_boards[KNIGHT + index];
  U64 attacks;

  while (knights) {
    const int square = popLSB(knights);
    attacks = get_knight_attacks(square);
    attacks &= ~player_pieces;
    while (attacks) {
      const int target = popLSB(attacks);
      moveList.push_back({square, target});
    }
  }
}

void generateKingMoves(const Position &position, vector<Move> &moveList) {
  const int index = position.side_to_move * 6;
  const U64 player_pieces = position.occupancies[position.side_to_move];

  U64 king = position.piece_boards[KING + index];
  int square = popLSB(king);

  U64 attacks = get_king_attacks(square);
  attacks &= ~player_pieces;
  while (attacks) {
    const int target = popLSB(attacks);
    moveList.push_back({square, target});
  }
  // King Special Moves
  if (is_king_attacked(position, position.side_to_move)) return;
  const uint8_t king_side = position.side_to_move == WHITE ? WHITE_KING_SIDE : BLACK_KING_SIDE;
  const uint8_t queen_side = position.side_to_move == WHITE ? WHITE_QUEEN_SIDE : BLACK_QUEEN_SIDE;
  const int rank_offset = position.side_to_move == WHITE ? 0 : 56;
  const int king_side_square = SQ_G1 + rank_offset;
  const int queen_side_square = SQ_C1 + rank_offset;
  const U64 king_side_path = 0x60ULL << rank_offset;
  const U64 queen_side_path = 0x0EULL << rank_offset;
  if (position.castling_right & king_side) {
    if (!(position.occupancies[BOTH] & king_side_path))
      moveList.push_back({square, king_side_square, CASTLE, KING_SIDE});
  }

  if (position.castling_right & queen_side) {
    if (!(position.occupancies[BOTH] & queen_side_path))
      moveList.push_back({square, queen_side_square, CASTLE, QUEEN_SIDE});
  }
}

template <std::size_t TableSize>
void generateSlidingMovesImpl(const Position &position, vector<Move> &moveList, U64 pieces,
                              const Magic<TableSize> &magic) {
  const U64 player_pieces = position.occupancies[position.side_to_move];
  U64 attacks;
  while (pieces) {
    const int square = popLSB(pieces);
    U64 mask = magic.mask[square];
    U64 number = magic.number[square];
    int bits = count_bits(mask);
    std::size_t index = ((mask & position.occupancies[BOTH]) * number) >> (64 - bits);
    attacks = magic.attacks[square][index];
    attacks &= ~player_pieces;
    while (attacks) {
      const int target = popLSB(attacks);
      moveList.push_back({square, target});
    }
  }
}

void generateSlidingMoves(const Position &position, vector<Move> &moveList, PieceType type) {
  const U64 queens = position.piece_boards[QUEEN + position.side_to_move * 6];

  if (type == BISHOP) {
    const U64 bishops = position.piece_boards[BISHOP + position.side_to_move * 6];
    generateSlidingMovesImpl(position, moveList, bishops | queens, get_bishop_magic());
  } else if (type == ROOK) {
    const U64 rooks = position.piece_boards[ROOK + position.side_to_move * 6];
    generateSlidingMovesImpl(position, moveList, rooks | queens, get_rook_magic());
  }
}