#include <chess/engine/attack_generator.h>
#include <random>

constexpr U64 BORDER_MASK = 0xFF818181818181FFULL;


constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_B = 0x0202020202020202ULL;
constexpr U64 FILE_G = 0x4040404040404040ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;

std::mt19937_64 rng{666}; // Fixed seed makes magic search reproducible.

U64 knight_attacks[64];


Magic<512> bishop;
Magic<4096> rook;

void precompute_all_attacks(){
    precompute_knight_attacks();

    precompute_bishop_mask();
    find_bishop_magics();

    precompute_rook_mask();
    find_rook_magics();
}

void precompute_knight_attacks(){
    for(int square = SQ_A1; square <= SQ_H8; square++){
        U64 bitboard = 1ULL << square;
        U64 attacks = 0ULL;

        //KNIGHT WHEN ON LEFT SIDE
        if((bitboard << 17) & ~FILE_A) attacks |= bitboard << 17;
        if((bitboard << 10) & ~(FILE_A | FILE_B)) attacks |= bitboard << 10;
        if((bitboard >> 15) & ~(FILE_A)) attacks |= bitboard >> 15; 
        if((bitboard >> 6) & (FILE_A | FILE_B)) attacks |= bitboard >> 6;

        //KNIGHT WHEN ON RIGHT SIDE
        if((bitboard << 15) & ~FILE_H) attacks |= bitboard << 15;
        if((bitboard << 6) & ~(FILE_G | FILE_H)) attacks |= bitboard << 6;
        if((bitboard >> 17) & ~FILE_H) attacks |= bitboard >> 17;
        if((bitboard >> 10) & ~(FILE_G | FILE_H)) attacks |= bitboard >> 10;

        knight_attacks[square] = attacks;
    }
}

void generate_all_blockers(U64 mask, std::vector<U64> &blockers){
    U64 current = 0ULL;
    do {
        blockers.push_back(current);
        current = (current - mask) & mask;
    } while(current);
}

U64 generate_magic_number(){
    return rng() & rng() & rng();
}

U64 shift_bitboard(U64 bitboard, int direction) {
    switch (direction) {
    case 1:  return (bitboard & ~FILE_H) << 1;
    case -1: return (bitboard & ~FILE_A) >> 1;
    case 8:  return bitboard << 8;
    case -8: return bitboard >> 8;
    case 9:  return (bitboard & ~FILE_H) << 9;
    case 7:  return (bitboard & ~FILE_A) << 7;
    case -7: return (bitboard & ~FILE_H) >> 7;
    case -9: return (bitboard & ~FILE_A) >> 9;
    default: return 0;
    }
}

U64 slow_attacks(int square, U64 occupied, PieceType type){
   const std::array<int, 4> directions =
    type == BISHOP
        ? std::array<int, 4>{9, 7, -7, -9}
        : std::array<int, 4>{1, 8, -1, -8};
    U64 attacks = 0ULL;
    for(const int direction:directions){
        U64 bitboard = 1ULL << square;
        while((bitboard = shift_bitboard(bitboard, direction)) !=0){            
            attacks |= bitboard;
            if(bitboard & occupied) break;
        }
    }
    return attacks;
}

void find_bishop_magics(){
    for(int square = SQ_A1; square <= SQ_H8; square++){
        const U64 mask = bishop.mask[square];
        const int bits = count_bits(mask);

        std::vector<U64> blockers;
        generate_all_blockers(mask,blockers);

        std::vector<U64> attacks(blockers.size());
        for(std::size_t i = 0; i < blockers.size(); ++i){
            attacks[i] = slow_attacks(square, blockers[i],BISHOP);
        }
        while (true){
            const U64 magic = generate_magic_number();
            std::fill(std::begin(bishop.attacks[square]),std::end(bishop.attacks[square]), 0ULL);

            bool fail = false;

            for(std::size_t i = 0; i < blockers.size(); ++i){
                const std::size_t index = (blockers[i] * magic >> (64 - bits));

                if(bishop.attacks[square][index]){
                    fail = true;
                    break;
                }

                bishop.attacks[square][index] = attacks[i];
            }

            if(!fail){
                bishop.number[square] = magic;
                break;
            }
        }
    }
}


void find_rook_magics(){
    for(int square = SQ_A1; square <= SQ_H8; square++){
        const U64 mask = rook.mask[square];
        const int bits = count_bits(mask);

        std::vector<U64> blockers;
        generate_all_blockers(mask,blockers);

        std::vector<U64> attacks(blockers.size());
        for(std::size_t i = 0; i < blockers.size(); ++i){
            attacks[i] = slow_attacks(square, blockers[i], ROOK);
        }
        while (true){
            const U64 magic = generate_magic_number();
            std::fill(std::begin(rook.attacks[square]),std::end(rook.attacks[square]), 0ULL);

            bool fail = false;

            for(std::size_t i = 0; i < blockers.size(); ++i){
                const std::size_t index = (blockers[i] * magic >> (64 - bits));

                if(rook.attacks[square][index]){
                    fail = true;
                    break;
                }

                rook.attacks[square][index] = attacks[i];
            }

            if(!fail){
                rook.number[square] = magic;
                break;
            }
        }
    }
}

void precompute_bishop_mask(){
    constexpr int directions[] = {9, 7, -7, -9};

    for(int square = SQ_A1; square <= SQ_H8; square++){
        U64 attacks = 0ULL;

        for(const int direction : directions){
            U64 ray = 1ULL << square;
            while(true){
                if(direction > 0){
                    ray = (ray & ~FILE_H) << direction;
                }else{
                    ray = (ray & ~FILE_A) >> -direction;
                }
                if(ray == 0ULL) break;
                attacks |= ray;
            }
        }
        bishop.mask[square] = attacks & ~BORDER_MASK;
    }
}

void precompute_rook_mask(){
    constexpr int directions[] = {1, 8, -1, -8};

    for(int square = SQ_A1; square <= SQ_H8; square++){
        U64 attacks = 0ULL;

        for(const int direction : directions){
            U64 ray = 1ULL << square;
            while(true){
                if(direction > 0){
                    ray = (ray & ~FILE_A) << direction;
                }else{
                    ray = (ray & ~FILE_H) >> -direction;
                }
                if(ray == 0ULL) break;
                attacks |= ray;
            }
        }
        rook.mask[square] = attacks & ~BORDER_MASK;
    }
}



U64 get_knight_attacks(int square){
    return knight_attacks[square];
}

const Magic<512>& get_bishop_magic(){
    return bishop;
}

const Magic<4096>& get_rook_magic(){
    return rook;
}