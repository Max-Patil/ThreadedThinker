/*
    MakeMove - This module is used to simply manage the all state of chess board that include updating 
    all variables that is used for playing chess
*/

#include "defs.h"

#define HASH_PIECE(piece, sq) (pos->pos_key ^= PiecePositionKeys[piece][sq])
#define HASH_CA               (pos->pos_key ^= CastleKeys[pos->castle_perm])
#define HASH_SIDE             (pos->pos_key ^= SideKey)
#define HASH_EP               (pos->pos_key ^= PiecePositionKeys[EMPTY][pos->enpassant_sq])

const int castlePermission[BOARD_SQUARES] = {
    // White side
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 

    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 

    // Black side
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15,  7, 15, 15, 15, 3, 15, 15, 11, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 
};

/*
    clear_piece(from) :
    1) Hash-out old piece position key
    2) Set the position(sq) to empty
    3) Update piece counters and score counter
    4) If piece is present at given square is pawn
    then update pawn bitboard by clearning
    5) clear piece from piece list

    add_piece(to, piece) :
    1) Hash in new piece position key
    2) Set the position of given piece to 'to sq'
    3) Update piece counters and score counter
    4) If piece is present at given square is pawn
    then update pawn bitboard by setting bits
    5) Add piece from piece list

    move_piece(to, from, piece) :
    1) Hash out old position of piece i.e hash(piece, from)
    and hash-in new position of piece i.e hash(piece, to)

    2) Update board position by setting position of piece to
    'to sq' and resetting position of piece from 'from sq' to 
    empty

    3) Updating bit board position if piece present at position
    (from) is pawn piece

    4) Updating piece list by modifying the position(sq) of piece
    piece from 'from sq' to 'to sq'

    Note : Only difference between move_piece and (add_piece, clear_piece)
    is that we are updating piece counters & scores in add piece and clear piece
*/

static inline void add_piece(board_t* pos, const int32_t piece, const int32_t to_sq)
{
    ASSERT(sq_on_board(to_sq));
    ASSERT(is_piece_valid(piece));

    int32_t color = COLOR_OF(piece), idx = 0, t_piece_num = INVALID;

    //1.) Hash in position key
    HASH_PIECE(piece, to_sq);

    //2.) Setting position of a given square to a given piece
    pos->board[to_sq] = piece;

    //3.) Update material score & counters appropriately
    pos->material_score[color] += VALUE_OF(piece);

    if(IS_BIG(piece))
    {
        pos->big_pieces[color]++;

        if(IS_MAJOR(piece))
            pos->major_pieces[color]++;
        else
            pos->minor_pieces[color]++;
    }
    //4.) Setting pawn bit board position appropriately
    else
    {
        SET_BIT(pos->pawns[color], SQ64(to_sq));
        SET_BIT(pos->pawns[BOTH],  SQ64(to_sq));
    }

    //5.) Adding piece to piece list and incrementing piece num
    pos->piece_list[piece][pos->piece_num[piece]++] = to_sq;
}

static inline void clear_piece(board_t* pos, const int32_t from_sq)
{

    ASSERT(sq_on_board(from_sq));
    ASSERT(check_board(pos));

    int32_t piece = pos->board[from_sq];
    ASSERT(is_piece_valid(piece));

    int32_t color = COLOR_OF(piece), idx = 0, t_piece_num = INVALID;

    //1.) Hash out position key
    HASH_PIECE(piece, from_sq);

    //2.) Setting position of a given square to empty
    pos->board[from_sq] = EMPTY;

    //3.) Update material score & counters appropriately
    pos->material_score[color] -= VALUE_OF(piece);

    if(IS_BIG(piece))
    {
        pos->big_pieces[color]--;

        if(IS_MAJOR(piece))
        {
            pos->major_pieces[color]--;
        }
        else
            pos->minor_pieces[color]--;
    }
    //4.) Clearing pawn from bitboard appropriately
    else
    {
        CLEAR_BIT(pos->pawns[color], SQ64(from_sq));
        CLEAR_BIT(pos->pawns[BOTH],  SQ64(from_sq));
    }

    //5.) Finally, update piece list
    for(idx = 0; idx < pos->piece_num[piece]; idx++)
    {
        if(pos->piece_list[piece][idx] == from_sq)
        {
            t_piece_num = idx;
            break;
        }
    }

    //Assertion is done to make sure square is valid and given piece is present in piece list at that position
    ASSERT(t_piece_num != INVALID);
    ASSERT(t_piece_num >= 0 && t_piece_num < 10);

    pos->piece_num[piece]--;
    pos->piece_list[piece][t_piece_num] = pos->piece_list[piece][pos->piece_num[piece]];
    // Above, we are simply, overriding invalid position value with last position value of that piece
}

static inline void move_piece(board_t* pos, const int32_t from, const int32_t to)
{
    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));

    int32_t color = COLOR_OF(pos->board[from]), piece = pos->board[from], idx = 0;

    #ifdef DEBUG
        int32_t t_piece_num = false;
    #endif

    //1.) Updating position of piece from 'from sq' to 'to sq' along with that update position key
    HASH_PIECE(piece, from);
    pos->board[from] = EMPTY;

    HASH_PIECE(piece, to);
    pos->board[to] = piece;

    //2.) Updating pawn position on board appropriately
    if(IS_PAWN(piece))
    {
        CLEAR_BIT(pos->pawns[color], SQ64(from));
        CLEAR_BIT(pos->pawns[BOTH],  SQ64(from));
        SET_BIT(pos->pawns[color], SQ64(to));
        SET_BIT(pos->pawns[BOTH],  SQ64(to));
    }

    //3.) Updating piece list only
    for(idx = 0; idx < pos->piece_num[piece]; idx++)
    {
        if(pos->piece_list[piece][idx] == from)
        {
            pos->piece_list[piece][idx] = to;
            
            #ifdef DEBUG
                t_piece_num = true;
            #endif

            break;
        }
    }

    ASSERT(t_piece_num);
}

/* MakeMove(move) :
    while making a move, transition happens from state A to state B, but we also need to 
    check whether transition is valid or not using check board and after making a move whether
    king is in check or not

    Note : state of chess board is denoted by position key (hash value)

    * During transition i.e while making a move following entities are affected
    1) Position key
    2) Piece counters and score counters
    3) Piece and position tracking data structures ( pawn bit board and piece list )
    4) Chess board history saving state of game play till now
    5) Side
    6) Fifty move counter
    7) History ply and ply 
    8) Castle permission
    9) Enpassant Square (is set only on pawn start move)

    Depending on following move type - position key, fifty move counter, enpassant square and castle permission
    will change

    QUIET MOVES
    MoveType(1) -> Castling move(MV_FLAG_CA) 
    MoveType(2) -> Pawn Promotion move only (MV_FLAG_PROM) 
    MoveType(3) -> Pawn Start Move(MV_FLAG_PS) -> Set Enpassant Square
    MoveType(4) -> Normal Move 

    CAPTURE MOVES
    MoveType(1) -> Enpassant move(MV_FLAG_EP) 
    MoveType(2) -> Capture move only( MV_FLAG_CA )

    PSEUDO LEGAL MOVE
    MoveType(1) -> Moving piece even though king Square is attacked i.e king is in check

    Note 1) Enpassant move is capture move therefore clear piece and move piece function is used
    Note 2) Handling pawn promotion move as quiet move and then handling capture move (MV_FLAG_PROM & MV_FLAG_CA)

    Entities that are affected by move and most important part of states
    1) Fifty moves ( Quiet Move(MoveType(2), MoveType(3) ) and All Capture moves ) excluding Normal moves & castling move
    2) Enpassant square ( Quiet Move(MoveType(3)) and Capture Move(MoveType(1)) )
    3) Castle Permission ( Quiet Move(MoveType(1)) )
    4) Position key -> ( Any Move + Side + Enpassant + Castle )

    Note 1) we need to hash-out (old state) & hash-in (new state) after moving piece
    Note 2) Position key depends on side key, castle key, piece position key and enpassant square key if it is set

    Making move involve following things
    1) Extract All information from move that is encoded in binary format
    
    2) Save all current information( move, fifty move, enpassant, castle permission & pos key) in history array so 
    that later it can retrieved while undoing move
    Note : Increment history ply, ply and fifty move counter 

    3) Handle castle move, by moving rook to appropriate square when castling is done
    Note : Hash-out and hash-in old and new state of hashkeys with respect to castle permission respectively, after changing
    castle permission

    4) Handling enpassant move, by capturing opponents's pawn i.e to clear opponent's pawn piece
    Note : If enpassant square is set, then hash-out old state of hashkey from position key and set
    enpassant square to NO SQUARE

    5) A pawn start move, trigger enpassant square to set either on RANK 3 or RANK 6 depending on side
    Note : After setting enpassant square, hash-in new state i.e hashkeys of enpassant square and also
    set fifty move counter to 0

    6) Handle capture, if piece is captured, then clear the piece and set fifty move counter to 0

    7) Handle quiet moves by simply moving piece from source square to target square

    8) As we have move piece from source to target square, to handle pawn promotion, we need to clear target square
    and add piece to target square

    9) Change or flip the side and hash-in new state by hashing out the old state of side hashkey

    10) Set king square to appropriate location 

    11) Finally, after making a move, check whether king square is attacked by opponent's pieces, if it is
    attacked then return false and undo move because it is pseudo legal move

*/

/*
    Below function return false after making move, side that has made the move is left themselves in check and 
    that is pseudo legal move but if king is in check then we need to get him, out of check with possible moves
    that resolve that check and if we don't have any move to get him out of check, then it is a checkmate 
*/
extern bool make_move(board_t* pos, const int32_t move)
{

    //1.) Get the from, to, side, capture and promoted from the move structure
    int32_t from = FROM_SQ(move);
    int32_t to   = TO_SQ(move);
    int32_t side = pos->side;
    int32_t promoted = PROMOTED(move);
    int32_t capture =  CAPTURE(move);



    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));
    ASSERT(is_side_valid(side));
    ASSERT(is_piece_valid(pos->board[from]));
    ASSERT(check_board(pos));


    ASSERT(pos->hist_ply >= 0 && pos->hist_ply < MAX_GAME_MOVES);
    ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);

    //2.) Store the current position in the history array
    pos->board_history[pos->hist_ply].pos_key = pos->pos_key;
    pos->board_history[pos->hist_ply].move = move;
    pos->board_history[pos->hist_ply].fifty_move = pos->fifty_move;
    pos->board_history[pos->hist_ply].enpassant = pos->enpassant_sq;
    pos->board_history[pos->hist_ply].castle_perm = pos->castle_perm;

    //Incrementing ply, histply and fifty move counter
    pos->ply++;
    pos->hist_ply++;
    pos->fifty_move++;

    //3.) Move the current piece from sq -> to sq
   
    //Quiet Move -> Castling move
    if(move & MV_FLAG_CA)
    { 
        switch(to)
        {
            //Moving rook to appropriate position, if king need to move from E1 & E8 to C1, C8 and G1, G8
            case C1:
                move_piece(pos, A1, D1);
                break;

            case C8:
                move_piece(pos, A8, D8);
                break;

            case G1:
                move_piece(pos, H1, F1);
                break;
            
            case G8:
                move_piece(pos, H8, F8);
                break;

            default: //If castling flag is set, as move is castling move, then 'to square' should be C1, C8, G1, G8 other wise it is invalid
                ASSERT(false);
                break;
        }
    }

    //We hash-out current state of castling permission
    HASH_CA;

    pos->castle_perm &= castlePermission[from];
    pos->castle_perm &= castlePermission[to];

    //We hash-in new state of castling permission
    HASH_CA;

    // Capture Move -> Enpassant move
    if(move & MV_FLAG_EP)
    {
        pos->fifty_move = 0;
        if(side == WHITE)
            clear_piece(pos, to - 10);
        else
            clear_piece(pos, to + 10);
    }

    if(pos->enpassant_sq != NO_SQ)
        HASH_EP; //Hash-out current state of enpassant square

    pos->enpassant_sq = NO_SQ;

    // Quiet Move -> Pawn start move
    if(IS_PAWN(pos->board[from]))
    {
        pos->fifty_move = 0;

        if(move & MV_FLAG_PS)
        {
            // For pawn start move, enpassant square will be set appropriately
            if(side == WHITE)
            {
                pos->enpassant_sq = from + 10;
                ASSERT(RanksBoard[pos->enpassant_sq] == RANK_3);
            }
            else
            {
                pos->enpassant_sq = from - 10;
                ASSERT(RanksBoard[pos->enpassant_sq] == RANK_6);
            }

            HASH_EP; // Hash-in new state of enpassant square
        }
    }

    // Capture Move -> Capture move only
    if(capture != EMPTY)
    {
        ASSERT(is_piece_valid(capture));
        pos->fifty_move = 0;
        clear_piece(pos, to);
    }

    // Quiet Move -> Normal move(king move, pawn start move and enpassant move)
    move_piece(pos, from, to);

    // Quiet Move -> Pawn Promotion move
    if(promoted != EMPTY)
    {
        //Pawn promotion will update material score even though it is quiet move
        ASSERT(is_piece_valid(promoted) && !IS_PAWN(promoted));
        pos->fifty_move = 0;
        clear_piece(pos, to);
        add_piece(pos, promoted, to);
    }

    pos->side = OPPONENT(side);
    HASH_SIDE; // Hash-out state of current side to hash-in state of opponent side

    //Updating king position if king captures or moved
    if(IS_KING(pos->board[to]))
        pos->king_sq[side] = to;

    ASSERT(check_board(pos));

    // Dealing with pseudo legal move
    if(is_square_attacked(pos, pos->side, pos->king_sq[side]))
    {
        undo_move(pos);
        return false;
    }

    return true;
}

extern void undo_move(board_t* pos)
{
    // Decrement history ply and ply
    pos->hist_ply--;
    pos->ply--;

    // Get the from, to, side, capture and promoted from the move
    int32_t move = pos->board_history[pos->hist_ply].move;
    int32_t from = FROM_SQ(move);
    int32_t to   = TO_SQ(move);
    int32_t capture = CAPTURE(move);
    int32_t promoted = PROMOTED(move);

    ASSERT(check_board(pos));
    ASSERT(sq_on_board(from));
    ASSERT(sq_on_board(to));

    if(pos->enpassant_sq != NO_SQ)
        HASH_EP; // Hash-out current state of enpassant square

    HASH_CA; // Hash out current state of castle permission

    pos->castle_perm = pos->board_history[pos->hist_ply].castle_perm;
    pos->fifty_move  = pos->board_history[pos->hist_ply].fifty_move;
    pos->enpassant_sq = pos->board_history[pos->hist_ply].enpassant;

    if(pos->enpassant_sq != NO_SQ)
        HASH_EP; // Hash in new state of enpassant i.e retrieved from board history

    HASH_CA; // Hash in new state of castle permission i.e retrieved from board history

    pos->side = OPPONENT(pos->side);
    HASH_SIDE; // Hash-in new state by hashing out current state of side

    // Reversing enpassant capture move by adding opponent's pawn piece
    if(MV_FLAG_EP & move)
    {
        if(pos->side == WHITE)
            add_piece(pos, bP, to - 10);
        else
            add_piece(pos, wP, to + 10);
    }

    // Reversing castle move, by moving rook back to its position
    if (MV_FLAG_CA & move)
    {
        switch(to)
        {
            case C1:
                move_piece(pos, D1, A1);
                break;
            case C8:
                move_piece(pos, D8, A8);
                break;
            case G1:
                move_piece(pos, F1, H1);
                break;
            case G8:
                move_piece(pos, F8, H8);
                break;
            default:
                ASSERT(false);
                break;
        }
    }

    // Reversing king position and pawn position
    move_piece(pos, to, from);

    if(IS_KING(pos->board[from]))
        pos->king_sq[pos->side] = from;
    
    if(capture != EMPTY)
    {
        ASSERT(is_piece_valid(capture));
        add_piece(pos, capture, to);
    }

    if(promoted != EMPTY)
    {
        ASSERT(is_piece_valid(promoted) && !IS_PAWN(promoted));
        clear_piece(pos, from);
        add_piece(pos, (COLOR_OF(promoted) == WHITE ? wP : bP), from);
    }

    ASSERT(check_board(pos));
}

extern void make_null_move(board_t* pos)
{
    //Checking whether everthing is correct
    ASSERT(check_board(pos));
    ASSERT(!is_square_attacked(pos, OPPONENT(pos->side), pos->king_sq[pos->side]));

    //Saving information to history array
    pos->board_history[pos->hist_ply].pos_key = pos->pos_key;
    pos->board_history[pos->hist_ply].fifty_move = pos->fifty_move;
    pos->board_history[pos->hist_ply].enpassant = pos->enpassant_sq;
    pos->board_history[pos->hist_ply].castle_perm = pos->castle_perm;
    pos->board_history[pos->hist_ply].move = NO_MOVE;

    //Incrementing history ply and ply
    pos->hist_ply++;
    pos->ply++;

    //Hash-out old state of enpassant if enpassant is set
    if(pos->enpassant_sq != NO_SQ)
        HASH_EP;

    pos->enpassant_sq = NO_SQ;

    //Hash-out old state of side 
    pos->side = OPPONENT(pos->side);
    HASH_SIDE;

    ASSERT(check_board(pos));
}

extern void undo_null_move(board_t* pos)
{
    ASSERT(check_board(pos));

    //Decrementing history ply and ply i.e undoing depth explored in current search
    pos->hist_ply--;
    pos->ply--;

    //Hash-out current state of enpassant if enpassant square is set
    if(pos->enpassant_sq != NO_SQ)
        HASH_EP;

    //Retrieving the stored information
    pos->castle_perm = pos->board_history[pos->hist_ply].castle_perm;
    pos->fifty_move  = pos->board_history[pos->hist_ply].fifty_move;
    pos->enpassant_sq = pos->board_history[pos->hist_ply].enpassant;

    //Hash-in previous saved state 
    if(pos->enpassant_sq != NO_SQ)
        HASH_EP;
    
    //Hash in state of previous position side hashkey
    pos->side = OPPONENT(pos->side);
    HASH_SIDE;

    ASSERT(check_board(pos));
}
