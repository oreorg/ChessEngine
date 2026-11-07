#pragma once

#include <chess/core/position.h>
#include <chess/engine/magic.h>
#include <iostream>
#include <vector>

void precompute_all_attacks();
void generate_all_blockers(Bitboard mask, std::vector<Bitboard> &blockers);
Bitboard generate_magic_number();

void precompute_knight_attacks();
void precompute_king_attacks();
void precompute_pawn_attacks();

void precompute_bishop_mask();
void find_bishop_magics();

void precompute_rook_mask();
void find_rook_magics();

Bitboard slow_attacks(int square, Bitboard occupied, PieceType type);

const MagicTable<512> &get_bishop_magic();
const MagicTable<4096> &get_rook_magic();

Bitboard get_knight_attacks(int square);
Bitboard get_king_attacks(int square);
Bitboard get_pawn_attacks(int square, PieceColor color);

Bitboard shift_bitboard(Bitboard bitboard, int direction);

bool is_king_attacked(Position position, PieceColor king_color);