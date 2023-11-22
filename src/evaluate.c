/*
    Evaluate - This module is simply used to evaluate by computing score given to the position that is based 
    on material and positional situtations
*/

#include "defs.h"


int32_t isolatedPawn = -10; // isolated pawn incentive;

                    //  RANK_1,  RANK_2  RANK_3,  RANK_4,  RANK_5, RANK_6  RANK_7,  RANK_8
int32_t passedPawn[8] = { 0,       5,      10,    20,      35,      60,     100,    200 }; // passed pawn incentive

int32_t rookOpenFile = 10;     // Open File rook incentive
int32_t rookSemiOpenFile = 5;  // Opponent pawn in rook attack file only
int32_t queenOpenFile = 5;     // No pawn in queen attack file
int32_t queenSemiOpenFile = 3; // No pawn of same side in queen attack file
int32_t bishopPair = 30;

//Switching to Endgame evaluation based on material score
#define ENDGAME_BEGIN (PieceValue[wR] + 2 * PieceValue[wN] + 2 * PieceValue[wP]) //( 400 + 400 + 200 ) => 1000 cp

// Piece Square Tables
const int32_t PawnTable[64] = {
    0, 0, 0, 0, 0, 0, 0, 0,         // RANK 1
    10, 10, 0, -10, -10, 0, 10, 10, // RANK 2
    5, 0, 0, 5, 5, 0, 0, 5,         // RANK 3
    0, 0, 10, 20, 20, 10, 0, 0,     // RANK 4
    5, 5, 5, 10, 10, 5, 5, 5,       // RANK 5
    10, 10, 10, 20, 20, 10, 10, 10, // RANK 6
    20, 20, 20, 30, 30, 20, 20, 20, // RANK 7
    0, 0, 0, 0, 0, 0, 0, 0,         // RANK 8
};

const int32_t knightTable[64] = {
    0, -10, 0, 0, 0, 0, -10, 0,
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 10, 10, 10, 10, 0, 0,
    0, 0, 10, 20, 20, 10, 5, 0,
    5, 10, 15, 20, 20, 15, 10, 5,
    5, 10, 10, 20, 20, 10, 10, 5,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

const int32_t bishopTable[64] = {
    0, 0, -10, 0, 0, -10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 0, 10, 15, 15, 10, 0, 0,
    0, 0, 0, 10, 10, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

const int32_t RookTable[64] = {
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    25, 25, 25, 25, 25, 25, 25, 25,
    0, 0, 5, 10, 10, 5, 0, 0};

const int32_t kingEndGameTable[64] = {
    -50, -10, 0, 0, 0, 0, -10, -50,
    -10, 0, 10, 10, 10, 10, 0, -10,
    0, 10, 15, 15, 15, 15, 10, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 10, 15, 20, 20, 15, 10, 0,
    0, 10, 15, 15, 15, 15, 10, 0,
    -10, 0, 10, 10, 10, 10, 0, -10,
    -50, -10, 0, 0, 0, 0, -10, -50

};

const int32_t kingWhileOpening[64] = {
    0, 5, 5, -10, -10, 0, 10, 5,
    -30, -30, -30, -30, -30, -30, -30, -30,
    -50, -50, -50, -50, -50, -50, -50, -50,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70,
    -70, -70, -70, -70, -70, -70, -70, -70
};

extern bool material_draw(const board_t *pos) //checkmate with best play by pieces on board
{
    // No queen and rook of either side
    if (pos->piece_num[wR] == 0 && pos->piece_num[wQ] == 0 && pos->piece_num[bQ] == 0 && pos->piece_num[bR] == 0)
    {
        // No bishop of either side
        if (pos->piece_num[bB] == 0 && pos->piece_num[wB] == 0)
        {
            // At least 2 knight of both side
            if (pos->piece_num[wN] < 3 && pos->piece_num[bN] < 3)
                return true;
        }
        // No Knight of either side
        else if (pos->piece_num[wN] == 0 && pos->piece_num[bN] == 0)
        {
            //Equal bishop material on both side
            if (abs(pos->piece_num[wB] - pos->piece_num[bB]) < 2)
                return true;
        }
        //At least two white knight with no bishop or only bishop with no knights
        else if((pos->piece_num[wN] < 3 && pos->piece_num[wB] == 0) || (pos->piece_num[wN] == 0 && pos->piece_num[wB] == 1))
        {
            //At least two black knight with no bishop or only bishop with no knights
            if((pos->piece_num[bN] < 3 && pos->piece_num[bB] == 0) || (pos->piece_num[bB] == 1 && pos->piece_num[bN] == 0))
                return true;
        }
    }//No Queen of either side
    else if(pos->piece_num[wQ] == 0 && pos->piece_num[bQ] == 0)
    {
        //Two rooks i.e one white rook and one black rook
        if(pos->piece_num[wR] == 1 && pos->piece_num[bR] == 1)
        {
            //At least one white bishop or knight and At least one black bishop or black knight
            if((pos->piece_num[wN] + pos->piece_num[wB] < 2 ) && (pos->piece_num[bN] + pos->piece_num[bB] < 2))
                return true;
        }
        //Only white Rook
        else if(pos->piece_num[wR] == 1 && pos->piece_num[bR] == 0)
        {
            //No bishops and knight of white side "and" ( At least one black bishop or black knight "or" At least two bishop or two knight or one bishop and knight of black side )
            if((pos->piece_num[wN] + pos->piece_num[wB] == 0) && (((pos->piece_num[bB] + pos->piece_num[bN] == 1) || (pos->piece_num[bN] + pos->piece_num[bB] == 2))))
                return true;
        }
        //Only black Rook
        else if(pos->piece_num[wR] == 0 && pos->piece_num[bR] == 1)
        {
            //No white bishop and knights "and" ( At least one white bishop or white knight "or" At least two bishop or two knight or one bishop and knight of white side ) 
            if((pos->piece_num[bN] + pos->piece_num[bB] == 0) && ((pos->piece_num[wN] + pos->piece_num[wB] == 1) || (pos->piece_num[wN] + pos->piece_num[wB] == 2)))
                return true;
        }

    }
    return false;
}

extern int32_t evaluate(const board_t *pos)
{

    int32_t piece = 0, piece_num = 0, sq = 0;

    // Material Evaluation
    int32_t score = pos->material_score[WHITE] - pos->material_score[BLACK];

    if (pos->piece_num[wP] == 0 && pos->piece_num[bP] == 0 && material_draw(pos) == true)
        return DRAW;

    // Position Evaluation

   // Pawn Piece Square Score evaluation
    piece = wP;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        ASSERT(sq_on_board(sq));
        score += PawnTable[SQ64(sq)];

        // Isolated Pawn Evaluation
        if ((IsolatedBitMask[SQ64(sq)] & pos->pawns[WHITE]) == 0)
             score += isolatedPawn;

        // Passed Pawn Evaluation
        if ((WhitePassedBitMask[SQ64(sq)] & pos->pawns[BLACK]) == 0)
             score += passedPawn[RanksBoard[sq]];        
    }

    piece = bP;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        ASSERT(sq_on_board(sq));
        score -= PawnTable[MIRROR64(SQ64(sq))];

        // Isolated Pawn Evaluation
        if ((IsolatedBitMask[SQ64(sq)] & pos->pawns[BLACK]) == 0)
               score -= isolatedPawn;
         
        // Passed Pawn Evaluation
        if ((BlackPassedBitMask[SQ64(sq)] & pos->pawns[WHITE]) == 0)
             score -= passedPawn[RANK_8 - RanksBoard[sq]];
    }

    // Knight Piece Square Score evaluation
    piece = wN;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        ASSERT(sq_on_board(sq));
        score += knightTable[SQ64(sq)];
    }

    piece = bN;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        ASSERT(sq_on_board(sq));
        score -= knightTable[MIRROR64(SQ64(sq))];
    }

    // Bishop Piece Square Score evaluation
    piece = wB;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        ASSERT(sq_on_board(sq));
        score += bishopTable[SQ64(sq)];
    }

    piece = bB;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        ASSERT(sq_on_board(sq));
        score -= bishopTable[MIRROR64(SQ64(sq))];
    }

    // Rook Piece Square Score evaluation
    piece = wR;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        score += RookTable[SQ64(sq)];

        // Open File Evaluation
        if ((pos->pawns[BOTH] & FilesBitMask[FilesBoard[sq]]) == 0)
            score += rookOpenFile;

        // Semi-Open File Evaluation
        if ((pos->pawns[WHITE] & FilesBitMask[FilesBoard[sq]]) == 0)
            score += rookSemiOpenFile;
    }

    piece = bR;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];
        score -= RookTable[MIRROR64(SQ64(sq))];

        // Open File Evaluation
        if ((pos->pawns[BOTH] & FilesBitMask[FilesBoard[sq]]) == 0)
            score -= rookOpenFile;

        // Semi-Open File Evaluation
        if ((pos->pawns[BLACK] & FilesBitMask[FilesBoard[sq]]) == 0)
            score -= rookSemiOpenFile;
    }

    // Queen Open File and Semi Open File evaluation
    piece = wQ;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];

        // Open File Evaluation
        if ((pos->pawns[BOTH] & FilesBitMask[FilesBoard[sq]]) == 0)
            score += queenOpenFile;

        // Semi-Open File Evaluation
        if ((pos->pawns[WHITE] & FilesBitMask[FilesBoard[sq]]) == 0)
            score += queenSemiOpenFile;
    }

    piece = bQ;
    for (piece_num = 0; piece_num < pos->piece_num[piece]; piece_num++)
    {
        sq = pos->piece_list[piece][piece_num];

        // Open File Evaluation
        if ((pos->pawns[BOTH] & FilesBitMask[FilesBoard[sq]]) == 0)
            score -= queenOpenFile;

        // Semi-Open File Evaluation
        if ((pos->pawns[BLACK] & FilesBitMask[FilesBoard[sq]]) == 0)
            score -= queenSemiOpenFile;
    }

    //King Piece Square Score Evaluation
    /*
        The king, which normally should stay hidden during the game should become active in the endgame, as 
        it can help escort pawns to the promotion square, attack enemy pawns, protect other pieces, and restrict 
        the movement of the enemy king.
    */
   piece = wK;
   sq = pos->piece_list[wK][0];

    if((pos->material_score[BLACK] <= ENDGAME_BEGIN))
        score += kingEndGameTable[SQ64(sq)];  //|_> Endgame Begining Score(1000 cp)
    
    else 
        score += kingWhileOpening[SQ64(sq)];
    
    piece = bK;
    sq = pos->piece_list[piece][0];

    if((pos->material_score[WHITE] <= ENDGAME_BEGIN))
        score -= kingEndGameTable[MIRROR64(SQ64(sq))];
    
    else 
        score -= kingWhileOpening[MIRROR64(SQ64(sq))];
    
    //Bishop Pair Evaluation
    if(pos->piece_num[wB] >= 2)
        score += bishopPair;
    
    if(pos->piece_num[bB] >= 2)
        score -= bishopPair;

    // Depending on side, we return positive or negative score
    if (pos->side == WHITE)
        return score;
    else
        return -score;
}
