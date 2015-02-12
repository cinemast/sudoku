/* Dummy header for Win32 port
 *
 * Provide prototypes for required 'curses' functions.
 */

#include <Windows.h>

typedef void * Window;

#define stdscr 0

#define A_BOLD   (FOREGROUND_INTENSITY)

void wclear (Window *);
void attron (unsigned short);
void attroff (unsigned short);
void mvaddstr (int, int, const char *);
void addch (int);
void move (int, int);
void wclrtoeol (Window *);
void wrefresh (Window *);
void redrawwin (Window *);
void mvaddch (int, int, int);
int wgetch (Window *);
void beep (void);
void noecho (void);
void raw (void);
void cbreak (void);

int initscr (void);
void endwin (void);

