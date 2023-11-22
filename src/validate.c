/*
    Validate - As its name sound, this module is used to check validity of chess board entities
*/

#include "defs.h"

extern inline bool sq_on_board(const int32_t sq)
{
    return (SQ64(sq) != OFFBOARD);
}

extern inline bool is_side_valid(const int32_t side)
{
    return ((side == WHITE || side == BLACK) ? true : false);
}

extern inline bool is_file_rank_valid(const int32_t fr)
{
    return ((fr >= 0 && fr <= 7) ? true : false);
}

extern inline bool is_piece_valid(const int32_t piece)
{
    return ((piece >= wP && piece <= bK) ? true : false);
}