#pragma once

#include <chess/core/position.h>
#include <chess/engine/move.h>
#include <vector>

MoveList generate_legal_moves(Position &position);

void generate_pseudo_legal_moves(Position &position, MoveList &move_list);
void generate_pawn_moves(Position &position, MoveList &move_list);
void generate_knight_moves(Position &position, MoveList &move_list);
void generate_king_moves(Position &position, MoveList &move_list);
void generate_sliding_moves(Position &position, MoveList &move_list, PieceType type);