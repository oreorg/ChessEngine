#pragma once

#include <chess/core/types.h>

struct Move{
    int startSquare;
    int targetSquare;
};

struct UndoInfo{
    int startSquare;
    int targetSquare;
    PieceType capturedPiece;
    bool isEnpassant;
};