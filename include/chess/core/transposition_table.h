#pragma once

#include <cstdint>

extern int random_table[12][64];

struct TranspositionTableEntry {
  std::uint64_t hash;
};

void init_tt();