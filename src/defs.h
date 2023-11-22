#ifndef DEFS_H
#define DEFS_H

//HEADER FILES 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "fpd.h"
#include <omp.h>

//BUG INFORMER
#ifndef DEBUG
#define ASSERT(condition)
#else
#define ASSERT(condition)               \
if(!(condition))          {             \
    printf("Failed : %s ", #condition); \
    printf("on %s ", __DATE__);         \
    printf("at %s ", __TIME__);         \
    printf("in file %s ", __FILE__);    \
    printf("at line %d\n", __LINE__);   \
    exit(EXIT_FAILURE);  }
#endif

//SYMBOLIC CONSTANTS DEFINITIONS
#define NAME "Threaded Thinker"
#define BOARD_SQUARES 120
#define MAX_GAME_MOVES 1024
#define MAX_POSITIONS_MOVES 256
#define MAX_DEPTH 64
#define OFFBOARD NO_SQ
#define RANKS  8
#define FILES  8
#define AGENTS 2
#define PIECE_TYPES 13
#define SQUARE_TYPES 64
#define NO_MOVE     0
#define DRAW        0
#define STALEMATE   0
#define INVALID    -1
#define INF         30000
#define IS_MATE     (INF - MAX_DEPTH)

//      0000 000000000000000 000000000000000 000000000000000 111111111111111
//      0000 000000000000000 000000000000000 111111111111111 000000000000000
//      0000 000000000000000 111111111111111 000000000000000 000000000000000
//      0000 111111111111111 000000000000000 000000000000000 000000000000000
//  OR  1111 000000000000000 000000000000000 000000000000000 000000000000000    

#define RAND_64     ( (uint64_t)rand() << 0  |   \
                      (uint64_t)rand() << 15 |   \
                      (uint64_t)rand() << 30 |   \
                      (uint64_t)rand() << 45 |   \
                      ((uint64_t)rand() & 0xf) << 60 )


//MACRO DEFINITIONS
#define OPPONENT(side) (side ^ 1)

//Representing move information using hexadecimal

/*
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
*/

#define MOVE(from, to, cap, prom, flag) ( (from) | ((to) << 7) | ((cap) << 14) | ((prom) << 20) | (flag))
#define FROM_SQ(move)   ((move) & 0x7f)
#define TO_SQ(move)     (((move) >> 7) & 0x7f)
#define CAPTURE(move)   (((move) >> 14) & 0xf)
#define PROMOTED(move)  (((move) >> 20) & 0xf)

#define MV_FLAG_EP      0x00040000 //  Expassant flag
#define MV_FLAG_PS      0x00080000 //  Pawn start flag
#define MV_FLAG_CA      0x01000000 //  Castle flag
#define MV_FLAG_CP      0x0007C000 //  Capture flag
#define MV_FLAG_PROM    0x00F00000 //  Promoted flag

//Array Mapping From 120 to 64 and 64 to 120 squares
#define SQ120(sq64)   Sq64ToSq120[sq64]
#define SQ64(sq120)   Sq120ToSq64[sq120]
#define MIRROR64(sq)  Mirror64[(sq)]

//Query through Mapped Array
#define IS_KNIGHT(piece)   PieceKnight[(piece)]
#define IS_PAWN(piece)     PiecePawn[(piece)]
#define IS_KING(piece)     PieceKing[(piece)]
#define IS_RQ(piece)       PieceRookQueen[(piece)]
#define IS_BQ(piece)       PieceBishopQueen[(piece)]

#define IS_MAJOR(piece)    PieceMajor[(piece)]
#define IS_MINOR(piece)    PieceMinor[(piece)]
#define IS_BIG(piece)      PieceBig[(piece)]
#define COLOR_OF(piece)    PieceColor[(piece)]
#define VALUE_OF(piece)    PieceValue[(piece)]

//For Bit Manipulation 
#define CLEAR_BIT(bb, sq) (bb = (bb) & ClearMask[(sq)])
#define SET_BIT(bb, sq)   (bb = (bb) | SetMask[(sq)])
#define COUNT_BITS(bb)    (count_bits(bb))
#define POP_BIT(bb)       (pop_bit(&bb))

#define FROM_SQ64_TO_SQ120(file_no, rank_no)  ( (21 + (file_no)) + ((rank_no) * 10) )
#define FILE_RANK_TO_SQ64(file_no, rank_no)   ( ((rank_no) * 8) + (file_no) )

//GLOBAL ARRAYS
extern int32_t Sq120ToSq64[BOARD_SQUARES]; //Mapping 120 based index to 64 squares
extern int32_t Sq64ToSq120[SQUARE_TYPES];  //Mapping 64 based index to 120 squares

//For Pawn Bitboard
extern uint64_t ClearMask[SQUARE_TYPES];
extern uint64_t SetMask[SQUARE_TYPES];
extern uint64_t FilesBitMask[FILES];
extern uint64_t RanksBitMask[RANKS];
extern uint64_t EmptyRankBitMask[AGENTS][RANKS];
extern uint64_t IsolatedBitMask[SQUARE_TYPES];
extern uint64_t BlackPassedBitMask[SQUARE_TYPES];
extern uint64_t WhitePassedBitMask[SQUARE_TYPES];

//For I/O of Chess Board Elements
extern char PiecesChars[PIECE_TYPES];
extern char SideChars[3]; //{ WHITE, BLACK, BOTH }
extern char RankChars[RANKS];
extern char FileChars[FILES];

extern const int32_t PieceValue[PIECE_TYPES];
extern const int32_t PieceColor[PIECE_TYPES];
extern int32_t FilesBoard[BOARD_SQUARES];
extern int32_t RanksBoard[BOARD_SQUARES];

extern const int32_t PieceBig[PIECE_TYPES];
extern const int32_t PieceMajor[PIECE_TYPES];
extern const int32_t PieceMinor[PIECE_TYPES];

extern const int32_t PieceKnight[PIECE_TYPES];
extern const int32_t PieceRookQueen[PIECE_TYPES];
extern const int32_t PieceBishopQueen[PIECE_TYPES];
extern const int32_t PieceKing[PIECE_TYPES];
extern const int32_t PiecePawn[PIECE_TYPES];

extern const int32_t Mirror64[SQUARE_TYPES];

//Data(Random64) for Zobrist hashkeys
extern uint64_t PiecePositionKeys[PIECE_TYPES][BOARD_SQUARES];
                                //   |_> 0TH PieceType i.e Empty is used for enpassant random64 hashkeys
extern uint64_t SideKey;
extern uint64_t CastleKeys[16];

//For Priortizing search
extern const uint64_t PriorityLevel[5];
extern const int32_t  VictimScore[PIECE_TYPES];
extern int32_t MvvLvaScores[PIECE_TYPES][PIECE_TYPES];


//ENUMERATORS
enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK }; //-> PIECE TYPES

enum{ FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE }; //-> FILES

enum{ RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE }; //-> RANKS

enum{ WHITE, BLACK, BOTH }; //SIDES

enum{ HF_NONE, HF_ALPHA, HF_BETA, HF_EXACT }; //-> FLAG FOR HASH TABLE

enum{ wKCA = 1, wQCA = 2, bKCA = 4, bQCA = 8}; // -> CASTLING RIGHTS

enum    // SQUARE TYPES
{
    A1 = 21, B1, C1, D1, E1, F1, G1, H1, 
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ
};

//TYPE DEFINITIONS
typedef struct Move move_t;
typedef struct MoveList move_list_t;
typedef struct HashEntry hash_entry_t;
typedef struct HashTable hash_table_t;
typedef struct UndoMove undo_move_t;
typedef struct ChessBoard board_t;
typedef struct SearchInfo search_info_t;

//INFORMATION STRUCTURES
struct Move
{
    int32_t move;
    int32_t score;
};

struct MoveList
{
    move_t moves[MAX_POSITIONS_MOVES];
    int32_t count; 
};

struct HashEntry
{
    uint64_t pos_key;
    int32_t move;
    int32_t score;
    int32_t depth;
    int32_t flags;
};

struct HashTable
{
    hash_entry_t* hash_entries;

    int32_t hit;
    int32_t cut;
    int32_t new_writes;
    int32_t over_writes;

    size_t total_entries;
};

struct UndoMove 
{
    //Data that is used to maintain state of the game
    int32_t move;
    int32_t castle_perm;
    int32_t enpassant;
    int32_t fifty_move;
    uint64_t pos_key;
};

struct ChessBoard
{
    //Board Position 
    int32_t board[BOARD_SQUARES];
    int32_t  king_sq[AGENTS];
    int32_t  pawns[AGENTS + 1];
    int32_t  enpassant_sq;
    undo_move_t board_history[MAX_GAME_MOVES];

    //Board Attributes
    int32_t side;
    int32_t fifty_move;
    int32_t ply;
    int32_t hist_ply;
    uint64_t pos_key;
    int32_t castle_perm;

    //Piece Attributes
    int32_t piece_num[PIECE_TYPES];
    int32_t material_score[AGENTS];
    int32_t big_pieces[AGENTS];
    int32_t major_pieces[AGENTS];
    int32_t minor_pieces[AGENTS];
    int32_t piece_list[PIECE_TYPES][10];
                                //  |_> Maximum number of pieces we can have on board is 10

    //Hash Table for board
    hash_table_t hash_table[1];
    int32_t pv_array[MAX_DEPTH];

    //Heuristics for Move ordering while searching
    int32_t search_history[PIECE_TYPES][BOARD_SQUARES];
    int32_t search_killers[AGENTS][MAX_DEPTH];
};

struct SearchInfo
{
    //Time Control Params for search
    int32_t start_time;
    int32_t stop_time;
    int32_t set_time;
    int32_t stopped;
    int32_t quit;
    int32_t moves_to_go;

    //Moves(Nodes) Attributes
    size_t total_enpassant;
    size_t total_checks;
    size_t total_quiet;
    size_t total_captures;
    size_t leaf_nodes;
    size_t total_nodes;

    //Search Attributes
    int32_t depth;
    int32_t set_depth;
    float fail_high;
    float fail_high_first;
    int32_t null_cut;
};

//FUNCTION DECLARATIONS

//Module - init
extern void init_all();
extern void init_sq120_to_sq64();
extern void init_hash64_keys();
extern void init_files_ranks();
extern void init_bitmasks();
extern void init_mvv_lva();
extern void init_pawn_eval_mask();

//Module - bitboard
extern void print_bitboard(uint64_t bb64);
extern inline int32_t pop_bit(uint64_t* pos);
extern inline int32_t count_bits(uint64_t bb64);

//Module - hashkeys
extern uint64_t generate_pos_key(const board_t* pos);

//Module - board
extern void reset_board(board_t* pos);
extern int32_t parse_fen(board_t* pos, char *fen);
extern void print_board(const board_t* pos);
extern void update_list_material(board_t* pos);
extern int check_board(board_t* pos);
extern void mirror_board(board_t* pos);

//Module - attack
extern int is_square_attacked(const board_t* pos, int32_t side, int32_t sq);
extern void show_attack_squares(const board_t* pos,  int32_t side);

//Module - I/O
extern char* print_square(const int32_t sq);
extern char* print_move(const int32_t move);
extern void print_move_list(move_list_t* p_list);
extern int32_t parse_move(const board_t* pos, char* s_move);

//Module - move generator
extern void generate_all_moves(const board_t* pos, move_list_t* p_list);
extern bool check_move_list(const board_t* pos, const move_list_t* p_list);
extern bool move_exists(board_t* pos, int32_t move);
extern void generate_all_capture_moves(const board_t* pos, move_list_t* p_list);
extern void undo_null_move(board_t* pos);
extern void make_null_move(board_t* pos);

//Module - validate
extern inline bool sq_on_board(const int32_t sq);
extern inline bool is_side_valid(const int side);
extern inline bool is_file_rank_valid(const int32_t fr);
extern inline bool is_piece_valid(const int piece);

//Module - makemove
extern bool make_move(board_t* pos, const int32_t move);
extern void undo_move(board_t* pos);

//Module - perft 
extern void perft_test(board_t* pos, int32_t depth, search_info_t* search_info);

//Module - search
extern void search_position(board_t* pos, search_info_t* search_info);

//Module - utility
extern int get_time_ms();

//Module - pvtable
extern void init_hash_table(hash_table_t *hash_table);
extern void store_pv_move(board_t* pos, const int32_t move);
extern int32_t probe_pv_move(const board_t* pos);
extern int32_t get_pv_line(board_t* pos, const int32_t depth);

//Module - evaluate
extern int32_t evaluate(const board_t* pos);
extern inline bool material_draw(const board_t* pos);

//Module - UCI 
extern void uci_loop(board_t* pos, search_info_t *search_info);

#endif // DEFS_H