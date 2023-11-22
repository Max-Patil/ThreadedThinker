/*
    Move Generator - This module is used to generate appropriate moves and store them in 
    move list or verify move list
*/

#include "defs.h"

// If side is white, then go from 0 to 2 else go from 4 to 6
//                          0, 1,  2,  3, 4,  5,  6,  7
const int SlidePiece[8] = {wB, wR, wQ, 0, bB, bR, bQ, 0};
const int SlideIndex[2] = {0, 4};

const int nonSlidePiece[6] = {wN, wK, 0, bN, bK, 0};
const int nonSlideIndex[2] = {0, 3};

const int PieceDir[13][8] = {
    // For white side
    {0, 0, 0, 0, 0, 0, 0, 0},           // Empty
    {0, 0, 0, 0, 0, 0, 0, 0},           // Pawn
    {-8, -19, -21, -12, 8, 19, 21, 12}, // Knight
    {-9, -11, 11, 9, 0, 0, 0, 0},       // Bishop
    {-1, -10, 1, 10, 0, 0, 0, 0},       // Rook
    {-1, -10, 1, 10, -9, -11, 11, 9},   // Queen
    {-1, -10, 1, 10, -9, -11, 11, 9},   // King

    // For black side
    {0, 0, 0, 0, 0, 0, 0, 0},           // Pawn
    {-8, -19, -21, -12, 8, 19, 21, 12}, // Knight
    {-9, -11, 11, 9, 0, 0, 0, 0},       // Bishop
    {-1, -10, 1, 10, 0, 0, 0, 0},       // Rook
    {-1, -10, 1, 10, -9, -11, 11, 9},   // Queen
    {-1, -10, 1, 10, -9, -11, 11, 9},   // King
};

// How many direction for each piece type will be represented using number of direction
const int32_t NumDir[13] = {0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8};

static inline void add_quiet_move(const board_t* pos, int move, move_list_t* p_list) 
{
    ASSERT(sq_on_board(FROM_SQ(move)));
    ASSERT(sq_on_board(TO_SQ(move)));

    p_list->moves[p_list->count].move = move;

    if(pos->search_killers[0][pos->ply] == move)
        p_list->moves[p_list->count].score = PriorityLevel[2];

    else if(pos->search_killers[1][pos->ply] == move)
        p_list->moves[p_list->count].score = PriorityLevel[3];
    
    else 
         p_list->moves[p_list->count].score = pos->search_history[pos->board[FROM_SQ(move)]][TO_SQ(move)];

    p_list->count++;
}

static inline void add_capture_move(const board_t* pos, int move, move_list_t* p_list)
{
    ASSERT(sq_on_board(FROM_SQ(move)));
    ASSERT(sq_on_board(TO_SQ(move)));

    p_list->moves[p_list->count].move = move;
    p_list->moves[p_list->count++].score = MvvLvaScores[CAPTURE(move)][pos->board[FROM_SQ(move)]] + PriorityLevel[1];
}

static inline void add_enpassant_move(const board_t* pos, int move, move_list_t* p_list)
{
    ASSERT(sq_on_board(FROM_SQ(move)));
    ASSERT(sq_on_board(TO_SQ(move)));
    ASSERT((RanksBoard[TO_SQ(move)]==RANK_6 && pos->side == WHITE) || (RanksBoard[TO_SQ(move)]==RANK_3 && pos->side == BLACK));

    p_list->moves[p_list->count].move = move;
    p_list->moves[p_list->count++].score = MvvLvaScores[wP][bP] + PriorityLevel[1];
}

/* Pawn Moves
    1) While we have pawn start, pawn can move one or two move ahead in vertical direction
    2) Pawn can capture, if opponent piece is present diagonally
    3) Pawn can promote if present on 2nd rank if side is black and on 7th rank if side is white
*/
static inline void add_white_pawn_capture_move(const board_t* pos, const int32_t from, const int32_t to, const int32_t cap, move_list_t* p_list)
{
    // As here, we are mentioning 7th rank, that means we are talking about white side
    // Either pawn piece can be promoted by capturing or simply capture piece in diagonal direction

    ASSERT(is_piece_valid(cap));
    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));

    if(RanksBoard[from] == RANK_7)
    {
        add_capture_move(pos, MOVE(from, to, cap, wQ, 0), p_list);
        add_capture_move(pos, MOVE(from, to, cap, wR, 0), p_list);
        add_capture_move(pos, MOVE(from, to, cap, wB, 0), p_list);
        add_capture_move(pos, MOVE(from, to, cap, wN, 0), p_list);
    }
    else 
        add_capture_move(pos, MOVE(from, to, cap, EMPTY, 0), p_list);
}

static inline void add_black_pawn_capture(const board_t* pos, const int32_t from, const int32_t to, const int32_t cap, move_list_t* p_list)
{
    // As here, we are mentioning 2nd rank, that means we are taking about black side
    // Either pawn is capturing and got promoted or just capturing
    ASSERT(is_piece_valid(cap));
    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));

    if(RanksBoard[from] == RANK_2)
    {
        add_capture_move(pos, MOVE(from, to, cap, bQ, 0), p_list);
        add_capture_move(pos, MOVE(from, to, cap, bR, 0), p_list);
        add_capture_move(pos, MOVE(from, to, cap, bB, 0), p_list);
        add_capture_move(pos, MOVE(from, to, cap, bN, 0), p_list);
    }
    else 
        add_capture_move(pos, MOVE(from, to, cap, EMPTY, 0), p_list);
}

static inline void add_white_pawn_quiet_move(const board_t* pos, const int32_t from, const int32_t to, move_list_t* p_list)
{
    // Either pawn can be promoted quietly or simply move 'from sq' to 'to sq'
    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));

    if (RanksBoard[from] == RANK_7)
    {
       
        add_quiet_move(pos, MOVE(from, to, EMPTY, wQ, 0), p_list);
        add_quiet_move(pos, MOVE(from, to, EMPTY, wR, 0), p_list);
        add_quiet_move(pos, MOVE(from, to, EMPTY, wB, 0), p_list);
        add_quiet_move(pos, MOVE(from, to, EMPTY, wN, 0), p_list);
    }
    else
        add_quiet_move(pos, MOVE(from, to, EMPTY, EMPTY, 0), p_list);
}

static inline void add_black_pawn_quiet_move(const board_t* pos, const int32_t from, const int32_t to, move_list_t* p_list)
{
    //Either pawn can be promoted with capture or just move from 'from sq' to 'to sq'
    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));

    if (RanksBoard[from] == RANK_2)
    {
        add_quiet_move(pos, MOVE(from, to, EMPTY, bQ, 0), p_list);
        add_quiet_move(pos, MOVE(from, to, EMPTY, bR, 0), p_list);
        add_quiet_move(pos, MOVE(from, to, EMPTY, bN, 0), p_list);
        add_quiet_move(pos, MOVE(from, to, EMPTY, bB, 0), p_list);
    }
    else
        add_quiet_move(pos, MOVE(from, to, EMPTY, EMPTY, 0), p_list);
}

extern void generate_all_moves(const board_t* pos, move_list_t* p_list)
{
    //Checking whether or not position of pieces on board is intact with other entities
    ASSERT(check_board(pos));

    p_list->count = 0;
    int32_t piece = EMPTY, side = pos->side, sq = 0, t_sq = 0, t_piece_num = 0;

    // Generating moves for pawn
    // 1.) Go through all position of pawns(piece_num) of a given side using piece list
    // 2.) Check for pawn start move, if present then add in move list
    // 3.) Check for pawn capture move that will be either promoted or just moving from 'from sq' to 'to sq' by capturing piece
    // and adding that to move list.
    // 4.) Check for enpassant move capture, if present will be added to move list by setting up the enpassant flag

    if(side == WHITE)
    {
        for(t_piece_num = 0; t_piece_num < pos->piece_num[wP]; ++t_piece_num)
        {
            sq = pos->piece_list[wP][t_piece_num];
            ASSERT(sq_on_board(sq));

            //Pawn start moves
            if(pos->board[sq + 10] == EMPTY)
            {
                add_white_pawn_quiet_move(pos, sq, sq + 10, p_list);
                if(RanksBoard[sq] == RANK_2 && pos->board[sq + 20] == EMPTY)
                    add_quiet_move(pos, MOVE(sq, (sq + 20), EMPTY, EMPTY, MV_FLAG_PS), p_list);
            }

            //Pawn normal capture moves
            if(sq_on_board(sq + 9) && COLOR_OF(pos->board[sq + 9]) == BLACK)
                add_white_pawn_capture_move(pos, sq, sq + 9, pos->board[sq + 9], p_list);

            if(sq_on_board(sq + 11) && COLOR_OF(pos->board[sq + 11]) == BLACK)
                add_white_pawn_capture_move(pos, sq, sq + 11, pos->board[sq + 11], p_list);

            //Note : sq + 9, can be offboard so we need to check whether or not, sq is offboard

            //Pawn enpassant capture moves
            if(pos->enpassant_sq != NO_SQ)
            {
                if((sq + 9) == pos->enpassant_sq)
                    add_enpassant_move(pos, MOVE(sq, (sq + 9), EMPTY, EMPTY, MV_FLAG_EP), p_list);
                
                if((sq + 11) == pos->enpassant_sq)
                    add_enpassant_move(pos, MOVE(sq, (sq + 11), EMPTY, EMPTY, MV_FLAG_EP), p_list);
            }
        }
        
        //Checking whether condition for king side castling meets or not for white side to add castling move
        if(pos->castle_perm & wKCA)
        {
            if(pos->board[F1] == EMPTY && pos->board[G1] == EMPTY)
            {
                if(!is_square_attacked(pos, BLACK, E1) && !is_square_attacked(pos, BLACK, F1))
                    add_quiet_move(pos, MOVE(E1, G1, EMPTY, EMPTY, MV_FLAG_CA), p_list);
            }
        }

        //Checking whether condition for queen side castling meets or not for white side to add castling move
        if(pos->castle_perm & wQCA)
        {
            if(pos->board[B1] == EMPTY && pos->board[C1] == EMPTY && pos->board[D1] == EMPTY)
            {
                if(!is_square_attacked(pos, BLACK, E1) && !is_square_attacked(pos, BLACK, D1))
                    add_quiet_move(pos, MOVE(E1, C1, EMPTY, EMPTY, MV_FLAG_CA), p_list);
            }
        }
    }
    //Note : Here, we have left checking whether C1 and G1 square is attacked by black that we will check after making move
    else //Side to play is black
    {
        for(t_piece_num = 0; t_piece_num < pos->piece_num[bP]; t_piece_num++)
        {
            sq = pos->piece_list[bP][t_piece_num];
            ASSERT(sq_on_board(sq));

            //Black Pawn start moves
            if(pos->board[sq - 10] == EMPTY)
            {
                add_black_pawn_quiet_move(pos, sq, (sq - 10), p_list);
                if(RanksBoard[sq] == RANK_7 && pos->board[sq - 20] == EMPTY)
                    add_quiet_move(pos, MOVE(sq, (sq - 20), EMPTY, EMPTY, MV_FLAG_PS), p_list);
            }

            //Black Pawn capture moves
            if(sq_on_board(sq - 9) && COLOR_OF(pos->board[sq - 9]) == WHITE)
                add_black_pawn_capture(pos, sq, (sq - 9), pos->board[sq - 9], p_list);

            if(sq_on_board(sq - 11) && COLOR_OF(pos->board[sq - 11]) == WHITE)
                 add_black_pawn_capture(pos, sq, (sq - 11), pos->board[sq - 11], p_list);
               

            if(pos->enpassant_sq != NO_SQ)
            {
                if((sq - 9) == pos->enpassant_sq)
                    add_enpassant_move(pos, MOVE(sq, (sq - 9), EMPTY, EMPTY, MV_FLAG_EP), p_list);
                
                if((sq - 11) == pos->enpassant_sq)
                    add_enpassant_move(pos, MOVE(sq, (sq - 11), EMPTY, EMPTY, MV_FLAG_EP), p_list);
            }
        } //Note : Here, we have left checking whether C8 and G8 square is attacked by white that we will checking after making move

        //Checking or not, king side castling condition meets for black side to add castling move
        if(pos->castle_perm & bKCA)
        {
            if(pos->board[F8] == EMPTY && pos->board[G8] == EMPTY)
            {
                if(!is_square_attacked(pos, WHITE, E8) && !is_square_attacked(pos, WHITE, F8))
                        add_quiet_move(pos, MOVE(E8, G8, EMPTY, EMPTY, MV_FLAG_CA), p_list);
                }
        }

        //Checking whether or not, queen side castling meets for black side to add castling move
        if(pos->castle_perm & bQCA)
        {
            if(pos->board[B8] == EMPTY && pos->board[C8] == EMPTY && pos->board[D8] == EMPTY)
            {
                if(!is_square_attacked(pos, WHITE, E8) && !is_square_attacked(pos, WHITE, D8))
                    add_quiet_move(pos, MOVE(E8, C8, EMPTY, EMPTY, MV_FLAG_CA), p_list);
            }
        }
    }

    /* MOVE GENERATION FOR SLIDING PIECES
        1) for each sliding piece of particular type present in sliding array i.e either for black or white
        2) for each piece of particular type on board
            for each direction of piece, slide piece on board and add that move, after going in that direction
            to movelist untill you hit by piece in order to capture or to stop sliding square in which sliding 
            piece can go till the offboard

    Note 1) Here, we are not going to use conditional statement for side, as code is similar for black and white
    Note 2) In piece slide array, non-slide pieces and sliding pieces are disjoint

    */
   int32_t dir = 0, idx = 0, piece_idx = 0;

   piece_idx = SlideIndex[side]; //Depending on side, we get slide index
   piece = SlidePiece[piece_idx];

   while(piece != EMPTY)
   {
        ASSERT(is_piece_valid(piece));

        for(t_piece_num = 0; t_piece_num < pos->piece_num[piece]; t_piece_num++)
        {
            sq = pos->piece_list[piece][t_piece_num];
            ASSERT(sq_on_board(sq));

            for(idx = 0; idx < NumDir[piece]; ++idx)
            {
                dir = PieceDir[piece][idx];
                t_sq = sq + dir;

                while(sq_on_board(t_sq))
                {
                    if(pos->board[t_sq] != EMPTY)
                    {
                        if(COLOR_OF(pos->board[t_sq]) == OPPONENT(side))
                              add_capture_move(pos, MOVE(sq, t_sq, pos->board[t_sq], EMPTY, 0), p_list);
                        break;
                    }
                    add_quiet_move(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), p_list);
                    t_sq += dir;
                }
            }
        }
        piece = SlidePiece[++piece_idx];
   }

   /* MOVE GENERATION FOR NON-SLIDING PIECES 
      1) for each non-sliding piece of particular type present in non-sliding array i.e of either white or black
      2) for each piece of particular type on board
            for each direction of piece on board, add move, after going in that direction to move list untill you 
            hit by piece in order to capture or to stop discovering in direction where we go offboard
   */

    piece_idx = nonSlideIndex[side];
    piece = nonSlidePiece[piece_idx];

    while(piece != EMPTY)
    {
        ASSERT(is_piece_valid(piece));

        for(t_piece_num = 0; t_piece_num < pos->piece_num[piece]; t_piece_num++)
        {
            sq = pos->piece_list[piece][t_piece_num];
            ASSERT(sq_on_board(sq));
          
            for(idx = 0; idx < NumDir[piece]; idx++)
            {
                dir = PieceDir[piece][idx];
                t_sq = sq + dir;

                if(!sq_on_board(t_sq))
                    continue;

                if(pos->board[t_sq] != EMPTY)
                {
                    if(COLOR_OF(pos->board[t_sq]) == OPPONENT(side))
                        add_capture_move(pos, MOVE(sq, t_sq, pos->board[t_sq], EMPTY, 0), p_list);
                    
                    continue;
                }  
                add_quiet_move(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), p_list);
            }
        }
        piece = nonSlidePiece[++piece_idx];
    }
    ASSERT(check_move_list(pos, p_list));
}

extern void generate_all_capture_moves(const board_t* pos, move_list_t* p_list)
{
    move_list_t list;
    generate_all_moves(pos, &list);

    p_list->count = 0;
    for(int move_num = 0; move_num < list.count; move_num++)
    {
        int32_t move = list.moves[move_num].move;
        if(move & MV_FLAG_CP)
        {
           // printf("move : %s\n", print_move(move));
            add_capture_move(pos, move, p_list);
        }
    }
    //printf("count : %d\n", p_list->count);
}

extern bool check_move_list(const board_t* pos, const move_list_t* p_list)
{
    if(p_list->count < 0 || p_list->count >= MAX_GAME_MOVES)
        return false;

    int32_t from = NO_SQ, to = NO_SQ;
    for(int move_num = 0; move_num < p_list->count; move_num++)
    {
        int32_t move = p_list->moves[move_num].move;
        to =   TO_SQ(move);
        from = FROM_SQ(move);

        if(!sq_on_board(to) || !sq_on_board(from))
            return false;
        
        if(is_piece_valid(pos->board[from]) == false)
            return false;
    }

    return true;
}

extern bool move_exists(board_t* pos, int32_t move)
{
    move_list_t list;
    generate_all_moves(pos, &list);

    for(int move_num = 0; move_num < list.count; move_num++)
    {
        if(!make_move(pos, move))
            continue;

        undo_move(pos);

        if(list.moves[move_num].move == move)
            return true;
    }
    return false;
}