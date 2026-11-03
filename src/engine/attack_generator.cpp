#include <bit>
#include <chess/engine/attack_generator.h>
#include <iterator>
#include <random>

constexpr U64 BORDER_MASK = 0xFF818181818181FFULL;

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_B = 0x0202020202020202ULL;
constexpr U64 FILE_G = 0x4040404040404040ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;

std::mt19937_64 rng{666}; // Fixed seed makes magic search reproducible.

U64 knight_attacks[64];
U64 king_attacks[64];
U64 pawn_attacks[2][64];

Magic<512> bishop_magic;
Magic<4096> rook_magic;

void precompute_all_attacks() {
  precompute_knight_attacks();
  precompute_king_attacks();
  precompute_pawn_attacks();

  precompute_bishop_mask();
  find_bishop_magics();

  precompute_rook_mask();
  find_rook_magics();
}

void precompute_knight_attacks() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    U64 bitboard = 1ULL << square;
    U64 attacks = 0ULL;

    // KNIGHT WHEN ON LEFT SIDE
    if ((bitboard << 17) & ~FILE_A) attacks |= bitboard << 17;
    if ((bitboard << 10) & ~(FILE_A | FILE_B)) attacks |= bitboard << 10;
    if ((bitboard >> 15) & ~(FILE_A)) attacks |= bitboard >> 15;
    if ((bitboard >> 6) & ~(FILE_A | FILE_B)) attacks |= bitboard >> 6;

    // KNIGHT WHEN ON RIGHT SIDE
    if ((bitboard << 15) & ~FILE_H) attacks |= bitboard << 15;
    if ((bitboard << 6) & ~(FILE_G | FILE_H)) attacks |= bitboard << 6;
    if ((bitboard >> 17) & ~FILE_H) attacks |= bitboard >> 17;
    if ((bitboard >> 10) & ~(FILE_G | FILE_H)) attacks |= bitboard >> 10;

    knight_attacks[square] = attacks;
  }
}

void precompute_pawn_attacks() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    U64 bitboard = 1ULL << square;
    U64 attacks = 0ULL;
    if (bitboard << 7 & ~FILE_H) attacks |= bitboard << 7;
    if (bitboard << 9 & ~FILE_A) attacks |= bitboard << 9;

    pawn_attacks[0][square] = vertical_flip(attacks);
    pawn_attacks[1][square] = attacks;
  }
}

void precompute_king_attacks() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    U64 bitboard = 1ULL << square;
    U64 attacks = 0ULL;

    if ((bitboard << 1) & ~FILE_A) attacks |= bitboard << 1;
    if ((bitboard << 7) & ~FILE_H) attacks |= bitboard << 7;
    if ((bitboard << 9) & ~FILE_A) attacks |= bitboard << 9;
    if ((bitboard << 8)) attacks |= bitboard << 8;

    if ((bitboard >> 1) & ~FILE_H) attacks |= bitboard >> 1;
    if ((bitboard >> 7) & ~FILE_A) attacks |= bitboard >> 7;
    if ((bitboard >> 9) & ~FILE_H) attacks |= bitboard >> 9;
    if ((bitboard >> 8)) attacks |= bitboard >> 8;

    king_attacks[square] = attacks;
  }
}

void generate_all_blockers(U64 mask, std::vector<U64> &blockers) {
  U64 current = 0ULL;
  do {
    blockers.push_back(current);
    current = (current - mask) & mask;
  } while (current);
}

U64 generate_magic_number() { return rng() & rng() & rng(); }

U64 shift_bitboard(U64 bitboard, int direction) {
  switch (direction) {
  case 1:
    return (bitboard & ~FILE_H) << 1;
  case -1:
    return (bitboard & ~FILE_A) >> 1;
  case 8:
    return bitboard << 8;
  case -8:
    return bitboard >> 8;
  case 9:
    return (bitboard & ~FILE_H) << 9;
  case 7:
    return (bitboard & ~FILE_A) << 7;
  case -7:
    return (bitboard & ~FILE_H) >> 7;
  case -9:
    return (bitboard & ~FILE_A) >> 9;
  default:
    return 0;
  }
}

U64 slow_attacks(int square, U64 occupied, PieceType type) {
  const std::array<int, 4> directions =
      type == BISHOP ? std::array<int, 4>{9, 7, -7, -9} : std::array<int, 4>{1, 8, -1, -8};
  U64 attacks = 0ULL;
  for (const int direction : directions) {
    U64 bitboard = 1ULL << square;
    while ((bitboard = shift_bitboard(bitboard, direction)) != 0) {
      attacks |= bitboard;
      if (bitboard & occupied) break;
    }
  }
  return attacks;
}

void find_bishop_magics() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    const U64 mask = bishop_magic.mask[square];
    const int bits = count_bits(mask);

    std::vector<U64> blockers;
    generate_all_blockers(mask, blockers);

    std::vector<U64> attacks(blockers.size());
    for (std::size_t i = 0; i < blockers.size(); ++i) {
      attacks[i] = slow_attacks(square, blockers[i], BISHOP);
    }
    while (true) {
      const U64 magic = generate_magic_number();
      std::fill(std::begin(bishop_magic.attacks[square]), std::end(bishop_magic.attacks[square]), 0ULL);

      bool fail = false;

      for (std::size_t i = 0; i < blockers.size(); ++i) {
        const std::size_t index = (blockers[i] * magic >> (64 - bits));

        if (bishop_magic.attacks[square][index] && bishop_magic.attacks[square][index] != attacks[i]) {
          fail = true;
          break;
        }

        bishop_magic.attacks[square][index] = attacks[i];
      }

      if (!fail) {
        bishop_magic.number[square] = magic;
        break;
      }
    }
  }
}

void find_rook_magics() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    const U64 mask = rook_magic.mask[square];
    const int bits = count_bits(mask);

    std::vector<U64> blockers;
    generate_all_blockers(mask, blockers);

    std::vector<U64> attacks(blockers.size());
    for (std::size_t i = 0; i < blockers.size(); i++) {
      attacks[i] = slow_attacks(square, blockers[i], ROOK);
    }
    while (true) {
      const U64 magic = generate_magic_number();
      std::fill(std::begin(rook_magic.attacks[square]), std::end(rook_magic.attacks[square]), 0ULL);

      bool fail = false;

      for (std::size_t i = 0; i < blockers.size(); i++) {
        const std::size_t index = (blockers[i] * magic >> (64 - bits));

        if (rook_magic.attacks[square][index] && rook_magic.attacks[square][index] != attacks[i]) {
          fail = true;
          break;
        }

        rook_magic.attacks[square][index] = attacks[i];
      }

      if (!fail) {
        rook_magic.number[square] = magic;
        break;
      }
    }
  }
}

U64 relevant_sliding_mask(int square, PieceType type) {
  const std::array<int, 4> directions =
      type == BISHOP ? std::array<int, 4>{9, 7, -7, -9} : std::array<int, 4>{1, 8, -1, -8};

  U64 mask = 0ULL;
  for (const int direction : directions) {
    U64 bitboard = 1ULL << square;
    while (true) {
      bitboard = shift_bitboard(bitboard, direction);
      if (bitboard == 0ULL) break;

      const U64 next = shift_bitboard(bitboard, direction);
      if (next == 0ULL) break;

      mask |= bitboard;
    }
  }

  return mask;
}

void precompute_bishop_mask() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    bishop_magic.mask[square] = relevant_sliding_mask(square, BISHOP);
  }
}

void precompute_rook_mask() {
  for (int square = SQ_A1; square <= SQ_H8; square++) {
    rook_magic.mask[square] = relevant_sliding_mask(square, ROOK);
  }
}

bool is_king_attacked(Position &position, PieceColor kingColor) {
  U64 king_bitboard = position.piece_boards[KING + kingColor * 6];
  U64 occupancies = position.occupancies[BOTH];
  int square = popLSB(king_bitboard);
  int rook_index = ((rook_magic.mask[square] & occupancies) * rook_magic.number[square]) >>
                   (64 - count_bits(rook_magic.mask[square]));
  int bishop_index = ((bishop_magic.mask[square] & occupancies) * bishop_magic.number[square]) >>
                     (64 - count_bits(bishop_magic.mask[square]));

  PieceColor opponent = kingColor == WHITE ? BLACK : WHITE;
  U64 rook = rook_magic.attacks[square][rook_index];
  U64 bishop = bishop_magic.attacks[square][bishop_index];
  U64 knight = knight_attacks[square];
  U64 pawn = pawn_attacks[opponent][square];
  U64 king = king_attacks[square];

  if (bishop & (position.piece_boards[BISHOP + opponent * 6] | position.piece_boards[QUEEN + opponent * 6]))
    return true;
  if (rook & (position.piece_boards[ROOK + opponent * 6] | position.piece_boards[QUEEN + opponent * 6])) return true;
  if (knight & position.piece_boards[KNIGHT + opponent * 6]) return true;
  if (pawn & position.piece_boards[PAWN + opponent * 6]) return true;
  if (king & position.piece_boards[KING + opponent * 6]) return true;

  return false;
}

U64 get_knight_attacks(int square) { return knight_attacks[square]; }
U64 get_king_attacks(int square) { return king_attacks[square]; }

U64 get_pawn_attacks(int square, PieceColor color) { return pawn_attacks[color][square]; }

const Magic<512> &get_bishop_magic() { return bishop_magic; }

const Magic<4096> &get_rook_magic() { return rook_magic; }