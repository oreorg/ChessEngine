#pragma once
#include <chess/core/helper.h>
#include <cstdint>
#include <random>

class ZobristHash {
public:
  U64 piece_keys[12][64];
  U64 side_key;
  U64 castling_keys[16];
  U64 ep_keys[8];

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