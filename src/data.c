/*
    Data : This module contains all elemental data of chess Board that is used
    by other module either to query the given piece or simply for input/output
*/

#include "defs.h"

char PiecesChars[PIECE_TYPES]       = ".PNBRQKpnbrqk";
char SideChars[3]                   = "wb-";
char RankChars[RANKS]               = "12345678";
char FileChars[FILES]               = "abcdefgh";

                                        // EMPTY,  PAWN,  KNIGHT,  BISHOP,  ROOK,  QUEEN,  KING,  pawn,  knight,  bishop,  rook,  queen,  king
const int32_t PieceBig[PIECE_TYPES]     = {  false,  false,  true,   true,   true,   true,   true,  false,  true,  true,  true,  true,  true  };
const int32_t PieceMajor[PIECE_TYPES]   = {  false,  false,  false,  false,   true,   true,  true,  false,  false,  false,  true,  true,  true };
const int32_t PieceMinor[PIECE_TYPES]   = {  false,  false,  true,   true,   false,   false, false, false,  true,   true,  false,  false, false };
const int32_t PieceValue[PIECE_TYPES]   = {  0,      100,    325,    325,    550,     1000,  50000, 100,    325,    325,   550,   1000,   50000 };
const int32_t PieceColor[PIECE_TYPES]   = {  BOTH,   WHITE,  WHITE,  WHITE,  WHITE,  WHITE,  WHITE,  BLACK,  BLACK,  BLACK, BLACK, BLACK,  BLACK };

                                                 // EMPTY,  PAWN,  KNIGHT,  BISHOP,  ROOK,  QUEEN,  KING,  pawn,  knight,  bishop,  rook,  queen,  king
const int32_t PiecePawn[PIECE_TYPES]             = { false,  true,  false,  false,  false,  false,  false,  true,  false,  false,  false,  false,  false };
const int32_t PieceKnight[PIECE_TYPES]           = { false,  false,  true,  false,  false,  false,  false,  false,  true,  false,  false,  false , false };
const int32_t PieceRookQueen[PIECE_TYPES]        = { false,  false,  false,  false,  true,  true,  false,  false,  false,  false,  true,  true,  false};
const int32_t PieceBishopQueen[PIECE_TYPES]      = { false,  false,  false,  true,  false,  true,  false,  false,  false,  true,  false,  true,  false };
const int32_t PieceKing[PIECE_TYPES]             = { false,  false,  false,  false, false,  false,  true,  false, false,  false,  false,  false, true };
       
                                          //EMPTY, wP,  wN,  wB,  wR,  wQ,  wK,   bP,  bN,  bB,  bR,  bQ,  bK
const int32_t VictimScore[PIECE_TYPES] = {  0  , 100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600 };

                                    // PV Moves Capture Moves,        Quiet Beta Cutoff Moves(1),     Quiet Beta Cutoff Moves(2),   
const uint64_t PriorityLevel[5] = { 20000000,        1000000,            900000,               800000,                      80000};

const int32_t Mirror64[SQUARE_TYPES] = {
    56, 57, 58, 59, 60, 61, 62, 63, // RANK 8
    48, 49, 50, 51, 52, 53, 54, 55, // RANK 7
    40, 41, 42, 43, 44, 45, 46, 47, // RANK 6
    32, 33, 34, 35, 36, 37, 38, 39, // RANK 5
    24, 25, 26, 27, 28, 29, 30, 31, // RANK 4
    16, 17, 18, 19, 20, 21, 22, 23, // RANK 3
    8, 9, 10, 11, 12, 13, 14, 15,   // RANK 2
    0, 1, 2, 3, 4, 5, 6, 7,         // RANK 1
};

