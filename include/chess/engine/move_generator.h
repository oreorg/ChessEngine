#pragma once

#include <chess/core/position.h>
#include <chess/engine/move.h>
#include <vector>

std::vector<Move> generate_pseudo_legal_moves(const Position &position);
std::vector<Move> generate_legal_moves(const Position &position);

void generate_pawn_moves(const Position &position, std::vector<Move> &move_list);
void generate_knight_moves(const Position &position, std::vector<Move> &move_list);
void generate_king_moves(const Position &position, std::vector<Move> &move_list);
void generate_sliding_moves(const Position &position, std::vector<Move> &move_list, PieceType type);