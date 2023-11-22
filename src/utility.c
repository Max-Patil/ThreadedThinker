//HEADER FILES
#include<stdio.h>
#include<stdlib.h>
#include "defs.h"

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <sys/time.h>
#endif

extern int get_time_ms()
{
    #ifdef _WIN32
        return GetTickCount();
    #else
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    #endif
}


int input_waiting()
{
#ifndef _WIN32 
  fd_set readfds;
  struct timeval tv;
  FD_ZERO(&readfds);
  FD_SET(fileno(stdin), &readfds);
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else   //
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init)
  {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe)
    {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }

  if (pipe)
  {
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) //peeking( looking ) if there is anything available on standard input
      return 1;
    return dw;
  }
  else
  {
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw;
  }
#endif
}

extern void read_input(search_info_t* info)
{
  int bytes;
  char input[526] = "", *endc;

  if (input_waiting())
  {
    info->stopped = TRUE;
    
    do
    {
      bytes = read(fileno(stdin), input, 526);
    } while (bytes < 0);

    endc = strchr(input, '\n');

    if (endc)
      *endc = 0;

    if (strlen(input) > 0)
      if (!strncmp(input, "quit", 4))
        info->quit = TRUE;

    return;
  }
}