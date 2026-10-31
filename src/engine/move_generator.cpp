#include <chess/engine/move_generator.h>
#include <chess/engine/attack_generator.h>
#include <bit>
#include <vector>
#include <iterator>

using namespace std;

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_B = 0x0202020202020202ULL;
constexpr U64 FILE_G = 0x4040404040404040ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;

constexpr U64 NOT_A    = ~FILE_A;
constexpr U64 NOT_AB   = ~(FILE_A | FILE_B);
constexpr U64 NOT_H    = ~FILE_H;
constexpr U64 NOT_GH   = ~(FILE_G | FILE_H);

constexpr U64 WHITE_START_RANK = 0x000000000000FF00ULL;
constexpr U64 BLACK_START_RANK = 0x00FF000000000000ULL;

vector<Move> generateLegalMoves(const Position position){
    vector <Move> moveList = {};
    generatePawnMoves(position, moveList);
    generateKnightMoves(position, moveList);
    generateSlidingMoves(position, moveList, BISHOP);
    generateSlidingMoves(position, moveList, ROOK);
    return moveList;
}



void generatePawnMoves(const Position position, vector<Move>& moveList){
    const bool isWhite = position.side_to_move == WHITE;
    const int direction = isWhite ? 8 : -8;
    const U64 pawns = position.piece_boards[isWhite? PAWN : PAWN + 6];
    const U64 opponent = position.occupancies[isWhite ? BLACK :WHITE];
    const U64 occupied = position.occupancies[BOTH];
    
    auto addMoves = [&moveList](U64 targets, int offset){
        while (targets != 0){
            const int target = popLSB(targets);
            moveList.push_back({target-offset, target});
        }
    };

    auto shiftForward = [isWhite](U64 board){
        return isWhite? board << 8 : board >> 8;
    };

    addMoves(shiftForward(pawns) & ~occupied, direction);

    const U64 startingPawns = pawns & (isWhite ? WHITE_START_RANK : BLACK_START_RANK);
    const U64 clearFirstStep = shiftForward(startingPawns) & ~occupied;

    addMoves(shiftForward(clearFirstStep) & ~occupied, 2 * direction);

    const U64 captureOne = isWhite
    ?((pawns & ~FILE_A) << 7) & opponent
    :((pawns & ~FILE_H) >> 7) & opponent;

    const U64 captureTwo = isWhite
    ?((pawns & ~FILE_H) << 9) & opponent
    :((pawns & ~FILE_A) >> 9) & opponent;

    addMoves(captureOne, isWhite? 7 : -7);
    addMoves(captureTwo, isWhite? 9 : -9);
}

void generateKnightMoves(const Position position, vector<Move>& moveList){
    const bool isWhite =  position.side_to_move == WHITE;
    const U64 player_pieces = position.occupancies[position.side_to_move];

    U64 knights = position.piece_boards[isWhite? KNIGHT : KNIGHT + 6];
    U64 attacks;

    while(knights){
        const int square = popLSB(knights);
        attacks = get_knight_attacks(square);
        attacks &= ~player_pieces;
        while (attacks){
            const int target = popLSB(attacks);
            moveList.push_back({square, target});
        }
    }
}

template<std::size_t TableSize>
void generateSlidingMovesImpl(const Position& position, vector<Move>& moveList, PieceType type, const Magic<TableSize>& magic){
    const bool isWhite = position.side_to_move == WHITE;
    U64 player_pieces = position.occupancies[position.side_to_move];
    U64 pieces = position.piece_boards[isWhite? type : type + 6] | position.piece_boards[isWhite ? QUEEN : QUEEN + 6];
    U64 attacks;
    while(pieces){
        const int square = popLSB(pieces);
        U64 mask = magic.mask[square];
        U64 number = magic.number[square];

        int bits = count_bits(mask);
        int index = ((mask & position.occupancies[BOTH]) * number) >> (64 - bits);
        attacks = magic.attacks[square][index];
        attacks &= ~player_pieces;
        while(attacks){
            const int target = popLSB(attacks);
            moveList.push_back({square,target});
        }
    }
}

void generateSlidingMoves(Position position, vector<Move>& moveList, PieceType type){
    if(type == BISHOP){
        generateSlidingMovesImpl(position, moveList, type, get_bishop_magic());
    } else if(type == ROOK){
        generateSlidingMovesImpl(position, moveList, type, get_rook_magic());
    }
}