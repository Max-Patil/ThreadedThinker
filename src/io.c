/*
    I/O - This module is used to display chess board information such square, move and movelist that is encoded as binary format
*/

#include "defs.h"

// Printing square in algebric notation
extern char *print_square(const int32_t sq)
{
    static char square[3];

    int32_t file = FilesBoard[sq];
    int32_t rank = RanksBoard[sq];

    sprintf(square, "%c%c", ('a' + file), ('1' + rank));

    return square;
}

// Printing move in algebric notation with promoted piece
extern char *print_move(const int32_t move)
{
    static char move_str[6];

    int32_t file_from = FilesBoard[FROM_SQ(move)];
    int32_t rank_from = RanksBoard[FROM_SQ(move)];
    int32_t file_to = FilesBoard[TO_SQ(move)];
    int32_t rank_to = RanksBoard[TO_SQ(move)];

    int32_t promoted = PROMOTED(move);

    // printtf("promoted Piece : %c\n", PiecesChars[promoted]);

    if (promoted)
    {
        char prom = 'q';
        if (IS_KNIGHT(promoted))
        {
            prom = 'n';
        }
        else if (IS_RQ(promoted) && !IS_BQ(promoted))
        {
            prom = 'r';
        }
        else if (!IS_RQ(promoted) && IS_BQ(promoted))
        {
            prom = 'b';
        }

        sprintf(move_str, "%c%c%c%c%c", ('a' + file_from), ('1' + rank_from), ('a' + file_to), ('1' + rank_to), prom);
    }
    else
        sprintf(move_str, "%c%c%c%c", ('a' + file_from), ('1' + rank_from), ('a' + file_to), ('1' + rank_to));

    // Note : Irrespective of side, we will denote promoted piece in small character only.

    return move_str;
}

extern void print_move_list(move_list_t *p_list)
{
    int idx = 0, score = 0, move = NO_MOVE;

    printf("\nMove List : Total %d Moves\n", p_list->count);

    for (idx = 0; idx < p_list->count; idx++)
    {
        move = p_list->moves[idx].move;
        score = p_list->moves[idx].score;
        printf("Move %3d -> %6s (score : %d)\n", idx + 1, print_move(move), score);
    }
}

extern int32_t parse_move(const board_t *pos, char *s_move)
{
    // algebric move -> a2a4
    if (s_move[1] > '8' || s_move[1] < '1')
        return NO_MOVE;

    if (s_move[3] > '8' || s_move[3] < '1')
        return NO_MOVE;

    if (s_move[0] > 'h' || s_move[0] < 'a')
        return NO_MOVE;

    if (s_move[2] > 'h' || s_move[2] < 'a')
        return NO_MOVE;

    int32_t from = FROM_SQ64_TO_SQ120(s_move[0] - 'a', s_move[1] - '1');
    int32_t to =   FROM_SQ64_TO_SQ120(s_move[2] - 'a', s_move[3] - '1');

    ASSERT(sq_on_board(from) && sq_on_board(to));

    move_list_t list;
    generate_all_moves(pos, &list);

    int32_t move = NO_MOVE, prom_piece = EMPTY;

    for (int move_num = 0; move_num < list.count; move_num++)
    {
        move = list.moves[move_num].move;

        if (FROM_SQ(move) == from && TO_SQ(move) == to)
        {
            prom_piece = PROMOTED(move);
            if (prom_piece)
            {
                if (IS_RQ(prom_piece) && !IS_BQ(prom_piece) && s_move[4] == 'r')
                    return move;

                else if (!IS_RQ(prom_piece) && IS_BQ(prom_piece) && s_move[4] == 'b')
                    return move;

                else if (IS_RQ(prom_piece) && IS_BQ(prom_piece) && s_move[4] == 'q')
                    return move;

                else if (IS_KNIGHT(prom_piece) && s_move[4] == 'n')
                    return move;

                continue;
            }

            return move;
        }
    }

    return NO_MOVE;
}