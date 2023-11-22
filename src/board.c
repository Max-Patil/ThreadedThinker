/*
    Board - This module is used to check integrity and manipulate state of actual chess board and its entities
*/

#include "defs.h"

extern void reset_board(board_t* pos)
{
   //initializing every member of board structure to 0
    memset(pos, 0, sizeof(board_t)); 
    
    for(int sq120 = 0; sq120 < BOARD_SQUARES; sq120++)
        pos->board[sq120] = OFFBOARD;

    for(int sq64 = 0; sq64 < SQUARE_TYPES; sq64++)
        pos->board[SQ120(sq64)] = EMPTY;

    pos->side = BOTH;
    pos->enpassant_sq = NO_SQ;
    pos->king_sq[WHITE] = pos->king_sq[BLACK] = NO_SQ;
    pos->pawns[WHITE] = pos->pawns[BLACK] = pos->pawns[BOTH] = 0ULL;

}

extern int parse_fen(board_t* pos, char *fen)
{
    ASSERT(pos != NULL && fen != NULL);

    int rank = RANK_8, file = FILE_A, piece = EMPTY, count = 0;
    int sq120 = 0, sq64 = 0, i = 0;

    reset_board(pos);

    while((rank >= RANK_1) && *fen)
    {
        count = 1; //For every valid piece, count will be 1

        switch(*fen)
        {
            //Fen notation for black pieces
            case 'p' : piece = bP; break;
            case 'r' : piece = bR; break;
            case 'b' : piece = bB; break;
            case 'n' : piece = bN; break;
            case 'q' : piece = bQ; break;
            case 'k' : piece = bK; break;

            //Fen notation for white pieces
            case 'P' : piece = wP; break;
            case 'R' : piece = wR; break;
            case 'N' : piece = wN; break;
            case 'B' : piece = wB; break;
            case 'Q' : piece = wQ; break;
            case 'K' : piece = wK; break;

            //Number of consecutive squares
            case '1' :
            case '2' :
            case '3' :
            case '4' :
            case '5' :
            case '6' :
            case '7' :
            case '8' :
                piece = EMPTY;
                count = *fen - '0';
                break;
            
            case '/':
            case ' ':
                rank = rank - 1;
                file = FILE_A;
                fen++;
                continue;
            
            default :
                printf("Invalid Fen Notation %s\n", fen);
                return INVALID;

        }

        for(i = 0; i < count; i++)
        {
            sq64 = rank * 8 + file;
            sq120 = SQ120(sq64);

            if(piece != EMPTY)
                pos->board[sq120] = piece;
            file++;
        }
        fen++;
    }

    // Setting Side 
    ASSERT(*fen == 'w' || *fen == 'b');
    pos->side = ((*fen == 'w') ? WHITE : BLACK); 
    fen += 2;

    // Setting Castling Right
    for(i = 0; i < 4; i++) 
    {
        if(*fen == ' ')
            break;
        
        switch(*fen) 
        {
            case 'K' : pos->castle_perm |= wKCA; break;
            case 'Q' : pos->castle_perm |= wQCA; break;
            case 'k' : pos->castle_perm |= bKCA; break;
            case 'q' : pos->castle_perm |= bQCA; break;
            //Note : we are simply ignorning '-' and move fen++
            default : break;
        }
        fen++;
    }
    
    fen++; //After incrementing fen, we move to enpassant square

    ASSERT(pos->castle_perm >= 0 && pos->castle_perm <= 15 );

    // Setting Enpassant Sq if avaliable
    if(*fen != '-')
    {
        file = *(fen) - 'a';
        rank = *(++fen) - '1';

        ASSERT(file >= FILE_A && file <= FILE_H);
        ASSERT(rank >= RANK_1 && rank <= RANK_8);

        pos->enpassant_sq = FROM_SQ64_TO_SQ120(file, rank);
    }

    pos->pos_key = generate_pos_key(pos);

    update_list_material(pos);

    return 0;
}

extern void print_board(const board_t* pos)
{
    int sq = 0, file = FILE_A, rank = RANK_8, piece = EMPTY;

    printf("\nGame Board\n  ");

    for(file = FILE_A; file <= FILE_H; file++)
        printf("%3c", 'a' + file);

    puts("\n");
    for(rank = RANK_8; rank >= RANK_1; rank--)
    {
        printf(" %d", rank + 1);

        for(file = FILE_A; file <= FILE_H; file++)
        {
            sq = FROM_SQ64_TO_SQ120(file, rank);
            piece = pos->board[sq];

            printf("%3c", PiecesChars[piece]);
        }
        printf("\n");
    }
 
    printf("\n  ");

    for(file = FILE_A; file <= FILE_H; file++)
        printf("%3c", 'a' + file);
    
    printf("\n\nside : %c\n", SideChars[pos->side]);
    printf("enpassant Sq : %d\n", pos->enpassant_sq);

    printf("castle : %c%c%c%c\n", pos->castle_perm & wKCA ? 'K' : '-',
                                  pos->castle_perm & wQCA ? 'Q' : '-',
                                  pos->castle_perm & bKCA ? 'k' : '-',
                                  pos->castle_perm & bQCA ? 'q' : '-');

    printf("poskey : %llX\n", pos->pos_key);
}

extern void update_list_material(board_t *pos)
{
    int piece = EMPTY, sq = 0, color = BOTH;

    for(sq = 0; sq < BOARD_SQUARES; sq++)
    {
        piece = pos->board[sq];

        if(piece != OFFBOARD && piece != EMPTY)
        {   
            color = COLOR_OF(piece);
            ASSERT(color == WHITE || color == BLACK);

            //Updating Big, Major and Minor Pieces
            if(IS_BIG(piece))
                pos->big_pieces[color]++;
            
            if(IS_MAJOR(piece))
                pos->major_pieces[color]++;

            if(IS_MINOR(piece))
                pos->minor_pieces[color]++;

            //Updating Material Score
            pos->material_score[color] += PieceValue[piece];

            //Updating Piece List
            pos->piece_list[piece][pos->piece_num[piece]++] = sq;

            //Updating King Square
            if(piece == wK || piece == bK)
                pos->king_sq[color] = sq;

            //Updating Pawn Bitboard
            if(IS_PAWN(piece))
            {
                SET_BIT(pos->pawns[color], SQ64(sq));
                SET_BIT(pos->pawns[BOTH],  SQ64(sq));
               
            }
        }
    }
}


extern int check_board(board_t* pos)
{
    
    /* Here, we have simply taken temporary variable or counters to simply update their value based 
    on current position of actual board. Later, after updation, these temporary counters will simply 
    be checked against actual counters/trackers of current position */

    int32_t t_piece_num[PIECE_TYPES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int32_t t_big_piece[AGENTS] = { 0,  0 };
    int32_t t_maj_piece[AGENTS] = { 0,  0 };
    int32_t t_min_piece[AGENTS] = { 0,  0 };
    int32_t t_material_score[AGENTS] = { 0, 0 };

    int sq64 = 0, t_piece = EMPTY, t_piece_count = 0, sq120 = 0, color = BOTH, pawn_count = 0;

    uint64_t t_pawns[3] = { 0ULL , 0ULL , 0ULL };

    t_pawns[WHITE]    = pos->pawns[WHITE];
    t_pawns[BLACK]    = pos->pawns[BLACK];
    t_pawns[BOTH]     = pos->pawns[BOTH];

    //Checking whether or not piece list is intact/aligned with actual chess board
    for(t_piece = wP; t_piece <= bK; t_piece++)
    {
        for(t_piece_count = 0; t_piece_count < pos->piece_num[t_piece]; t_piece_count++)
        {
            sq120 = pos->piece_list[t_piece][t_piece_count];
            ASSERT(pos->board[sq120] == t_piece);
        }
    }

    //Update temporary piece count numbers and material scores
    for(sq64 = 0; sq64 < SQUARE_TYPES; sq64++)
    {
        sq120 = SQ120(sq64);
        t_piece = pos->board[sq120];
        t_piece_num[t_piece]++;
        color = PieceColor[t_piece];

        if(IS_BIG(t_piece))
            t_big_piece[color]++;
        
        if(IS_MAJOR(t_piece))
            t_maj_piece[color]++;

        if(IS_MINOR(t_piece))
            t_min_piece[color]++;

        t_material_score[color] += PieceValue[t_piece];
    }

    //Check temporary counters with actual counters
    for(t_piece = wP; t_piece <= bK; t_piece++)
        ASSERT(t_piece_num[t_piece] == pos->piece_num[t_piece]);
    
    //Couting pawn and Checking their respective position on bit board for white, black & both
    pawn_count = COUNT_BITS(t_pawns[WHITE]);
    ASSERT(pawn_count == pos->piece_num[wP]);

    pawn_count = COUNT_BITS(t_pawns[BLACK]);
    ASSERT(pawn_count == pos->piece_num[bP]);

    pawn_count = COUNT_BITS(t_pawns[BOTH]);
    ASSERT(pawn_count == pos->piece_num[bP] + pos->piece_num[wP]);

    while(t_pawns[WHITE])
    {
        sq64 = POP_BIT(t_pawns[WHITE]);
        ASSERT(pos->board[SQ120(sq64)] == wP);
    }

    while(t_pawns[BLACK])
    {
        sq64 = POP_BIT(t_pawns[BLACK]);
        ASSERT(pos->board[SQ120(sq64)] == bP);
    }

    while(t_pawns[BOTH])
    {
        sq64 = POP_BIT(t_pawns[BOTH]);
        ASSERT(pos->board[SQ120(sq64)] == wP || pos->board[SQ120(sq64)] == bP);
    }

    //Checking updated temporary counters with actual counters
    ASSERT(t_material_score[WHITE] == pos->material_score[WHITE] && t_material_score[BLACK] == pos->material_score[BLACK]);
    ASSERT(t_maj_piece[WHITE] == pos->major_pieces[WHITE] && t_maj_piece[BLACK] == pos->major_pieces[BLACK]);
    // printf("piece_num[bB] : %d\n", pos->piece_num[bB]);

    // printf("t_min_piece[white] : %d and minor_piece[white] : %d\n", t_min_piece[WHITE], pos->minor_pieces[WHITE]);
    // printf("t_min_piece[black] : %d and minor_piece[black] : %d\n", t_min_piece[BLACK], pos->minor_pieces[BLACK]);
    ASSERT(t_min_piece[WHITE] == pos->minor_pieces[WHITE] && t_min_piece[BLACK] == pos->minor_pieces[BLACK]);
    ASSERT(t_big_piece[WHITE] == pos->big_pieces[WHITE]   && t_big_piece[BLACK] == pos->big_pieces[BLACK]);

    //Checking side
    ASSERT(pos->side == WHITE || pos->side == BLACK);

    //Checking whether or not same valid key is generated based on position of actual chess board 
    ASSERT(generate_pos_key(pos) == pos->pos_key);

    //Checking white and black king position on board are intact with respect to their position stored in king square
    ASSERT(pos->board[pos->king_sq[WHITE]] == wK && pos->board[pos->king_sq[BLACK]] == bK);

    /* Checking enpassant square, which either represents NoSquare or square at its appropriate rank depending on side
    If side to play is white then target enpassant should be on 6th rank else if side to play is black then enpassant 
    target enpassant should be 3rd rank */
    ASSERT(pos->enpassant_sq == NO_SQ || (RanksBoard[pos->enpassant_sq] == RANK_6 && pos->side == WHITE) || (RanksBoard[pos->enpassant_sq] == RANK_3 && pos->side == BLACK));

    ASSERT(pos->castle_perm >= 0 && pos->castle_perm <= 15);

    return true;
}

extern void mirror_board(board_t* pos) //->For Evaluation Debug
{
    int32_t BoardPieces[SQUARE_TYPES]  = { 0 };
    int32_t SwapPieces[PIECE_TYPES]    = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    int32_t t_side = OPPONENT(pos->side), t_castle_perm = 0, t_enpassant_sq = NO_SQ, t_piece = EMPTY;

    if(pos->castle_perm & wKCA)
        t_castle_perm |= bKCA;

    if(pos->castle_perm & wQCA)
        t_castle_perm |= bQCA;
    
    if(pos->castle_perm & bKCA)
        t_castle_perm |= wKCA;

    if(pos->castle_perm & bQCA)
        t_castle_perm |= wQCA;

    if(pos->enpassant_sq != NO_SQ)
        t_enpassant_sq = SQ120(MIRROR64(SQ64(pos->enpassant_sq)));

    for(int32_t sq = 0; sq < SQUARE_TYPES; sq++)
        BoardPieces[sq] = pos->board[SQ120(MIRROR64(sq))];

    reset_board(pos);

    for(int32_t sq = 0; sq < SQUARE_TYPES; sq++)
    {
        t_piece = SwapPieces[BoardPieces[sq]];
        pos->board[SQ120(sq)] = t_piece;
    }

    pos->side = t_side;
    pos->castle_perm = t_castle_perm;
    pos->enpassant_sq = t_enpassant_sq;
    pos->pos_key = generate_pos_key(pos);

    update_list_material(pos);

    ASSERT(check_board(pos));
}