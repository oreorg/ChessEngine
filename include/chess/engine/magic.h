#pragma once

#include <chess/core/position.h>

template <std::size_t TableSize>
struct Magic{
    U64 number[64];
    U64 mask[64];
    U64 attacks[64][TableSize];
};