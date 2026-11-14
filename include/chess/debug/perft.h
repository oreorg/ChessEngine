#pragma once

#pragma once
#include <chess/core/position.h>
#include <cstdint>

namespace Debugging {
  // Standalone perft function operating on a Position reference
  U64 count_nodes(Position &pos, int depth);
  void perft(Position &pos, int depth);
} // namespace Debugging