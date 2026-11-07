#pragma once
#include <cstdint>
#include <random>

using Bitboard = std::uint64_t;

class ZobristHash {
public:
  Bitboard piece_keys[12][64];
  std::uint64_t side_key;
  std::uint64_t castling_keys[16];
  std::uint64_t ep_keys[8];

  ZobristHash() {
    std::mt19937_64 random_bitboard(1337);
    for (int piece = 0; piece < 12; piece++) {
      for (int square = 0; square < 64; square++) {
        piece_keys[piece][square] = random_bitboard();
      }
    }

    side_key = random_bitboard();

    for (int i = 0; i < 16; i++) {
      castling_keys[i] = random_bitboard();
    }

    for (int i = 0; i < 8; i++) {
      ep_keys[i] = random_bitboard();
    }
  }
};

inline ZobristHash ZOBRIST_KEYS;