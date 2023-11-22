// HEADER FILES
#include "ctype.h"
#include "defs.h"

board_t ChessBoard;
search_info_t search_info[1];

// ENTRY POINT OF ALL FUNCTION
int main(void)
{
    init_all();

    uci_loop(&ChessBoard, search_info);
    ASSERT(check_board(&ChessBoard));

    free(ChessBoard.hash_table->hash_entries);
    return EXIT_SUCCESS;
}