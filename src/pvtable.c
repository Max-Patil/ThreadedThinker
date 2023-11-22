/*
    PvTable - This module helps search process by storing revelant information that can be retrieved while re-searching the same branch of the
    tree .In other words, PV Hash Table is kind of transposition table that is used to retrieved the result in order to continue searching optimum 
    line that is found earlier.
*/

#include "defs.h"

#define MB 2

const int32_t total_entries = 0x100000 * MB;

extern void init_hash_table(hash_table_t *hash_table)
{
    hash_table->total_entries = total_entries / sizeof(hash_entry_t);

    if(hash_table->total_entries)
        hash_table->hash_entries = (hash_entry_t*)calloc(hash_table->total_entries, sizeof(hash_entry_t));

    if(hash_table->hash_entries)
        printf("Hash Table initialize complete with %d entries\n", hash_table->total_entries);
        
}

extern void store_pv_move(board_t* pos, const int32_t move)
{
    int32_t index =(pos->pos_key) % (pos->hash_table->total_entries);
    ASSERT(index >= 0 && index <= pos->hash_table->total_entries - 1);

    pos->hash_table->hash_entries[index].move = move;
    pos->hash_table->hash_entries[index].pos_key = pos->pos_key;
}

extern int32_t probe_pv_move(const board_t *pos)
{
    int32_t index = pos->pos_key % pos->hash_table->total_entries;
    ASSERT(index >= 0 && index <= pos->hash_table->total_entries - 1);

    if(pos->hash_table->hash_entries[index].pos_key == pos->pos_key)
        return pos->hash_table->hash_entries[index].move;

    return NO_MOVE;
}

/* Below function returns number of moves that is obtained by making a move if exist and 
probe that position as we make a move to fill pv array */

extern int32_t get_pv_line(board_t* pos, const int32_t depth)
{

    int32_t pv_move = probe_pv_move(pos);
    int32_t count = 0;

    while(pv_move != NO_MOVE && count < depth)
    {

        // checking whether pv move is one of all possible moves generated from current board position
        if(move_exists(pos, pv_move))
        {
            make_move(pos, pv_move);
            pos->pv_array[count++] = pv_move;
        }
        else
            break;
        
        // Getting the next pv move from pv table after following pv move by making move
        pv_move = probe_pv_move(pos); 

        /* whenever pv table have legal move then we make the move and probe it again and again untill 
        we encountered no more move or illegal move that is generated because of collision */
    }

    // Decrementing ply of current search along pv line to return back to original position
    while(count && pos->ply > 0)
        undo_move(pos);
    
    return count;
}