/*
    Search - This module is used to drive search process that helps to find nodes in search tree that is considered as best with respect to side that is
    playing by looking ahead from the given position to certain depth
*/

#include "defs.h"

#define R 4 // R represents Depth Reduction

int is_null = 0;

static void check_up(search_info_t *search_info)
{
    // Check if time is up or interrupted by Gui
    if (search_info->set_time == true && get_time_ms() > search_info->stop_time)
        search_info->stopped = true;

    read_input(search_info);// -> input polling
    /*
        Note : After every 2047 nodes, we used to check whether or not, there is quit
        command string available in standard input and if it found quit command then
        program will simply quit else it will simply stop the search and will take input
        from uci_loop
    */
}

static bool is_repetition(const board_t *pos)
{
    for (int index = pos->hist_ply - pos->fifty_move; index < pos->hist_ply - 1; index++)
    {
        if (pos->pos_key == pos->board_history[index].pos_key)
            return true;
    }

    return false;
}

static void pick_next_move(move_list_t *p_list, int32_t move_num)
{
    move_t temp_move;
    int32_t best_score = 0, best_num = move_num, score;

    for (int index = move_num; index < p_list->count; index++)
    {
        score = p_list->moves[index].score;
        if (score > best_score)
        {
            best_score = score;
            best_num = index;
        }
    }

    if (move_num != best_num)
    {
        temp_move = p_list->moves[move_num];
        p_list->moves[move_num] = p_list->moves[best_num];
        p_list->moves[best_num] = temp_move;
    }
}

static void clear_search(board_t *pos, search_info_t *search_info)
{
    // Clearing history and killers heuristics Information
    for (int piece_type = EMPTY; piece_type < bK; piece_type++)
        for (int t_sq = 0; t_sq < BOARD_SQUARES; t_sq++)
            pos->search_history[piece_type][t_sq] = 0;

    for (int side = 0; side < AGENTS; side++)
        for (int depth = 0; depth < MAX_DEPTH; depth++)
            pos->search_killers[side][depth] = 0;

    // Clearing Principle Variation Table
    memset(pos->hash_table->hash_entries, 0, sizeof(hash_entry_t) * pos->hash_table->total_entries);

    // Reset Ply (depth that goes in current search)
    pos->ply = 0;

    // Initialize search information
    search_info->stopped = false;
    search_info->leaf_nodes = 0;
    search_info->total_nodes = 0;
    search_info->fail_high = 1;
    search_info->fail_high_first = 0;

    // Initialize hash table
    pos->hash_table->over_writes = 0;
    pos->hash_table->hit = 0;
    pos->hash_table->cut = 0;
}

static int32_t quiescence(board_t *pos, search_info_t *search_info, int32_t alpha, int32_t beta)
{
    ASSERT(check_board(pos));
    ASSERT(beta > alpha);

    if ((search_info->total_nodes & 2047) == 0)
        check_up(search_info);

    search_info->total_nodes++;

    if ((is_repetition(pos) || pos->fifty_move >= 100) && pos->ply)
        return DRAW;

    if (pos->ply > MAX_DEPTH - 1)
        return evaluate(pos);

    int32_t score = evaluate(pos);
   
    // printf("score : %d alpha : %d and beta : %d\n", score, alpha, beta);

    if (score >= beta)
        return beta;

     if (score > alpha)
        alpha = score;

    move_list_t list;
    generate_all_capture_moves(pos, &list);

    int32_t legal_moves = 0, best_move = NO_MOVE, move = NO_MOVE;
    score = -INF;

    for (int move_num = 0; move_num < list.count; move_num++)
    {
        move = list.moves[move_num].move;

        if (!make_move(pos, move))
            continue;

        legal_moves++;

        score = -quiescence(pos, search_info, -beta, -alpha);

        undo_move(pos);

        if (search_info->stopped == true)
            return 0;

        if (score > alpha)
        {
            if (score >= beta)
            {
                if (legal_moves == 1)
                    search_info->fail_high_first++;

                search_info->fail_high++;

                return beta;
            }
            alpha = score;
        }
    }

    return alpha;
}

static int32_t alpha_beta(board_t *pos, search_info_t *search_info, int32_t depth, int32_t alpha, int32_t beta, bool do_null)
{

    ASSERT(check_board(pos));
    ASSERT(depth >= 0);

    if (depth <= 0)
        return quiescence(pos, search_info, alpha, beta);

   

    if ((search_info->total_nodes & 2047) == 0)
        check_up(search_info);

    search_info->total_nodes++;

    if ((is_repetition(pos) || pos->fifty_move >= 100) && pos->ply)
        return DRAW;

    if (pos->ply > MAX_DEPTH - 1)
        return evaluate(pos);

    int32_t in_check = is_square_attacked(pos, OPPONENT(pos->side), pos->king_sq[pos->side]);

    if (in_check == true) //-> depth extension if king is in check
        depth++;

    int32_t score = -INF;

   // Null Move Pruning with depth reduction 'R' and minimal window search
    while (do_null && !in_check && pos->ply > pos->hist_ply  && (pos->big_pieces[pos->side] > 1) && depth >= R)
    {
        make_null_move(pos);

        score = -alpha_beta(pos, search_info, depth - R, -beta, -beta + 1, false);

        undo_null_move(pos);

        if (search_info->stopped == true)
            return 0;

        if (score >= beta)
            return beta;
    }

    move_list_t list;
    generate_all_moves(pos, &list);

    int32_t legal_moves = 0, old_alpha = alpha, best_move = NO_MOVE, move = NO_MOVE;

    int32_t pv_move = probe_pv_move(pos);

    if (pv_move != NO_MOVE)
    {
        for (int move_num = 0; move_num < list.count; move_num++)
        {
            if (list.moves[move_num].move == pv_move)
            {
                list.moves[move_num].score = PriorityLevel[0];
                break;
            }
        }
    }

    score = -INF;

    for (int move_num = 0; move_num < list.count; move_num++)
    {
        pick_next_move(&list, move_num);
        move = list.moves[move_num].move;

        if (!make_move(pos, move))
            continue;

        legal_moves++;
        score = -alpha_beta(pos, search_info, depth - 1, -beta, -alpha, true);

        undo_move(pos);

        if (search_info->stopped == true)
            return 0;

        if (score > alpha)
        {
            if (score >= beta)
            {
                if (legal_moves == 1)
                    search_info->fail_high_first++;

                search_info->fail_high++;

                if (!(move & MV_FLAG_CP))
                {
                    pos->search_killers[1][pos->ply] = pos->search_killers[0][pos->ply];
                    pos->search_killers[0][pos->ply] = move;
                }

                return beta;
            }

            alpha = score;

            if (!(move & MV_FLAG_CP))
                pos->search_history[pos->board[FROM_SQ(move)]][TO_SQ(move)] += depth;

            best_move = move;
        }
    }

    // Detecting checkmate or stalemate
    if (legal_moves == 0)
    {
        if (in_check)
            return -IS_MATE + pos->ply;
        else
            return STALEMATE;
    }

    if (alpha != old_alpha)
        store_pv_move(pos, best_move);

    return alpha;
}

extern void search_position(board_t *pos, search_info_t *search_info)
{
    int32_t curr_depth = 0, pv_moves = 0, best_score = -INF;
    int32_t best_move = NO_MOVE, pv_num = 0;

    clear_search(pos, search_info);

    for (curr_depth = 1; curr_depth <= search_info->depth; curr_depth++)
    {
        best_score = alpha_beta(pos, search_info, curr_depth, -INF, INF, false);

        // are we out of time ?
        if (search_info->stopped == true)
            break;

        pv_moves = get_pv_line(pos, curr_depth);
        best_move = pos->pv_array[0];

        printf("info score cp %d depth %d nodes %ld time %d pv",
               best_score, curr_depth, search_info->total_nodes, get_time_ms() - search_info->start_time);

        pv_moves = get_pv_line(pos, curr_depth);

        for (pv_num = 0; pv_num < pv_moves; pv_num++)
            printf(" %s", print_move(pos->pv_array[pv_num]));

        printf("\n");
        // printf("Ordering : %.2f\n", (search_info->fail_high_first / search_info->fail_high));
    }
    printf("bestmove %s\n", print_move(best_move));
}
