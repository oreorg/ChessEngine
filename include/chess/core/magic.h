#pragma once

#include <chess/core/helper.h>

template <std::size_t TableSize>
struct MagicTable{
    U64 number[64];
    U64 mask[64];
    U64 attacks[64][TableSize];
};