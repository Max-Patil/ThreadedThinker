/*
    Attack - This module is specifically used to check whether or not, on a given square
    any piece from opponent side attacked or not
*/

#include "defs.h"

const int32_t KnightDir[8]   =   { -8, -19, -21, -12, 8, 19, 21, 12 };
const int32_t RookDir[4]     =   { -1, -10, 1, 10 };
const int32_t BishopDir[4]   =   { -9, -11, 11, 9 };
const int32_t KingDir[8]     =   { -1, -10, 1, 10, -9, -11, 11, 9 };

int32_t is_square_attacked(const board_t* pos, int32_t side, int32_t sq)
{
    int32_t piece = EMPTY, t_sq = 0, dir = 0, index = 0;

    ASSERT(check_board(pos));
    ASSERT(is_side_valid(side));
    ASSERT(sq_on_board(sq));


    //Checking whether or not sq is attacked by pawn in diagonal direction
    if(side == WHITE)
    {
        if(pos->board[sq - 11] == wP || pos->board[sq - 9] == wP)
            return true;
    }
    else 
    {
         if(pos->board[sq + 11] == bP || pos->board[sq + 9] == bP)
            return true;
    }
       

    //Checking whether or not sq is attacked by knight in 'L' direction
    for(index = 0; index < 8; index++)
    {
        piece = pos->board[sq + KnightDir[index]];
        if(piece != OFFBOARD && IS_KNIGHT(piece) && PieceColor[piece] == side)
            return true;
    }


    //Checking whether or not sq is attacked by rooks and queen in vertical and horizontal direction
    for(index = 0; index < 4; index++)
    {
        dir = RookDir[index];
        t_sq = sq + dir;
        piece = pos->board[t_sq];

        while(piece != OFFBOARD)
        {
            if(piece != EMPTY)
            {
                if(IS_RQ(piece) == true && PieceColor[piece] == side)
                    return true;
                break;
            }
            t_sq += dir;
            piece = pos->board[t_sq];
        }
    }


    //Checking whether or not sq is attacked by bishop and queen in diagonal direction
    for(index = 0; index < 4; index++)
    {
        dir = BishopDir[index];
        t_sq = sq + dir;
        piece = pos->board[t_sq];

        while(piece != OFFBOARD)
        {
            if(piece != EMPTY)
            {
                if(IS_BQ(piece) == true && PieceColor[piece] == side)
                    return true;
                break;
            }
            t_sq += dir;
            piece = pos->board[t_sq];
        }
    }

    //Checking whether or not sq is attacked by king in all direction only by one square
    for(index = 0; index < 8; ++index)
    {
        piece = pos->board[sq + KingDir[index]];
        if(piece != OFFBOARD && IS_KING(piece) == true && PieceColor[piece] == side)
            return true;
    }

    return false;
}

extern void show_attack_squares(const board_t* pos, int32_t side)
{
    int32_t rank = 0, file = 0, sq = 0;

    printf("\n\nSquares attacked by : %c\n", SideChars[side]);

    for(rank = RANK_8; rank >= RANK_1; rank--)
    {
        for(file = FILE_A; file <= FILE_H; file++)
        {
            sq = FROM_SQ64_TO_SQ120(file, rank);

            if(is_square_attacked(pos, side, sq))
            {
                printf("x");
            }
            else
                printf("-");
        }
        printf("\n");
    }
    printf("\n\n");
}
