/*
    Bitboard - This module is used for holding information associated to pawn
    pieces so that using bit manipulation, we can able to identify pattern or
    structure of pawn for evaluations
*/

#include "defs.h"

const int BitTable[64] = {
    63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61,
    29, 2, 51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 
    4, 49, 5, 52, 26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 
    14,12, 55, 38, 28, 58, 20, 37, 17, 36, 8
};

extern inline int pop_bit(uint64_t* pos)
{
    uint64_t bb = *pos ^ (*pos - 1);
    uint32_t fold = (unsigned)((bb & 0xffffffff) ^ (bb >> 32));

    *pos = *pos & (*pos - 1);
    return BitTable[(fold * 0x783a9b23) >> 26];
}

extern inline int count_bits(uint64_t bb)
{
    size_t count = 0;
    for(count = 0; bb; count++, bb = bb & (bb - 1));

    return count;
}

extern void print_bitboard(uint64_t bb64)
{
    uint64_t shift_me = 1ULL;
    printf("\n");
    for(int rank = RANK_8; rank >= RANK_1; rank--)
    {
        for(int file = FILE_A; file <= FILE_H; file++)
        {
            int sq = FROM_SQ64_TO_SQ120(file, rank);
            int sq64 = SQ64(sq);

            if(bb64 & (shift_me << sq64))
                printf("x");
            else
                printf("-");
        }
        printf("\n");
    }
    printf("\n");
}