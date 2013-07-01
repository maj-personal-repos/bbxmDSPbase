/*
 *   debug.h
 */

#include <stdio.h>
#include <ncurses.h>

// Enables or disables debug output
#ifdef _DEBUG_
    //#define DBG(fmt, args...) fprintf(stderr, "Debug: " fmt, ## args)
    //#define DBG(fmt, args...) mvprintw(0,0,"Debug: " fmt, ## args); refresh()
    #define DBG(fmt, args...)
    #define dspTraceDump(ce) Engine_fwriteTrace((ce), "[DSP] ", stderr)
#else
    #define DBG(fmt, args...)
    #define dspTraceDump(ce)
#endif

#define ERR(fmt, args...) fprintf(stderr, "Error: " fmt, ## args)
#define PRN(fmt, args...) mvprintw(0,0,fmt, ## args); refresh()
