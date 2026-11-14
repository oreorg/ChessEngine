#pragma once

#include <chess/core/position.h>
#include <chess/core/magic.h>
#include <vector>

void precompute_all_attacks();
void generate_all_blockers(U64 mask, std::vector<U64> &blockers);
U64 generate_magic_number();

void precompute_knight_attacks();
void precompute_king_attacks();
void precompute_pawn_attacks();

void precompute_bishop_mask();
void find_bishop_magics();

void precompute_rook_mask();
void find_rook_magics();

U64 slow_attacks(int square, U64 occupied, PieceType type);

const MagicTable<512> &get_bishop_magic();
const MagicTable<4096> &get_rook_magic();

U64 get_knight_attacks(int square);
U64 get_king_attacks(int square);
U64 get_pawn_attacks(int square, PieceColor color);

U64 shift_bitboard(U64 bitboard, int direction);

bool is_square_attacked(Position &position, int square, PieceColor opponent);