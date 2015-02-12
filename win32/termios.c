/* Dumb termios implementation for Win32 port
 */

#include "termios.h"

int
tcgetattr (int fd, struct termios * t)
{
    if (fd || t) {
        /*unreferenced formal parameter */
    }
    return 0;
}

int
tcsetattr (int fd, int mode, struct termios * t)
{
    if (fd || mode || t) {
        /*unreferenced formal parameter */
    }
    return 0;
}

