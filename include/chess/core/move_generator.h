#pragma once
#include <chess/core/attack_generator.h>
#include <chess/core/move.h>
#include <chess/core/position.h>

MoveList generate_legal_moves(Position &position);
void generate_pseudo_legal_moves(Position &position, MoveList &move_list);