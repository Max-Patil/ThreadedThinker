/*
    This module is simply used to test the performance and verify whether or not
    move generator generates correct results
*/

#include "defs.h"

extern void perft(board_t* pos, int32_t depth, search_info_t* search_info)
{
    ASSERT(check_board(pos));

    if(depth == 0)
    {
        search_info->leaf_nodes++;
        return;
    }

    move_list_t list;
    generate_all_moves(pos, &list);

    for(int move_num = 0; move_num < list.count; move_num++)
    {
        int32_t move = list.moves[move_num].move;

        if(!make_move(pos, move))
            continue;

        perft(pos, depth - 1, search_info);
        undo_move(pos);

    }
}

extern void perft_test(board_t* pos, int32_t depth, search_info_t* search_info)
{
    ASSERT(check_board(pos));

    print_board(pos);
    printf("\nStarting Test to Depth : %d\n", depth);

    move_list_t list;
    generate_all_moves(pos, &list);

    int32_t move = NO_MOVE, start = get_time_ms();
    size_t cummulative_nodes = search_info->leaf_nodes, old_nodes = 0;

    for(int move_num = 0; move_num < list.count; ++move_num)
    {
        move = list.moves[move_num].move;

        if(!make_move(pos, move))
            continue;
   
        cummulative_nodes = search_info->leaf_nodes;
        perft(pos, depth - 1, search_info);

        undo_move(pos);
        old_nodes = search_info->leaf_nodes - cummulative_nodes;
        printf("move %2d -> %5s : %lu\n", move_num + 1, print_move(move), old_nodes);
    }
    
    printf("\nTest complete : Total %lu leaf nodes visited in %d ms\n\n", search_info->leaf_nodes, get_time_ms() - start);
}