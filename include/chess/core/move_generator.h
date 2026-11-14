#pragma once

#include <chess/core/position.h>
#include <chess/core/move.h>

MoveList generate_legal_moves(Position &position);
void generate_pseudo_legal_moves(Position &position, MoveList &move_list);