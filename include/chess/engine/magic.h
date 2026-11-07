#pragma once

#include <chess/core/position.h>

template <std::size_t TableSize>
struct MagicTable{
    Bitboard number[64];
    Bitboard mask[64];
    Bitboard attacks[64][TableSize];
};