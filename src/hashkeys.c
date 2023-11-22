/*
    Hashkeys - This module is used for generating hash keys for given state of position
*/

#include "defs.h"

extern uint64_t generate_pos_key(const board_t* pos)
{
    int sq = 0, piece = EMPTY;
    uint64_t final_key = 0;

    for(int sq = 0; sq < BOARD_SQUARES; sq++)
    {
        piece = pos->board[sq];
        if(piece != OFFBOARD && piece != EMPTY)
        {
            ASSERT(piece >= wP && piece <= bK);
            final_key ^= PiecePositionKeys[piece][sq];
        }
    }

    if(pos->side == WHITE)
        final_key ^= SideKey;
    
    if(pos->enpassant_sq != NO_SQ)
    {
        ASSERT(pos->enpassant_sq >= 0 && pos->enpassant_sq < BOARD_SQUARES);
        ASSERT(RanksBoard[pos->enpassant_sq] == RANK_3 || RanksBoard[pos->enpassant_sq] == RANK_6);
        final_key ^= PiecePositionKeys[EMPTY][pos->enpassant_sq];
    }

    ASSERT(pos->castle_perm >= 0 && pos->castle_perm <= 15);
    final_key ^= CastleKeys[pos->castle_perm];

    return final_key;
}
