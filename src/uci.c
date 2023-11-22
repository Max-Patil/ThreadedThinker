/*
    UCI - This module defines to uci protocols that is used to connect engine to GUI and that controls state
    of engine through GUI 
*/

#include"defs.h"
#include<ctype.h>

#define INPUT_BUFFER 2048
#define substr(parse_str, str_found) strstr(parse_str, str_found)

void parse_go(board_t* pos,  search_info_t* search_info, char* line)
{
    init_hash_table(pos->hash_table);
    //go depth 6 wtime 18000 btime 18000 binc 1000 winc 1000 movetime 1000 movestogo 40
    //Note : Time is affected by given time to execute the search, increment, movetime and movestogo

    int depth = INVALID, move_time = INVALID, time = INVALID;
    int moves_to_go = 30, inc = 0;

    char *ptr = NULL;
    search_info->set_time = false; //we assume that we are in infinite analysis mode
    search_info->quit = false;

    if((ptr = substr(line, "infinite"))); //semicolon, null statement

    if((ptr = substr(line, "binc")) && pos->side == BLACK)
        inc = atoi(ptr + 5); //After skipping 5 characters i.e "binc ", we encounter actuall increment number
    
    if((ptr = substr(line, "winc")) && pos->side == WHITE)
        inc = atoi(ptr + 5);

    if((ptr = substr(line, "wtime")) && pos->side == WHITE)
        time = atoi(ptr + 6);
    
    if((ptr = substr(line, "btime")) && pos->side == BLACK)
        time = atoi(ptr + 6);
    
    if((ptr = substr(line, "movestogo")))
        moves_to_go = atoi(ptr + 10);
    
    if ((ptr = strstr(line,"movetime"))) 
		move_time = atoi(ptr + 9);

    if((ptr = substr(line, "depth")))
        depth = atoi(ptr + 6);
    
    //After setting or parsing string appropriately
    if(move_time != INVALID)
    {
        time = move_time;
        moves_to_go = 1;
    }

    search_info->start_time = get_time_ms();
    search_info->depth = depth;

    if(time != INVALID)
    {
        search_info->set_time = true;
        time = time / moves_to_go;
        time = time - 50; //substracting 50 ms so that we don't over run
        search_info->stop_time = search_info->start_time + time + inc;
    }

    if(depth == INVALID) //If depth wasn't set appropriately then set it to max depth
        search_info->depth = MAX_DEPTH;
    
    printf("time : %d, start : %d ms, stop : %d ms, depth : %d, time_set = %d\n\n", time, search_info->start_time, 
          search_info->stop_time, search_info->depth, search_info->set_time);

    search_position(pos, search_info);
}

void parse_position(board_t* pos, char* input) //-> It simply parse the input i.e what position to start from
{
   
    //Following all possible valid parse string 

    //1) position fen "fen string"
    //2) position startpos
    //3) .... moves e2e4 e7e5 b7b8q
    
    //Parsing input string till fen notation
    input += 9; //To skip "position " string, total size is 9 bytes
    char *ptr = NULL;

    if(!strncmp(input, "startpos", 8))
        parse_fen(pos, START_FEN);
    else
    {
        /*
            strstr() is a standard C library function that searches for the 
            first occurrence of the substring "fen" in the string
        */
        ptr = substr(input, "fen");

        if(ptr) //if substring is found after parsing input string then parse fen notation string
        {
            ptr += 5; //skipping "fen " string part
            parse_fen(pos, ptr);
        }
        else    //otherwise set fen string as starting position by default
            parse_fen(pos, START_FEN);
    }

    //Parsing input as moves i.e .... moves e2e4 e5e7   f2c6q
    ptr = substr(input, "moves");
    int move = NO_MOVE;

    if(ptr) //if we found occurences of "moves" as string in input string
    {
        ptr += 6; //skipping "moves" string from input string

        while(*ptr) //Loop over remanining algebric moves till the end of input string
        {
            move = parse_move(pos, ptr);
            if(move == NO_MOVE)
                break;

            //if move parsed is valid then make move and starting searching another algebric move string
            make_move(pos, move);

            pos->ply = 0; //Setting ply to 0, as while making move, we increment ply

            while(*ptr && *ptr != ' ')  //skipping spaces
                ptr++;

            ptr++; //Increment by 1 to start of new move string
        }
    }

    print_board(pos);
    move_list_t list;

    generate_all_moves(pos, &list);
    print_move_list(&list);

}

void console_mode(board_t* pos, search_info_t* search_info)
{
    char input[32];
    bool quit = false;
    int32_t move = NO_MOVE;

    if(pos->pos_key == 0)
        parse_fen(pos, START_FEN);

    while (true)
    {
        print_board(pos);

        printf("\n\nPlease enter a command > ");
        fgets(input, 32, stdin);

        if (input[0] == 'q') // quit
        {
            search_info->quit = true;
            break;
        }
        else if(input[0] == 'u') // uci mode
            break;
        
        else if (input[0] == 't') // undo move
        {
            if (pos->hist_ply)
                undo_move(pos);
        }
        else if (input[0] == 'p') // perft
        {
            char *ptr = input;
            ptr += 2;
            int max_depth = atoi(ptr);
         
            printf("\nPerft to depth : %d\n\n", max_depth);
            perft_test(pos, max_depth, search_info);
            search_info->leaf_nodes = 0;
            search_info->total_nodes = 0;

        }
        else // make move
        {
            move = parse_move(pos, input);
            if (move)
            {
                make_move(pos, move);
            }
            else
                printf("move : %s not parsed\n", input);
        }

        fflush(stdin);
    }
}

void uci_loop(board_t *pos, search_info_t *search_info)
{
    
    /*
        In C programming language, setbuf() is a standard library function that is used to 
        change the buffering of a stream.

        By default, output to a stream is buffered, which means that the data is not immediately 
        written to the underlying file or device. Instead, it is stored in a buffer in memory and 
        written to the file or device at a later time, typically when the buffer is full or when 
        the stream is closed.
    */

   //Setting buffering for standard input and standard output to zero i.e turning off buffering
   //It is turn off, to not miss any command
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    char line[INPUT_BUFFER];

    printf("\nid name %s\n", NAME);
    puts("id author Mitesh Patil");
    puts("uciok\n");

    while(true)
    {

        memset(&line[0], 0, sizeof(line));
        fflush(stdout);  //Flushing everything that is send to console

        if(!fgets(line, INPUT_BUFFER, stdin))
            continue;   //Getting command from terminal to input buffer(line)
        
        if(line[0] == '\n') 
            continue;   //If we recieve newline from terminal, we simply query again
        
        if(!strncmp(line, "isready", 7))
        {   //Part of UCI Protocol 
            puts("readyok");
            continue;
        }
        else if(!strncmp(line, "position", 8))
            //parsing string of algebric moves 
            parse_position(pos, line);

        else if(!strncmp(line, "ucinewgame", 10))
            //parsing string of algebric moves with starting position as fen notation
            parse_position(pos, "position startpos");

        else if(!strncmp(line, "go", 2))
            //parsing info to search (think) for given position
            parse_go(pos, search_info, line);
        
        else if(!strncmp(line, "uci", 3))
        {
            printf("id name %s\n", NAME);
            puts("id author Mitesh Patil");
            puts("uciok");
        }
        else if(!strncmp(line, "quit", 4))
        {
            search_info->quit = true;
            break;
        }
        else if(!strncmp(line, "mirror", 6))
        {
            print_board(pos);
            printf("Evaluation : %d\n", evaluate(pos));
            mirror_board(pos);
            print_board(pos);
            printf("Mirror Evaluation : %d\n", evaluate(pos));
            continue;
        }
        else if(!strncmp(line, "console", 7))
            console_mode(pos, search_info);

        if(search_info->quit == true)
            break;  
        //Note : quit state can be set to true from other functional units that need to check to make sure that program will terminate as expected
    }
}