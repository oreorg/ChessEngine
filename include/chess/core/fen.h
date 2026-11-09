#pragma once
#include <chess/core/position.h>
#include <chess/core/types.h>
#include <string_view>

PieceType get_piece_type_by_char(char character);
Position parse_fen(std::string_view fen);