#pragma once

#include <chess/core/position.h>
#include <chess/engine/move.h>
#include <vector>

std::vector<Move> generatePsuedoLegalMoves(const Position &position);
std::vector<Move> generateLegalMoves(const Position &position);

void generatePawnMoves(const Position &position, std::vector<Move> &moveList);
void generateKnightMoves(const Position &position, std::vector<Move> &moveList);
void generateKingMoves(const Position &position, std::vector<Move> &moveList);
void generateSlidingMoves(const Position &position, std::vector<Move> &moveList, PieceType type);