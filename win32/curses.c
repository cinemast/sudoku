/* Dumb curses implementation for Win32 port
 */

#include "curses.h"

static HANDLE _conin;
static HANDLE _conout;

#define FOREGROUND_WHITE (FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN)
#define WHITE_ON_BLACK FOREGROUND_WHITE

/* Cloning these definition from sudoku.c here, as
 * these virtual key codes wil vork just as good
 * the letter equivalents worked in the navigation,
 * but when typing the file name, these will make
 * possible the proper handling of the arrow keys.
 */
#define VKEY_UP         (256+'A')
#define VKEY_DOWN       (256+'B')
#define VKEY_RIGHT      (256+'C')
#define VKEY_LEFT       (256+'D')
#define VKEY_HOME       (256+'H')
#define VKEY_INSERT     (256+'2')
#define VKEY_DELETE     (256+'3')
#define VKEY_BACK       (0x08)

void
wclear (Window *    w)
{
    COORD sz, xy = { 0, 0 };
    DWORD wrote;
    if (w) {
        /*unreferenced formal parameter */
    }
    sz = GetLargestConsoleWindowSize(_conout);
    FillConsoleOutputCharacter(_conout, ' ', sz.X * sz.Y, xy, &wrote);
}

void
attron (unsigned short      attr)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(_conout, &csbi)) {
        csbi.wAttributes |= attr;
        SetConsoleTextAttribute(_conout, csbi.wAttributes);
    } else {
        csbi.wAttributes = (FOREGROUND_INTENSITY | WHITE_ON_BLACK);
    }
    SetConsoleTextAttribute(_conout, csbi.wAttributes);
}

void
attroff (unsigned short     attr)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(_conout, &csbi)) {
        csbi.wAttributes &= ~attr;
        SetConsoleTextAttribute(_conout, csbi.wAttributes);
    } else {
        csbi.wAttributes = WHITE_ON_BLACK;
    }
    SetConsoleTextAttribute(_conout, csbi.wAttributes);
}

void
move (int      y,
      int      x)
{
    COORD xy;

    xy.X = (SHORT)x;
    xy.Y = (SHORT)y;

    SetConsoleCursorPosition(_conout, xy);
}

void
mvaddstr (int            y,
          int            x,
          const char *   str)
{
    DWORD wrote;

    move(y, x);
#if defined (UNICODE)
    if (str != NULL) {
        size_t      returnValue = 0;
        size_t      str_size    = strlen(str);
        wchar_t *   wcsptr      = NULL;
        if (str_size > 0) {
            wcsptr = (wchar_t *) malloc(sizeof (wchar_t) * (str_size + 1));
            if (wcsptr != NULL) {
                if (0 == mbstowcs_s(&returnValue,
                                    wcsptr,
                                    str_size + 1,
                                    str,
                                    str_size)) {
                    WriteConsole(_conout,
                                 wcsptr,
                                 (DWORD)returnValue,
                                 &wrote,
                                 0);
                }
                free (wcsptr);
            }
        }
    }
#else
    WriteConsole(_conout, str, (DWORD)strlen(str), &wrote, 0);
#endif
}

void
addch (int      ch)
{
    DWORD wrote;
    WriteConsole(_conout, &ch, 1, &wrote, 0);
}

void
mvaddch (int        y,
         int        x,
         int        ch)
{
    move(y, x);
    addch(ch);
}

void
wclrtoeol (Window *     w)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    DWORD wrote;
    if (w) {
        /*unreferenced formal parameter */
    }
    GetConsoleScreenBufferInfo(_conout, &info);

    FillConsoleOutputCharacter(_conout,
                               ' ',
                               info.dwSize.X - info.dwCursorPosition.X,
                               info.dwCursorPosition,
                               &wrote);
}

void
wrefresh (Window *      w)
{
    if (w) {
        /*unreferenced formal parameter */
    }
    /* Do nothing - all changes are immediate */
}

void
redrawwin (Window *     w)
{
    if (w) {
        /*unreferenced formal parameter */
    }
    /* Do nothing - all changes are immediate */
}

int
wgetch (Window *        w)
{
    INPUT_RECORD in;
    DWORD got;
    if (w) {
        /*unreferenced formal parameter */
    }
    for(;;) {
        do {
            ReadConsoleInput(_conin, &in, 1, &got);
        } while (KEY_EVENT != in.EventType || 0 == in.Event.KeyEvent.bKeyDown);
        /* Translate direction keys into vi(1) motion */
        switch (in.Event.KeyEvent.wVirtualKeyCode) {
            case VK_LEFT:   return VKEY_LEFT;    // 'h'
            case VK_RIGHT:  return VKEY_RIGHT;   // 'l'
            case VK_UP:     return VKEY_UP;      // 'k'
            case VK_DOWN:   return VKEY_DOWN;    // 'j'
            case VK_INSERT: return VKEY_INSERT;
            case VK_DELETE: return VKEY_DELETE;
            case VK_BACK:   return VKEY_BACK;

            /* Ignore standard modifier keys */
            case VK_SHIFT:
            case VK_CONTROL:
            case VK_MENU:
                    continue;
        }
        return (unsigned char)in.Event.KeyEvent.uChar.AsciiChar;
    }
}

void
beep (void)
{
    Beep(650, 250);
}

void
noecho (void)
{
    /* Do nothing */
}

void
raw (void)
{
    SetConsoleMode(_conin, ENABLE_PROCESSED_INPUT);
}

void
cbreak (void)
{
    SetConsoleMode(_conin, ENABLE_PROCESSED_INPUT);
}

int
initscr (void)
{
    _conin = GetStdHandle(STD_INPUT_HANDLE);
    _conout = GetStdHandle(STD_OUTPUT_HANDLE);
#if defined (UNICODE)
    SetConsoleTitle(L"Sudoku");
#else
    SetConsoleTitle("Sudoku");
#endif
    return 1;
}

void
endwin (void)
{
    CloseHandle(_conin);
    CloseHandle(_conout);
}
