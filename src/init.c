/*
    init -> This Module is used to initialize all the neccessary global data required for
            proper functioning of board position
*/
#include "defs.h"

//For Pawn Bitboard
uint64_t ClearMask[SQUARE_TYPES];
uint64_t SetMask[SQUARE_TYPES];
uint64_t FilesBitMask[FILES];
uint64_t RanksBitMask[RANKS];
uint64_t EmptyRankBitMask[AGENTS][RANKS];
uint64_t IsolatedBitMask[SQUARE_TYPES];
uint64_t BlackPassedBitMask[SQUARE_TYPES];
uint64_t WhitePassedBitMask[SQUARE_TYPES];

int32_t Sq120ToSq64[BOARD_SQUARES]; //Mapping 120 based index to 64 squares
int32_t Sq64ToSq120[SQUARE_TYPES];  //Mapping 64 based index to 120 squares

//Data(Random64) for Zobrist hashkeys
uint64_t PiecePositionKeys[PIECE_TYPES][BOARD_SQUARES];
uint64_t SideKey;
uint64_t CastleKeys[16];

int32_t MvvLvaScores[PIECE_TYPES][PIECE_TYPES];

/*
99 99 99 99 99 99 99 99 99 99
99 99 99 99 99 99 99 99 99 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99  0  1  2  3  4  5  6  7 99
99 99 99 99 99 99 99 99 99 99
99 99 99 99 99 99 99 99 99 99
   a  b  c  d   e  f  g  h
*/
int32_t FilesBoard[BOARD_SQUARES];

/*
99 99 99 99 99 99 99 99 99 99
99 99 99 99 99 99 99 99 99 99
99  0  0  0  0  0  0  0  0 99  // RANK_1
99  1  1  1  1  1  1  1  1 99  // RANK_2
99  2  2  2  2  2  2  2  2 99  // RANK_3
99  3  3  3  3  3  3  3  3 99  // RANK_4
99  4  4  4  4  4  4  4  4 99  // RANK_5
99  5  5  5  5  5  5  5  5 99  // RANK_6
99  6  6  6  6  6  6  6  6 99  // RANK_7
99  7  7  7  7  7  7  7  7 99  // RANK_8
99 99 99 99 99 99 99 99 99 99
99 99 99 99 99 99 99 99 99 99
*/
int32_t RanksBoard[BOARD_SQUARES];

//FUNCTION DEFINITIONS
extern void init_sq120_to_sq64()
{
    int file = FILE_A, rank = RANK_1;
    int sq = A1, sq64 = 0;

    for(int index = 0; index < BOARD_SQUARES; index++)
        Sq120ToSq64[index] = NO_SQ;


    for(int index = 0; index < 64; index++)
        Sq64ToSq120[index] = BOARD_SQUARES;

    for(rank = RANK_1; rank <= RANK_8; rank++)
    {
        for(file = FILE_A; file <= FILE_H; file++)
        {
            sq = FROM_SQ64_TO_SQ120(file, rank);
            Sq64ToSq120[sq64] = sq;
            Sq120ToSq64[sq] = sq64++;
        }
    }
}

extern void init_bitmasks()
{
    for(int index = 0; index < SQUARE_TYPES; index++)
    {
        SetMask[index] |= (1ULL << index);
        ClearMask[index] = ~SetMask[index];
    }
}

extern void init_hash64_keys()
{
    for(int piece = EMPTY; piece < PIECE_TYPES; piece++)
        for(int sq = 0; sq < BOARD_SQUARES; sq++)
            PiecePositionKeys[piece][sq] = RAND_64;
    
    SideKey = RAND_64;

    for(int index = 0; index < 16; index++)
        CastleKeys[index] = RAND_64;
        
}

extern void init_files_ranks()
{
    for(int sq = 0; sq < BOARD_SQUARES; sq++)
        FilesBoard[sq] = RanksBoard[sq] = OFFBOARD;

    for(int rank = RANK_1; rank <= RANK_8; ++rank)
    {
        for(int file = FILE_A; file <= FILE_H; file++)
        {
            int sq = FROM_SQ64_TO_SQ120(file, rank);
            FilesBoard[sq] = file;
            RanksBoard[sq] = rank;
        }
    }
}

extern void init_mvv_lva()
{
    for(int32_t attacker = wP; attacker <= bK; attacker++)
    {
        for(int32_t victim = wP; victim <= bK; ++victim)
            MvvLvaScores[victim][attacker] = (VictimScore[victim] + 6) - (VictimScore[attacker] / 100);
    }
}

extern void init_pawn_eval_mask()
{
    memset(FilesBitMask, 0, sizeof(FilesBitMask));
    memset(RanksBitMask, 0,sizeof(RanksBitMask));

    for(int32_t rank = RANK_8; rank >= RANK_1; rank--)
    {
        for(int32_t file = FILE_A; file <= FILE_H; file++)
        {
            int32_t sq = rank * 8 + file;
            SET_BIT(FilesBitMask[file], sq);
            SET_BIT(RanksBitMask[rank], sq);
        }
    }

    EmptyRankBitMask[BLACK][RANK_8] = ~RanksBitMask[RANK_8];
    for(int32_t rank = RANK_7; rank >= RANK_1; rank--)
        EmptyRankBitMask[BLACK][rank] = (EmptyRankBitMask[BLACK][rank + 1] & ~RanksBitMask[rank]); 
    
    for(int32_t rank = RANK_1; rank < RANK_8; rank++)
       EmptyRankBitMask[WHITE][rank] = ~(EmptyRankBitMask[BLACK][rank + 1]);

    for(int32_t rank = RANK_8; rank >= RANK_1; rank--)
    {
        for(int32_t file = FILE_A; file <= FILE_H; file++)
        {
            int sq = rank * 8 + file;

            //ISOLATED PAWN AREA MASKING
            if((file - 1) >= FILE_A)
                IsolatedBitMask[sq] |= FilesBitMask[file - 1];
            
            if((file + 1) <= FILE_H)
                IsolatedBitMask[sq] |= FilesBitMask[file + 1];

            //PASSED PAWN AREA MASKING
            BlackPassedBitMask[sq] |= (IsolatedBitMask[sq] | FilesBitMask[file]) & EmptyRankBitMask[BLACK][rank];
            WhitePassedBitMask[sq] |= (IsolatedBitMask[sq] | FilesBitMask[file]) & EmptyRankBitMask[WHITE][rank];
         }
    }
}

extern void init_all()
{
    init_sq120_to_sq64();
    init_bitmasks();
    init_hash64_keys();
    init_files_ranks();
    init_mvv_lva();
    init_pawn_eval_mask();
}