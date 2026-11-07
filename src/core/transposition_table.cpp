
#include <chess/core/transposition_table.h>

int random_table[12][64];

void init_tt() {
  for (auto &piece_values : random_table) {
    for (auto &square_value : piece_values) {
      square_value = 1;
    }
  }
}
