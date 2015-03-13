/* sudoku.c - sudoku game
 *
 * Writing a fun Su-Do-Ku game has turned out to be a difficult exercise.
 * The biggest difficulty is keeping the game fun - and this means allowing
 * the user to make mistakes. The game is not much fun if it prevents the
 * user from making moves, or if it informs them of an incorrect move.
 * With movement constraints, the 'game' is little more than an automated
 * solver (and no fun at all).
 *
 * Another challenge is generating good puzzles that are entertaining to
 * solve. It is certainly true that there is an art to creating good
 * Su-Do-Ku puzzles, and that good hand generated puzzles are more
 * entertaining than many computer generated puzzles - I just hope that
 * the algorithm implemented here provides fun puzzles. It is an area
 * that needs work. The puzzle classification is very simple, and could
 * also do with work. Finally, understanding the automatically generated
 * hints is sometimes more work than solving the puzzle - a better, and
 * more human friendly, mechanism is needed.
 *
 * Comments, suggestions, and contributions are always welcome - send email
 * to: mike 'at' laurasia.com.au. Note that this code assumes a single
 * threaded process, makes extensive use of global variables, and has
 * not been written to be reused in other applications. The code makes no
 * use of dynamic memory allocation, and hence, requires no heap. It should
 * also run with minimal stack space.
 *
 * This code and accompanying files have been placed into the public domain
 * by Michael Kennett, July 2005. It is provided without any warranty
 * whatsoever, and in no event shall Michael Kennett be liable for
 * any damages of any kind, however caused, arising from this software.
 */

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <curses.h>
#include <stddef.h>

/* Default file locations */
#if !defined (TEMPLATE)
#define TEMPLATE "/usr/share/sudoku/template"
#endif
#if !defined (PRECANNED)
#define PRECANNED "/usr/share/sudoku/precanned"
#endif
#define TEMPLATE_FALLBACK "template"
#define PRECANNED_FALLBACK "precanned"
#define DEFAULT_BOARD_NAME "board"

#if defined (_MSC_VER)
#pragma warning(disable: 4996)
#endif

#define BC0 0x00
#define BC1 0x01
#define BC2 0x04
#define BC3 0x05
#define BC4 0x08
#define BC5 0x09
#define BC6 0x0B
#define BC7 0x10
#define BC8 0x11
#define BC9 0x14
#define BCa 0x15
#define BCb 0x18
#define BCc 0x19
#define BCd 0x1B
#define BCe 0x40
#define BCf 0x41
#define BCg 0x44
#define BCh 0x45
#define BCi 0x48
#define BCj 0x49
#define BCk 0x4B
#define BCl 0x50
#define BCm 0x51
#define BCn 0x54
#define BCo 0x55
#define BCp 0x58
#define BCq 0x59
#define BCr 0x5B
#define BCs 0x80
#define BCt 0x81
#define BCu 0x84
#define BCv 0x85
#define BCw 0x90
#define BCx 0x91
#define BCy 0x94
#define BCz 0x95

/* Default template */
#define HC(a) BC##a
#define TB(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w)    \
            HC(a),HC(b),HC(c),HC(d),HC(e),HC(f),HC(g),HC(h), \
            HC(i),HC(j),HC(k),HC(l),HC(m),HC(n),HC(o),HC(p), \
            HC(q),HC(r),HC(s),HC(t),HC(u),HC(v),HC(w)

#define T2(a1,b1,c1,d1,e1,f1,g1,h1,i1,j1,k1,       \
           l1,m1,n1,o1,p1,q1,r1,s1,t1,u1,v1,w1,    \
           a2,b2,c2,d2,e2,f2,g2,h2,i2,j2,k2,       \
           l2,m2,n2,o2,p2,q2,r2,s2,t2,u2,v2,w2)    \
          TB(a1,b1,c1,d1,e1,f1,g1,h1,i1,j1,k1,     \
             l1,m1,n1,o1,p1,q1,r1,s1,t1,u1,v1,w1), \
          TB(a2,b2,c2,d2,e2,f2,g2,h2,i2,j2,k2,     \
             l2,m2,n2,o2,p2,q2,r2,s2,t2,u2,v2,w2)

static const char dtct[4] = {
    '.',    /* 0 */
    '*',    /* 1 */
    '\n',   /* 2 */
    '%'     /* 3 */
};

static const unsigned char default_template [] = {
 T2(d,h,x,l,9,i,e,w,1,m,i,0,u,9,1,c,2,u,l,9,b,h,x, d,1,y,e,1,q,8,w,e,e,c,h,x,3,2,b,7,z,1,3,p,0,x),
 T2(r,0,x,n,e,c,9,u,e,n,4,8,t,n,2,i,l,w,3,n,b,1,y, r,e,w,0,h,p,3,u,7,8,c,g,w,8,8,i,e,z,g,1,b,2,y),
 T2(r,e,y,3,3,5,7,t,2,n,4,h,t,n,e,4,8,s,f,f,q,2,y, 6,g,v,3,g,b,3,t,h,9,5,0,s,m,g,5,f,x,g,e,j,h,s),
 T2(6,m,t,h,2,i,8,t,3,8,j,0,u,8,f,5,8,v,e,g,5,a,t, 6,h,t,n,e,b,7,y,1,f,5,h,t,3,1,q,7,w,2,n,4,h,t),
 T2(k,l,s,3,8,5,3,v,9,g,5,g,s,h,l,i,f,t,8,f,5,9,u, d,3,t,g,7,j,g,u,7,a,p,0,y,l,8,i,g,u,8,g,4,f,x),
 T2(d,1,t,9,a,i,g,u,h,e,4,n,s,2,g,j,g,u,l,m,4,1,x, d,7,y,h,e,4,h,t,7,m,b,g,w,9,8,4,h,t,2,g,q,7,w),
 T2(6,h,u,m,8,i,0,y,f,g,j,7,u,h,2,q,0,u,7,a,j,g,t, k,g,u,f,2,j,7,u,7,7,p,0,y,7,7,i,7,u,f,2,j,g,u),
 T2(k,g,u,1,0,5,h,t,g,h,i,0,u,g,h,4,h,t,1,0,j,g,u, k,n,u,1,0,5,1,t,h,g,5,7,s,h,g,5,1,t,1,0,j,n,u),
 T2(k,0,u,9,m,b,g,w,l,9,4,0,s,l,9,b,g,w,9,m,i,0,u, 6,e,u,3,9,i,2,t,2,h,b,0,w,g,f,4,f,u,l,e,j,2,s),
 T2(k,g,u,1,0,5,8,t,9,m,i,7,u,9,m,4,8,t,1,0,j,g,u, k,1,v,e,2,p,7,y,l,9,b,0,w,l,9,p,7,y,e,2,i,1,v),
 T2(r,n,s,0,0,j,3,t,1,1,5,1,t,1,1,5,f,v,1,0,4,n,y, 6,0,s,g,g,i,h,v,f,2,5,h,t,f,2,j,h,v,g,g,4,0,s),
 T2(6,9,w,8,8,4,e,w,n,8,4,1,t,7,o,b,2,s,7,8,c,l,s, k,9,s,g,e,5,3,t,0,l,c,7,w,a,0,4,f,t,3,g,4,l,u),
 T2(d,g,w,7,2,4,7,u,3,1,5,h,t,1,f,j,7,s,e,7,b,g,w, k,2,s,7,1,5,m,v,7,0,j,8,v,1,7,i,a,t,1,8,4,e,u),
 T2(6,e,s,2,1,4,l,z,9,f,5,h,t,3,m,p,a,s,0,f,4,2,s, d,1,x,8,8,5,g,s,l,9,4,0,s,l,9,4,g,s,8,8,c,1,x),
 T2(6,0,v,m,1,4,9,w,9,e,c,0,w,3,l,b,l,s,0,a,j,1,s, 6,n,s,f,2,5,1,t,8,7,j,0,u,8,7,5,1,t,f,2,5,n,s),
 T2(k,0,u,1,1,c,g,w,l,9,4,7,s,l,9,b,g,w,1,1,j,0,u, r,0,y,2,f,4,h,t,e,2,b,0,w,e,2,4,h,t,2,f,p,0,y),
 T2(k,8,s,1,2,q,0,t,m,0,4,1,t,0,9,5,1,y,f,0,5,7,v, 6,3,s,8,0,i,7,y,1,n,4,8,t,n,0,q,7,u,0,7,5,e,t),
 T2(6,l,u,2,1,j,2,w,9,e,4,g,s,2,l,b,e,u,1,f,i,9,s, d,3,u,7,2,5,f,t,0,m,4,0,s,9,1,4,3,t,f,7,i,e,x),
 T2(6,2,x,7,1,4,8,y,e,f,4,g,s,2,3,p,7,t,0,8,b,f,s, d,g,w,0,0,i,1,v,g,g,i,7,u,g,g,i,1,v,0,0,b,g,w),
 T2(k,7,x,e,g,5,0,s,9,g,4,g,s,g,l,4,0,s,h,2,b,8,y, 6,e,s,8,f,5,a,s,8,0,j,0,u,1,7,5,l,t,3,8,5,2,s),
 T2(6,e,t,1,0,4,a,v,0,f,c,0,w,3,1,i,m,t,0,0,5,3,s, k,n,u,1,0,5,1,t,g,g,4,0,s,g,g,4,1,t,1,0,j,n,u),
 T2(r,e,w,g,e,5,7,s,1,e,4,8,t,2,0,5,7,s,3,g,b,2,y, r,e,w,g,e,5,7,s,1,e,4,8,t,2,0,5,7,s,3,g,b,2,y),
 T2(6,1,t,n,n,p,0,y,e,3,4,7,s,e,3,p,0,y,n,n,4,1,t, 6,3,t,2,m,i,0,w,1,g,4,7,s,g,0,c,0,u,9,f,4,f,t),
 T2(r,0,s,3,1,i,g,s,f,7,b,1,x,7,2,5,g,u,0,f,5,0,y, d,e,x,1,7,i,2,u,e,3,4,1,t,e,3,i,e,u,7,0,c,3,w),
 T2(6,f,t,g,f,b,3,x,1,m,i,g,u,9,1,c,f,x,2,h,4,3,t, d,7,w,2,f,b,g,w,8,7,5,h,t,8,7,c,g,w,2,f,b,7,w),
 T2(k,g,t,3,e,4,9,u,0,a,4,0,s,l,1,i,l,s,2,e,5,h,u, 6,8,u,0,2,b,g,u,h,7,i,0,u,7,g,j,g,w,e,0,i,7,t),
 T2(6,f,w,e,1,4,2,v,9,l,4,7,s,9,l,i,f,s,0,3,b,2,t, 6,g,w,h,7,i,2,s,0,7,q,0,y,8,0,4,e,u,7,g,c,g,s),
 T2(6,7,t,m,7,b,3,w,7,0,4,n,s,0,7,b,e,x,7,9,5,8,s, d,0,w,9,l,4,n,s,l,a,p,0,y,l,a,4,n,s,9,l,b,0,w),
 T2(6,2,s,7,8,4,3,y,8,2,c,0,w,f,7,q,e,t,7,8,4,e,s, k,g,u,7,7,4,g,u,2,0,5,n,s,1,e,i,g,s,7,7,i,g,u),
 T2(6,7,t,8,0,4,e,z,0,e,j,g,u,3,0,p,3,s,0,7,5,8,s, 6,g,x,0,1,i,7,w,l,8,b,0,w,7,a,b,7,u,0,1,b,h,s),
 T2(k,7,v,2,h,i,3,t,3,1,p,g,y,0,f,5,f,v,g,f,i,8,u, d,8,s,e,e,i,7,t,m,9,i,0,u,l,9,5,8,u,2,2,4,7,x),
 T2(6,e,x,h,1,j,0,s,8,9,b,0,w,l,7,5,0,u,1,h,c,3,s, 6,7,v,m,0,b,e,u,7,f,b,g,w,2,8,i,2,w,0,9,j,8,s),
 T2(k,h,w,9,0,c,2,s,3,2,i,1,v,e,e,5,e,w,1,l,b,g,v, 6,e,x,2,1,p,e,s,7,3,5,g,s,f,8,4,2,y,0,f,b,3,s),
 T2(d,e,s,1,m,b,f,s,a,7,b,0,w,7,l,5,2,x,9,1,5,2,w, d,8,w,e,7,4,f,u,7,e,4,h,t,2,7,i,2,t,7,2,b,7,x),
 T2(k,g,u,7,7,i,g,u,9,l,b,0,w,9,l,i,g,u,7,7,i,g,u, 6,0,t,l,0,5,2,y,3,7,b,n,w,7,e,q,e,s,1,9,4,1,s),
 T2(d,1,w,2,e,i,2,u,e,a,5,7,s,m,3,i,e,u,2,e,b,0,x, r,2,y,h,9,4,f,w,1,l,b,g,w,9,0,c,2,t,l,g,q,e,y),
 T2(6,1,t,0,8,c,9,u,e,0,5,h,t,1,2,i,l,w,8,1,4,1,t, d,7,u,e,7,b,f,t,7,0,c,h,x,1,7,4,3,x,7,2,i,7,w),
 T2(d,n,w,2,9,4,7,s,8,e,i,1,v,2,7,5,7,s,l,e,b,n,w, k,h,w,l,3,i,2,s,7,l,5,0,s,a,7,4,e,u,e,a,b,g,v),
 T2(6,0,t,2,l,5,2,y,e,7,c,7,w,8,2,p,e,s,a,e,4,1,s, d,e,v,2,m,b,2,w,0,f,5,1,t,3,1,b,e,w,9,f,i,3,w),
 T2(d,e,t,e,8,i,7,t,f,3,4,0,s,e,3,5,8,u,7,3,4,3,w, d,7,u,h,0,5,f,s,3,1,i,0,u,0,f,5,2,t,1,g,j,7,w),
 T2(6,9,w,0,7,i,g,w,9,7,4,8,t,7,l,b,g,u,7,0,b,l,s, r,9,s,1,n,4,0,s,1,e,i,n,u,2,0,5,0,s,n,0,5,l,y),
 T2(6,h,t,9,f,b,1,z,f,2,j,7,u,f,2,q,1,x,2,m,4,h,t, d,7,w,2,n,i,9,s,2,7,b,1,x,7,e,4,l,u,n,e,b,7,w),
 T2(6,1,s,2,m,i,f,x,7,1,i,n,u,0,8,b,3,v,9,f,4,0,t, 6,e,z,8,f,b,l,t,g,1,c,1,x,1,h,4,a,w,2,8,q,3,s),
 T2(d,g,s,3,e,j,7,t,g,1,5,1,t,1,h,4,8,u,3,e,5,g,w, k,0,w,l,0,b,m,w,0,g,5,8,t,h,0,b,9,x,0,9,b,0,u),
 T2(k,m,t,2,2,5,9,s,7,l,4,g,s,9,7,4,l,s,f,e,4,a,v, k,3,s,2,e,i,0,w,8,2,j,7,u,f,7,c,0,u,2,e,4,e,v),
 T2(r,l,w,e,7,5,7,s,g,e,4,1,t,2,g,4,7,s,8,2,b,9,y, d,0,w,f,2,5,o,t,g,g,i,0,u,g,g,4,o,t,f,2,c,0,w),
 T2(d,7,u,8,n,4,0,u,1,2,4,n,s,e,0,j,0,s,n,7,j,7,w, r,1,y,e,f,4,e,s,9,7,4,g,s,7,l,4,2,s,2,3,p,0,z),
 T2(k,3,u,0,0,j,3,s,7,f,b,1,x,2,8,4,e,v,1,0,i,e,v, r,0,x,f,7,4,3,s,0,1,c,h,x,1,1,4,e,t,7,2,c,1,y),
 T2(k,0,u,8,8,5,h,t,2,e,4,g,s,2,e,4,h,t,8,8,j,0,u, k,e,u,m,2,c,f,x,1,3,i,1,v,e,1,c,3,x,f,9,j,2,u),
 T2(d,1,x,g,g,b,g,w,l,9,4,0,s,l,9,b,g,w,g,g,b,1,x, d,2,u,0,1,c,e,u,8,8,4,0,s,7,8,j,2,w,1,1,i,e,w),
 T2(r,f,w,n,l,b,7,t,0,o,4,g,s,n,1,4,8,w,9,n,b,2,z, d,7,z,0,2,5,m,t,1,a,4,0,s,l,1,5,a,t,f,0,p,8,w),
 T2(k,7,w,e,e,5,1,t,e,1,c,7,w,1,3,4,1,t,3,2,b,7,u, 6,2,s,1,h,i,8,z,2,l,4,g,s,9,e,p,8,v,g,1,5,e,s),
 T2(6,e,t,e,8,p,1,t,f,g,i,0,u,g,2,5,1,z,7,3,4,3,s, d,0,w,e,2,i,h,v,2,f,b,0,w,2,f,i,h,v,e,2,b,0,w),
 T2(d,7,w,8,8,5,h,t,1,0,5,g,s,1,0,5,h,t,8,8,c,7,w, r,e,s,1,1,i,e,t,2,1,c,8,x,1,f,4,3,u,0,1,5,2,y),
 T2(k,9,x,1,0,4,f,s,0,3,q,0,y,f,1,4,2,t,0,0,c,m,u, r,7,x,7,0,c,e,u,l,2,b,7,w,e,9,i,2,w,1,7,b,8,y),
 T2(6,0,s,e,f,4,o,y,7,n,4,0,s,n,7,p,n,t,2,3,4,0,s, 6,l,w,l,0,b,0,x,8,e,4,1,t,2,7,c,1,w,0,9,b,9,s),
 T2(k,a,s,2,n,j,g,w,9,7,c,7,w,8,l,b,g,u,o,e,4,l,v, d,0,x,l,2,b,e,s,7,e,i,o,v,2,7,4,2,w,e,9,b,1,w),
 T2(r,9,s,l,1,c,e,v,8,g,i,g,u,g,7,j,3,w,1,a,4,l,y, 6,n,s,2,f,i,1,v,7,7,b,8,x,7,7,i,1,v,2,f,4,n,s),
 T2(k,2,u,7,1,j,0,s,3,1,5,8,t,1,f,5,0,u,1,8,i,e,u, d,n,w,0,9,i,3,t,h,9,c,0,w,m,g,5,f,v,l,0,b,n,w),
 T2(k,1,t,e,1,c,e,w,l,g,i,7,u,g,9,b,2,w,1,3,4,1,v, d,2,u,e,7,i,e,u,1,f,4,8,t,2,1,j,2,u,7,2,i,e,w),
 T2(6,8,u,0,7,p,a,w,m,7,4,7,s,7,9,c,l,z,7,0,i,7,t, k,3,t,0,8,q,e,w,7,0,j,0,u,1,7,b,2,y,8,1,4,f,v),
 T2(6,3,y,f,0,i,0,s,7,f,i,h,v,2,8,4,0,u,0,2,q,e,t, k,0,u,7,7,p,n,y,g,g,b,0,w,g,g,p,n,y,7,7,i,0,u),
 T2(6,g,s,7,7,p,g,y,2,e,b,1,x,2,e,p,g,y,7,7,4,g,s, d,2,w,e,8,i,e,s,7,e,j,7,u,3,7,4,2,u,7,3,b,e,w),
 T2(k,g,t,2,3,4,1,u,7,2,c,0,w,f,7,i,0,t,e,f,4,h,u, k,7,u,l,9,b,0,w,8,7,5,g,s,8,7,c,0,w,l,9,i,7,u),
 T2(6,e,t,2,1,j,e,w,7,2,p,0,y,e,7,b,2,u,1,f,4,3,s, 6,h,t,8,l,4,l,s,0,a,c,0,w,m,1,4,9,s,9,7,5,h,t),
 T2(6,7,x,2,2,b,1,s,2,h,b,0,w,g,f,4,0,x,e,e,b,8,s, 6,0,s,g,g,p,1,z,l,a,4,0,s,l,a,p,1,z,g,g,4,0,s),
 T2(r,7,u,2,f,j,e,s,l,2,b,0,w,e,9,4,2,u,3,f,i,7,y, d,g,w,g,g,4,1,t,l,9,b,0,w,l,9,4,1,t,g,g,b,g,w),
 T2(d,7,y,f,1,p,1,s,e,7,j,0,u,8,2,4,0,z,0,3,q,7,w, k,7,t,0,f,5,l,x,3,g,4,7,s,g,e,c,a,s,3,1,4,8,u),
 T2(6,7,z,h,f,4,0,t,e,0,q,g,y,1,2,4,1,s,2,h,q,8,s, d,g,w,0,0,p,1,z,l,9,b,0,w,l,9,p,1,z,0,0,b,g,w),
 T2(k,m,s,2,f,j,e,u,1,1,b,1,x,0,1,j,2,u,3,f,4,9,v, 6,8,v,h,e,4,f,s,7,1,b,1,x,0,8,4,2,t,2,g,j,8,t),
 T2(6,3,v,1,7,j,7,w,2,9,4,g,s,l,e,b,7,u,8,0,j,f,t, r,2,s,g,3,5,e,y,f,0,b,0,w,0,2,q,2,s,f,h,4,e,y),
 T2(6,0,s,h,9,5,8,v,0,3,b,1,x,e,1,i,8,t,m,g,5,0,s, 6,g,s,2,e,p,1,z,l,9,b,0,w,l,9,p,1,z,2,e,4,g,s),
 T2(6,2,t,8,f,b,2,t,7,3,j,0,u,f,8,4,f,w,2,8,5,f,s, d,h,x,l,9,i,0,u,e,2,b,0,w,e,2,i,0,u,l,9,b,h,x),
 T2(k,7,w,l,2,5,n,s,2,7,p,0,y,7,e,4,n,s,f,9,b,7,u, d,0,w,e,0,q,8,t,f,g,i,0,u,g,2,5,8,z,1,2,b,0,w),
 T2(d,8,v,9,0,4,2,w,m,0,b,g,w,0,9,c,e,s,0,l,i,8,x, k,1,u,1,g,b,g,t,2,8,5,0,s,8,f,4,h,w,g,0,j,0,v),
 T2(6,0,y,7,2,b,h,s,3,8,5,g,s,8,f,5,g,x,e,7,p,0,s, k,1,u,h,1,4,e,x,0,l,j,0,u,a,0,b,3,s,0,h,j,0,v),
 T2(d,h,x,7,7,i,0,u,l,9,b,0,w,l,9,i,0,u,7,7,b,h,x, 6,2,u,0,l,b,9,x,1,f,p,0,y,2,1,c,m,w,9,0,i,e,s),
 T2(6,1,t,l,9,b,7,w,n,n,4,0,s,n,n,b,7,w,l,9,4,1,t, 6,1,y,8,9,4,8,s,e,8,b,g,w,7,3,4,7,t,l,7,q,0,t),
 T2(k,g,s,3,9,4,7,s,3,7,b,0,w,7,e,5,7,s,l,e,5,g,u, d,1,x,n,n,b,0,w,e,2,i,7,u,e,2,b,0,w,n,n,b,1,x),
 T2(r,2,s,7,8,4,2,w,2,g,b,g,w,g,e,b,e,s,7,8,4,e,y, k,g,u,l,9,b,0,w,7,8,b,0,w,7,8,b,0,w,l,9,i,g,u),
 T2(6,a,t,7,0,c,2,v,9,3,4,0,s,e,m,i,f,w,1,7,4,m,t, d,7,w,3,e,5,g,s,2,f,5,1,t,3,f,4,g,s,3,e,c,7,w),
 T2(6,1,t,e,2,p,0,y,l,9,p,0,y,l,9,p,0,y,e,2,4,1,t, 6,9,z,0,f,5,1,t,f,e,4,1,t,2,2,5,1,t,3,1,p,m,s),
 T2(6,0,z,8,g,i,7,t,1,f,i,0,u,2,1,5,8,u,g,7,q,1,s, k,h,v,0,0,p,0,y,l,9,i,7,u,l,9,p,0,y,0,0,i,h,v),
 T2(r,g,y,g,g,b,0,w,e,2,i,0,u,e,2,b,0,w,g,g,p,g,y, r,g,u,8,f,4,7,u,e,9,4,0,s,l,2,i,7,s,2,8,j,g,y),
 T2(6,1,v,3,e,4,g,t,7,9,5,g,s,m,7,4,h,s,2,e,j,1,t, k,g,w,0,8,5,7,v,2,e,5,g,s,3,e,i,8,s,8,1,b,g,u),
 T2(k,2,t,f,0,5,m,w,7,3,5,0,s,f,8,b,9,t,1,2,5,f,u, k,3,t,2,8,i,g,s,e,e,b,g,w,2,2,4,g,u,7,f,4,f,v),
 T2(d,g,s,7,7,4,2,x,7,n,b,7,w,n,7,b,f,s,7,7,4,g,w, 6,g,x,l,e,p,1,w,0,1,p,0,y,0,1,b,0,z,2,9,b,h,s),
 T2(d,0,w,g,h,i,0,u,l,9,i,0,u,l,9,i,0,u,g,h,b,0,w, d,h,x,7,7,p,0,y,e,2,i,0,u,e,2,p,0,y,7,7,b,h,x),
 T2(k,3,w,0,h,j,1,s,7,1,j,0,u,1,8,4,0,v,h,1,b,e,v, d,0,y,h,2,c,7,s,2,2,b,1,x,e,e,4,7,w,f,g,q,0,w),
 T2(k,0,u,n,n,b,1,x,e,3,4,7,s,e,3,b,1,x,n,n,i,0,u, 6,8,u,9,1,b,f,s,9,0,j,g,u,1,l,4,2,x,0,m,i,7,t),
 T2(6,7,z,7,0,j,e,u,e,7,p,g,y,7,2,i,2,u,1,7,p,8,s, d,3,u,0,1,4,l,w,8,9,b,1,x,l,7,c,9,s,0,1,i,e,x),
 T2(k,2,v,e,f,b,0,s,n,2,j,0,u,f,n,4,0,w,2,3,i,f,u, d,2,y,2,1,p,e,u,8,3,4,0,s,e,8,j,2,y,0,f,p,e,w),
 T2(d,1,x,g,g,b,0,w,l,9,i,0,u,l,9,b,0,w,g,g,b,1,x, 6,g,x,h,0,5,e,s,n,e,5,0,s,3,n,4,2,s,1,g,c,h,s),
 T2(r,l,t,g,7,q,0,s,e,0,5,g,s,1,2,4,0,y,8,g,4,a,y, 6,h,s,2,l,p,e,t,m,e,4,7,s,2,9,5,3,y,9,e,4,g,t),
 T2(6,2,y,3,2,b,7,y,2,8,b,0,w,7,f,p,7,w,e,e,q,e,s, k,a,y,7,0,p,2,t,2,8,4,0,s,7,f,4,f,y,0,7,p,l,v),
 T2(k,g,u,9,l,b,0,w,l,a,4,0,s,l,a,b,0,w,9,l,i,g,u, k,g,v,0,2,c,0,x,g,l,4,7,s,9,g,b,1,w,f,0,i,h,u),
 T2(6,e,w,3,8,4,g,w,e,8,b,1,x,7,3,b,g,s,7,f,c,2,s, d,0,v,1,h,i,0,s,e,9,i,h,v,l,2,4,0,u,g,1,j,1,w),
 T2(k,g,u,l,9,i,1,v,e,2,b,0,w,e,2,i,1,v,l,9,i,g,u, 6,2,t,3,e,4,f,u,n,f,b,0,w,2,o,i,2,t,2,e,5,f,s),
 T2(d,e,y,e,0,c,2,u,e,h,i,0,u,g,3,i,e,w,1,2,p,2,w, 6,3,t,3,3,b,2,u,1,1,5,1,t,1,1,j,e,w,e,f,5,f,t),
 T2(k,2,u,e,e,5,h,v,9,0,b,0,w,0,l,i,h,t,3,2,i,e,u, 6,a,x,m,0,j,0,s,8,2,4,1,t,e,7,5,0,u,1,9,c,m,t),
 T2(k,e,s,f,m,b,1,u,9,1,4,g,s,0,m,i,0,x,9,3,5,2,u, 6,2,y,g,e,4,f,t,1,3,i,1,v,e,1,5,3,t,2,g,p,e,s),
 T2(d,g,t,l,g,4,0,t,2,3,4,h,t,e,f,4,1,s,g,9,4,h,w, k,8,t,1,0,c,8,s,m,7,i,0,u,7,9,5,7,x,1,0,5,8,v),
 T2(6,e,v,m,7,4,7,s,1,e,q,g,y,3,0,5,7,s,7,9,j,3,s, d,e,u,e,f,i,2,w,9,1,b,g,w,0,m,b,e,u,2,3,i,2,w),
 T2(d,f,u,0,3,c,7,t,7,e,4,h,t,2,7,4,8,w,f,1,i,2,x, d,2,s,8,3,j,3,s,0,l,b,1,x,9,0,4,e,v,f,8,5,e,w),
 T2(d,9,w,2,l,i,l,w,0,2,5,1,t,f,0,b,9,u,9,e,b,l,w, 6,1,t,g,g,p,0,y,l,9,b,0,w,l,9,p,0,y,g,g,4,1,t),
 T2(d,h,s,1,9,i,e,u,2,1,j,0,u,1,f,i,2,u,l,0,5,g,x, 6,1,y,9,3,b,2,s,m,2,i,0,u,e,9,5,e,w,e,m,p,0,t),
 T2(r,g,y,0,1,i,0,u,l,9,b,0,w,l,9,i,0,u,0,1,p,g,y, d,2,s,3,1,4,1,v,g,2,c,g,w,f,g,i,1,t,0,f,5,e,w),
 T2(k,2,s,3,7,5,8,w,8,3,i,0,u,e,8,c,7,t,8,e,5,e,u, d,0,w,9,m,b,1,x,l,9,4,0,s,l,9,b,1,x,9,m,b,0,w),
 T2(k,3,s,1,1,5,7,w,8,g,p,0,y,g,7,c,7,s,1,1,5,e,v, r,0,y,e,2,b,1,x,l,9,i,0,u,l,9,b,1,x,e,2,p,0,y),
 T2(r,e,t,f,1,5,e,t,7,1,j,0,u,1,8,4,3,s,1,3,5,3,y, d,3,y,0,3,4,e,y,7,e,5,0,s,3,7,p,2,s,e,1,p,e,x),
 T2(d,8,x,0,0,4,2,z,e,1,c,g,w,1,3,p,f,s,0,0,b,8,x, 6,9,y,3,3,5,0,s,2,g,i,1,v,g,e,4,0,s,f,f,q,l,s),
 T2(r,0,y,l,9,b,0,w,l,9,b,0,w,l,9,b,0,w,l,9,p,0,y, k,e,v,7,e,5,7,x,e,1,j,0,u,1,3,b,8,s,3,7,i,3,u),
 T2(k,n,s,e,0,j,0,w,1,e,j,h,v,3,0,c,0,u,1,2,4,n,u, d,3,u,e,8,b,e,s,1,e,c,1,x,3,0,5,2,w,7,3,i,e,x),
 T2(d,9,u,9,2,i,7,s,7,o,4,7,s,n,8,4,7,u,e,l,i,l,w, 6,9,w,1,l,5,1,u,e,1,j,g,u,1,3,i,0,t,a,0,c,l,s),
 T2(6,1,u,3,e,5,3,v,1,h,4,7,s,g,1,j,f,t,3,e,j,0,t, k,g,u,e,2,b,1,x,l,9,b,0,w,l,9,b,1,x,e,2,i,g,u),
 T2(6,9,u,e,h,b,e,t,1,7,5,g,s,8,0,5,3,w,g,3,i,l,s, d,0,v,9,2,4,f,t,1,f,i,1,v,2,1,5,3,t,e,l,i,1,w),
 T2(6,f,w,1,1,4,1,v,f,1,c,g,w,1,3,j,1,t,0,1,c,2,t, k,7,x,1,3,4,2,w,e,o,4,7,s,n,3,b,e,s,e,1,c,8,u),
 T2(6,e,w,8,9,i,0,y,7,1,5,g,s,1,8,p,0,u,l,7,c,2,s, 6,7,t,2,g,c,7,s,7,g,c,0,w,h,7,4,7,w,h,e,4,8,s),
 T2(d,7,t,1,9,5,2,t,l,9,5,0,s,m,9,4,f,s,m,0,5,8,w, k,7,u,e,g,4,7,y,1,n,i,g,u,n,0,q,7,s,g,2,i,7,u),
 T2(k,8,w,2,7,b,m,s,l,7,4,g,s,7,9,4,9,x,7,e,b,7,v, 6,e,y,3,0,4,e,x,3,h,i,0,u,g,f,c,3,s,0,e,q,2,s),
 T2(k,2,v,1,8,c,7,s,9,g,4,7,s,g,l,4,7,w,8,1,j,f,u, 6,m,s,l,7,4,2,y,l,7,j,0,u,8,9,p,e,s,7,9,4,9,t),
 T2(d,2,s,g,3,5,m,u,1,2,4,1,t,e,0,j,9,t,f,h,4,e,w, r,0,y,l,7,b,g,s,1,m,4,7,s,9,1,5,g,w,7,9,p,0,y),
 T2(6,h,t,g,g,b,0,w,l,a,4,0,s,l,a,b,0,w,g,g,4,h,t, k,l,u,1,0,4,l,y,8,8,4,g,s,7,8,q,9,s,0,0,j,9,u),
 T2(r,e,s,e,e,c,a,w,g,1,4,0,s,0,h,b,l,x,3,2,4,2,y, d,l,x,3,e,4,l,u,2,7,4,g,s,7,e,i,9,s,2,e,c,a,w),
 T2(k,0,u,7,8,p,0,y,l,a,4,0,s,l,a,p,0,y,7,8,i,0,u, 6,g,s,2,e,b,1,x,l,a,b,0,w,l,a,b,1,x,2,e,4,g,s),
 T2(6,1,t,0,g,q,9,s,0,e,q,1,z,3,0,4,l,y,h,0,4,1,t, d,7,v,g,8,4,e,s,9,7,c,0,w,8,l,4,2,s,7,h,i,8,w),
 T2(k,0,w,e,3,c,e,v,2,2,4,1,t,e,e,i,3,w,f,3,b,0,u, 6,7,t,3,e,i,3,t,l,l,4,g,s,9,9,4,f,v,2,e,5,8,s),
 T2(k,2,w,e,e,q,2,s,g,8,5,0,s,8,h,4,e,y,3,2,b,e,u, d,m,w,g,0,j,2,u,1,e,4,h,t,2,0,j,e,u,1,g,b,9,t),
 T2(k,e,y,7,9,4,0,w,h,7,b,g,w,7,g,c,0,s,l,7,p,2,u, 6,g,s,n,n,i,1,v,e,2,b,0,w,e,2,i,1,v,n,n,4,g,s),
 T2(d,m,w,7,a,4,0,s,7,f,b,1,x,2,8,4,0,s,l,8,b,9,x, d,9,s,1,m,i,e,s,1,l,5,g,s,a,0,5,2,u,9,1,5,l,w),
 TB(d,0,u,f,9,4,e,y,a,2,4,1,t,e,l,q,2,s,l,2,j,0,w)
};

static const char * program;        /* argv[0] */

static const char * requested_class = NULL;
static time_t start_time;

/* Common state encoding in a 32-bit integer:
 *   bits  0-6    index
 *         7-15   state  [bit high signals digits not possible]
 *        16-19   digit
 *           20   fixed  [set if digit initially fixed]
 *           21   choice [set if solver chose this digit]
 *           22   ignore [set if ignored by reapply()]
 *           23   unused
 *        24-26   hint
 *        27-31   unused
 */
#define INDEX_MASK              0x0000007f
#define GET_INDEX(val)          (INDEX_MASK&(val))
#define SET_INDEX(val)          (val)

#define STATE_MASK              0x0000ff80
#define STATE_SHIFT             (7-1)                        /* digits 1..9 */
#define DIGIT_STATE(digit)      (1<<(STATE_SHIFT+(digit)))

#define DIGIT_MASK              0x000f0000
#define DIGIT_SHIFT             16
#define GET_DIGIT(val)          (((val)&DIGIT_MASK)>>(DIGIT_SHIFT))
#define SET_DIGIT(val)          ((val)<<(DIGIT_SHIFT))

#define FIXED                   0x00100000
#define CHOICE                  0x00200000
#define IGNORED                 0x00400000

/* Hint codes (c.f. singles(), pairs(), findmoves()) */
#define HINT_ROW                0x01000000
#define HINT_COLUMN             0x02000000
#define HINT_BLOCK              0x04000000

/* For a general board it may be necessary to do backtracking (i.e. to
 * rewind the board to an earlier state), and make choices during the
 * solution process. This can be implemented naturally using recursion,
 * but it is more efficient to maintain a single board.
 */
static int board[81];

/* Addressing board elements: linear array 0..80 */
#define ROW(idx)                ((idx)/9)
#define COLUMN(idx)             ((idx)%9)
#define BLOCK(idx)              (3*(ROW(idx)/3)+(COLUMN(idx)/3))
#define INDEX(row,col)          (9*(row)+(col))

/* Blocks indexed 0..9 */
#define IDX_BLOCK(row,col)      (3*((row)/3)+((col)/3))
#define TOP_LEFT(block)         (INDEX(block/3,block%3))

/* Board state */
#define STATE(idx)              ((board[idx])&STATE_MASK)
#define DIGIT(idx)              (GET_DIGIT(board[idx]))
#define HINT(idx)               ((board[idx])&HINT_MASK)
#define IS_EMPTY(idx)           (0 == DIGIT(idx))
#define DISALLOWED(idx,digit)   ((board[idx])&DIGIT_STATE(digit))
#define IS_FIXED(idx)           (board[idx]&FIXED)

/* Record move history, and maintain a counter for the current
 * move number. Concessions are made for the user interface, and
 * allow digit 0 to indicate clearing a square. The move history
 * is used to support 'undo's for the user interface, and hence
 * is larger than required - there is sufficient space to solve
 * the puzzle, undo every move, and then redo the puzzle - and
 * if the user requires more space, then the full history will be
 * lost.
 */
static int idx_history;
static int history[3 * 81];

/* Possible moves for a given board (c.f. fillmoves()).
 * Also used by choice() when the deterministic solver has failed,
 * and for calculating user hints. The number of hints is stored
 * in num_hints, or -1 if no hints calculated. The number of hints
 * requested by the user since their last move is stored in req_hints;
 * if the user keeps requesting hints, start giving more information.
 * Finally, record the last hint issued to the user; attempt to give
 * different hints each time.
 */
static int idx_possible;
static int possible[81];
static int num_hints;
static int req_hints;
static int last_hint;

static int pass;    /* count # passes of deterministic solver */

/* Support for template file */
static FILE * ftmplt;
static int n_tmplt;                 /* Number of templates in file */
static int tmplt[81];             /* Template indices */
static int len_tmplt;               /* Number of template indices */

/* Command line options */
static enum opt_format_enum
{
    fStandard,
    fCompact,
    fCSV,
    fPostScript,
    fHTML
} opt_format = fStandard;

static int opt_describe = 0;
static int opt_generate = 0;
static int num_generate = 1;  /* Number boards to generate w/ -g */
static int opt_random = 1;
static int opt_statistics = 0;
static int opt_spoilerhint = 0;
static int opt_solve = 0;
static int opt_restrict = 0;

static FILE default_template_file;  /* We will use only the address of it. */
static int default_template_siz = 0;
static int default_template_pos = 0;

/* Special modified versions of template-file handling functions */
static
FILE *
topen (const char *     filename,
       const char *     mode)
{
    FILE * stream = 0;
    if (mode != 0
        && strcmp(mode, "r") == 0) {
        stream = fopen(filename, "r");
        if (0 == stream && strcmp(TEMPLATE_FALLBACK, filename) == 0) {
            stream = &default_template_file;
            default_template_siz = sizeof(default_template) * 4;
            default_template_pos = 0;
        }
    }
    return stream;
}

static
int
tclose (FILE *  stream)
{
    int ret = -1;
    if (stream == &default_template_file) {
        default_template_siz = 0;
        default_template_pos = 0;
        ret = 0;
    } else if (0 != stream) {
        ret = fclose(stream);
    }
    return ret;
}

static
int
tseek (FILE *   stream,
       long     offset,
       int      origin)
{
    int ret = -1;
    if (stream != &default_template_file) {
        ret = fseek(stream,
                    offset,
                    origin);
    } else {
        switch (origin) {
            case SEEK_CUR:
                if (default_template_siz > default_template_pos + offset
                    && default_template_pos + offset >= 0) {
                    default_template_pos = offset;
                    ret = 0;
                }
                break;
            case SEEK_END:
                if (default_template_siz - offset >= 0 && offset <= 0) {
                    default_template_pos = offset;
                    ret = 0;
                }
                break;
            case SEEK_SET:
                if (default_template_siz > offset && offset >= 0) {
                    default_template_pos = offset;
                    ret = 0;
                }
                break;
            default:
                /* Invalid origin */
                break;
        }
    }
    return ret;
}

static
char *
tgets (char *   str,
       int      n,
       FILE *    stream)
{
    char * ret = 0;
    if (stream == &default_template_file) {
        int i = 0;
        if (str != 0
            && n > 0
            && default_template_siz > default_template_pos
            && default_template_pos >= 0) {
            char ch = 0;
            while (default_template_pos < default_template_siz
                   && n > i) {
                unsigned char ci = default_template[default_template_pos >> 2];
                int j = (default_template_pos++ & 3);
                if (j > 0) {
                    ci >>= 2 * j;
                }
                ch = dtct[ci & 3];
                str[i++] = ch;
                if (ch == '\n') {
                    break;
                }
            }
            if (i < n) {
                str[i++] = 0;
                ret = str;
            } else if (i == n && 0 < n) {
                str[n - 1] = 0;
                ret = str;
            }
        }
    } else {
        ret = fgets (str,
                     n,
                     stream);
    }
    return ret;
}

/* Write default template
 */
static
int
write_default_template(const char *     template_path)
{
    int     ret = -1;
    FILE *  f   = fopen(template_path, "r");
    if (0 == f) {
        f = fopen(template_path, "w");
        if (0 != f) {
            size_t i;
            for (i = 0; i < sizeof(default_template); ++i) {
                unsigned char ch = default_template[i];
                int j;
                for (j = 0; j < 4; ++j) {
                    fprintf(f, "%c", dtct[ch & 3]);
                    ch >>= 2;
                }
            }
            ret = fclose(f);
        }
    } else {
        fclose(f);
    }
    return ret;
}

/* Reset global state */
static
void
reset (void)
{
    memset(board, 0x00, sizeof(board));
    memset(history, 0x00, sizeof(history));
    idx_history = 0;
    pass = 0;
}

/* Write text representation to given file */
static
void
text (FILE * f, const char * title)
{
    int i;
    if (fCSV != opt_format) {
        if (0 != title) {
            fprintf(f, "%% %s\n", title);
        }
        for (i = 0; i < 81; ++i) {
            if (IS_EMPTY(i)) {
                fprintf(f, fStandard == opt_format ? " ." : ".");
            } else {
                fprintf(f, fStandard == opt_format ? "%2d" : "%d",
                            GET_DIGIT(board[i]));
            }
            if (8 == COLUMN(i)) {
                fprintf(f, "\n");
                if (fStandard == opt_format && i != 80 && 2 == ROW(i) % 3) {
                    fprintf(f, "-------+-------+-------\n");
                }
            } else if (fStandard == opt_format && 2 == COLUMN(i) % 3) {
                fprintf(f, " |");
            }
        }
    } else {
        for (i = 0; i < 81; ++i) {
            if (!IS_EMPTY(i)) {
                fprintf (f, "%d", GET_DIGIT(board[i]));
            }
            if (8 == COLUMN(i)) {
                fprintf(f, "\n");
            } else {
                fprintf(f, ",");
            }
        }
    }
}

/* Write PostScript representation to given file */
static
void
postscript (FILE * f, const char * title)
{
#define PS_WIDTH          20   /* Size of each box (points) */
#define PS_MARGIN         5    /* Margin around board (points) */
#define PS_THICK          3    /* Width of thick lines (points) */
#define PS_THIN           1    /* Width of thin lines (points) */
#define PS_BASELINE       5    /* Offset of character base line */

#define PS_TOTWIDTH  (9*PS_WIDTH+2*PS_MARGIN)  /* Total board width */

/* Page size */
#define PS_A4_WIDTH       612
#define PS_A4_HEIGHT      792

#define PS_LEFT_OFFSET    ((PS_A4_WIDTH - PS_TOTWIDTH)/2)
#define PS_BASE_OFFSET    ((PS_A4_HEIGHT - PS_TOTWIDTH)/2)

#define _STR(x)      #x
#define STR(x)       _STR(x)

    int i;
    time_t t;

    time(&t);
    fprintf(f,
            "%%!PS-Adobe-3.0 EPSF-3.0\n"
            "%%%%BoundingBox: %d %d %d %d\n"
            "%%%%Creator: Sudoku by Michael Kennett\n"
            "%%%%CreationDate: %s",
            PS_LEFT_OFFSET,
            PS_BASE_OFFSET,
            PS_LEFT_OFFSET + PS_TOTWIDTH,
            PS_BASE_OFFSET + PS_TOTWIDTH,
            ctime(&t));
    if (0 != title) {
        fprintf(f, "%%%%Title: %s\n", title);
    }
    fprintf(f, "%%%%EndComments\n");

    /* Write the board contents as a string */
    fprintf(f, "(");
    for (i = 0; i < 81; ++i) {
        if (!IS_EMPTY(i)) {
            fprintf(f, "%d", GET_DIGIT(board[i]));
        } else {
            fprintf(f, " ");
        }
    }
    fprintf(f, ")\n");

    /* Co-ordinate transform */
    fprintf(f, "%d %d translate\n", PS_LEFT_OFFSET, PS_BASE_OFFSET);

    /* Draw board - thin lines first, then thick lines */
    fprintf(f,
            "0 setgray\n"
             STR(PS_THIN)
             " setlinewidth "
             "1 8 "                         /* index, followed by loop count */
             "{dup "                                           /* keep index */
              STR(PS_WIDTH) " mul "
              STR(PS_MARGIN) " add "                     /* Compute position */
              "dup dup dup\n"                                    /* 4 copies */
              "  " STR(PS_MARGIN) " moveto "                /* vertical line */
              "%d lineto "
              STR(PS_MARGIN) " exch moveto "              /* horizontal line */
              "%d exch lineto "
              "1 add"                                        /* update index */
             "} repeat pop stroke\n"
             STR(PS_THICK) " setlinewidth "   /* Repeat code for thick lines */
             "1 2 "
             "{dup "
              "%d mul " STR(PS_MARGIN) " add "
              "dup dup dup\n"
              "  " STR(PS_MARGIN) " moveto "
              "%d lineto "
              STR(PS_MARGIN) " exch moveto "
              "%d exch lineto "
              "1 add"
             "} repeat pop stroke\n"
             /* Draw outside border */
             "1 setlinejoin "
             STR(PS_MARGIN) " " STR(PS_MARGIN) " moveto "
             STR(PS_MARGIN) " %d lineto "
             "%d %d lineto "
             "%d " STR(PS_MARGIN) " lineto closepath stroke\n",
            PS_TOTWIDTH - PS_MARGIN,
            PS_TOTWIDTH - PS_MARGIN,
            3 * PS_WIDTH,
            PS_TOTWIDTH - PS_MARGIN,
            PS_TOTWIDTH - PS_MARGIN,
            PS_TOTWIDTH - PS_MARGIN,
            PS_TOTWIDTH - PS_MARGIN,
            PS_TOTWIDTH - PS_MARGIN,
            PS_TOTWIDTH - PS_MARGIN);
    /* Now the code for drawing digits */
    fprintf(f, 
            "/Helvetica-Bold findfont 12 scalefont setfont\n"
              "0 81 "                       /* index, followed by loop count */
              "{2 copy 1 getinterval "                     /* load character */
               "dup stringwidth pop\n"              /* and compute the width */
               "  " STR(PS_WIDTH) " exch sub 2 div "      /* and the x-delta */
               "2 index "                                    /* reload index */
               "9 mod " STR(PS_WIDTH) " mul add "
               STR(PS_MARGIN) " add\n"               /* compute x coordinate */
               "  8 3 index 9 idiv sub "
               STR(PS_WIDTH) " mul %d add\n"         /* compute y coordinate */
               "  moveto show "                                    /* render */
               "1 add"                                       /* update index */
              "} repeat pop pop\n",
            PS_MARGIN + PS_BASELINE);
}

static
void
html (FILE * f, const char * title)
{
    int i;

    fprintf(f, "<html><head>");
    if (0 != title) {
        fprintf(f, "<title>%s</title>", title);
    }
    fprintf(f,
            "</head><body>"
            "<table"
                " align=\"center\" border=\"1\""
                " cellpadding=\"3\" cellspacing=\"1\""
                " rules=\"all\""
            " >\n");

    for (i = 0; i < 81; ++i) {
        if (0 == i % 9) {
            fprintf(f, "<tr>");
        }
        fprintf(f, "<td>");
        if (IS_EMPTY(i)) {
            fprintf(f, "&nbsp;&nbsp;&nbsp;");
        } else {
            fprintf(f, "&nbsp;%d&nbsp;", GET_DIGIT(board[i]));
        }
        fprintf(f, "</td>");
        if (8 == i % 9) {
            fprintf(f, "</tr>\n");
        }
    }
    fprintf(f,
            "</table>"
            "</body>"
            "</html>\n");
}

static
void
print (FILE * f, const char * title)
{
    switch (opt_format) {
        case fStandard:
        case fCompact:
        case fCSV:
            text(f, title);
            break;
        case fPostScript:
            postscript(f, title);
            break;
        case fHTML:
            html(f, title);
            break;
    }
}

/* Describe solution history */
static
void
describe (FILE * f)
{
    int i, j;
    for (i = j = 0; i < idx_history; ++i) {
        if (0 == (history[i] & FIXED)) {
            if (i < idx_history && 0 < j) {
                fprintf(f, 0 == j % 6 ? "\n" : ", ");
            }
            fprintf(f, "%d %c> (%d,%d)", GET_DIGIT(history[i]),
                                      history[i] & CHOICE ? '*' : '-',
                                      1 + ROW(GET_INDEX(history[i])),
                                      1 + COLUMN(GET_INDEX(history[i])));
            ++j;
        }
    }
    fprintf(f, "\n");
}

/* Management of the move history - compression */
static
void
compress(int limit)
{
    int i, j, k;
    /* Find the first ignored history item, if there is any. */
    for (k = 0; k < idx_history; ++k) {
        if (0 != (history[k] & IGNORED)) {
            break;
        }
    }
    /* Move the rest to compress the history */
    for (i = j = k; i < idx_history && j < limit; ++i) {
        if (!(history[i] & IGNORED)) {
            history[j++] = history[i];
        }
    }
    for (; i < idx_history; ++i) {
        history[j++] = history[i];
    }
    idx_history = j;
}

/* Management of the move history - adding a move */
static
void
add_move (int idx, int digit, int choice)
{
    int i;

    if (sizeof(history) / sizeof(int) - 1 <= idx_history) {
        compress(81);
    }
    /* Never ignore the last move */
    history[idx_history++] = SET_INDEX(idx) | SET_DIGIT(digit) | choice;

    /* Ignore all previous references to idx */
    for (i = idx_history - 2; 0 <= i; --i) {
        if (GET_INDEX(history[i]) == idx) {
            history[i] |= IGNORED;
            break;
        }
    }
}

/* Iteration over rows/columns/blocks handled by specialised code.
 * Each function returns a block index - call must manage element/idx.
 */
static
int
idx_row (int el, int idx)      /* Index within a row */
{
    return INDEX(el, idx);
}

static
int
idx_column (int el, int idx)   /* Index within a column */
{
    return INDEX (idx, el);
}

static
int
idx_block (int el, int idx)    /* Index within a block */
{
    return INDEX(3 * (el / 3) + idx / 3, 3 * (el % 3) + idx % 3);
}

/* Update board state after setting a digit (clearing not handled)
 */
static
void
update (int idx)
{
    const int row = ROW(idx);
    const int col = COLUMN(idx);
    const int block = IDX_BLOCK(row, col);
    const int mask = DIGIT_STATE(DIGIT(idx));
    int i;

    board[idx] |= STATE_MASK;  /* filled - no choice possible */

    /* Digit cannot appear in row, column or block */
    for (i = 0; i < 9; ++i) {
        board[idx_row(row, i)] |= mask;
        board[idx_column(col, i)] |= mask;
        board[idx_block(block, i)] |= mask;
    }
}

/* Refresh board state, given move history. Note that this can yield
 * an incorrect state if the user has made errors - return -1 if an
 * incorrect state is generated; else return 0 for a correct state.
 */
static
int
reapply (void)
{
    int digit, idx, j;
    int allok = 0;
    memset(board, 0x00, sizeof(board));
    for (j = 0; j < idx_history; ++j) {
        if (!(history[j] & IGNORED) && 0 != GET_DIGIT(history[j])) {
            idx = GET_INDEX(history[j]);
            digit = GET_DIGIT(history[j]);
            if (!IS_EMPTY(idx) || DISALLOWED(idx, digit)) {
                allok = -1;
            }
            board[idx] = SET_DIGIT(digit);
            if (history[j] & FIXED) {
                board[idx] |= FIXED;
            }
            update(idx);
        }
    }
    return allok;
}

/* Clear moves, leaving fixed squares
 */
static
void
clear_moves (void)
{
    for (idx_history = 0; history[idx_history] & FIXED; ++idx_history) {
       ;
    }
    reapply();
}

static int digits[9];  /* # digits expressed in element square */
static int counts[9];  /* Counts of the allowed positions indexed by digit-1 */

/* Count # set bits (within STATE_MASK) */
static
int
numset (int mask)
{
    int i, n = 0;
    for (i = STATE_SHIFT + 1; i <= STATE_SHIFT + 9; ++i) {
        if (mask & (1<<i)) {
            ++n;
        } else {
            ++counts[i - STATE_SHIFT - 1];
        }
    }
    return n;
}

static
void
count_set_digits (int el, int (*idx_fn)(int, int))
{
    int i;
    memset(counts, 0x00, sizeof(counts));
    for (i = 0; i < 9; ++i) {
        digits[i] = numset(board[(*idx_fn)(el, i)]);
    }
}

/* Fill square with given digit, and update state.
 * Returns 0 on success, else -1 on error (i.e. invalid fill)
 */
static
int
fill (int idx, int digit)
{
    assert(0 != digit);

    if (!IS_EMPTY(idx)) {
        return (DIGIT(idx) == digit) ? 0 : -1;
    }
    if (DISALLOWED(idx, digit)) {
        return -1;
    }

    board[idx] = SET_DIGIT(digit);
    update(idx);
    add_move(idx, digit, 0);

    return 0;
}

/* User-level fill square; allowing clears and overwrites, and
 * invalid moves...
 */
static
void
fillx(int idx, int digit)
{
    /* Nothing to do if digit already set or cleared */
    if (DIGIT(idx) == digit) {
        return;
    }
    if (0 != digit && IS_EMPTY(idx)) {
        board[idx] = SET_DIGIT(digit) | SET_INDEX(idx);
        update(idx);
        add_move(idx, digit, 0);
    } else {
        /* Clearing or overwriting is more time consuming */
        add_move(idx, 0, 0);
        reapply();

        /* Always apply moves - even in invalid */
        if (0 != digit) {
            if (idx_history > 0) {
                history[idx_history - 1] |= SET_DIGIT(digit);
                reapply();
            } else {
                beep();
            }
        }
    }
}

/* Find all squares with a single digit allowed -- do not mutate board
 * Additionally check for each digit the contradiction whether there
 * are all squares forbidden while there is no such digit placed yet
 * Return -1 on contradiction, and 0 otherwise.
 */
static
int
singles (int el, int (*idx_fn)(int, int), int hintcode)
{
    int i;

    count_set_digits(el, idx_fn);

    for (i = 0; i < 9; ++i) {
        int c = i;
        if (0 == counts[c]) {
            /* No allowed position left for digit 'c+1' in the element. */
            int nd = 0;
            int j;
            for (j = 0; j < 9; ++j) {
                if (DIGIT((*idx_fn)(el, j)) == c + 1) {
                    /* The element already contains this digit */
                    ++nd;
                    break;
                }
            }
            if (nd == 0) {
                /* Contradiction found, this element can't have this  digit. */
                break;
            }
        }
        if (1 == counts[c] &&
            idx_possible < 81) {
            /* One allowed position left for digit 'c+1' in the element. */
            int j;
            for (j = 0; j < 9; ++j) {
                /* Let's find the place. */
                int idx = (*idx_fn)(el, j);
                if (!DISALLOWED(idx, c + 1)) {
                    possible[idx_possible++] = SET_INDEX(idx)
                                               | SET_DIGIT(c + 1)
                                               | hintcode;
                    /* We have found that single position, safe to break. */
                    break;
                }
            }
        }
        if (8 == digits[i] &&
            idx_possible < 81) {
            /* 8 digits are masked at this position - just one remaining */
            int idx = (*idx_fn)(el, i);
            int sta = (STATE_MASK & ~STATE(idx));
            int d = 0;
            /* Let's find the appropriate digit */
            for (sta >>= STATE_SHIFT + 1; 0 != sta; sta >>= 1) {
                ++d;
            }
            assert (0 < d && d < 10 && !DISALLOWED(idx, d));
            if (0 < d && d < 10) {
                /* It seems to be an appropriate digit */
                possible[idx_possible++] = SET_INDEX(idx)
                                            | SET_DIGIT(d)
                                            | hintcode;
            }
        }
    }
    return -(i < 9);
}

/* Given the board state, find all possible 'moves' (i.e. squares with just
 * a single digit).
 *
 * Returns the number of (deterministic) moves (and fills the moves array),
 * or 0 if no moves are possible. This function does not mutate the board
 * state, and hence, can return the same move multiple times (with
 * different hints).
 */
static
int
findmoves (void)
{
    int el;

    idx_possible = 0;
    for (el = 0; el < 9; ++el) {
        if (-1 == singles(el, idx_row, HINT_ROW)        ||
            -1 == singles(el, idx_column, HINT_COLUMN)  ||
            -1 == singles(el, idx_block, HINT_BLOCK)) {
            return -1;
        }
    }
    return idx_possible;
}

/* Strategies for refining the board state
 *  - 'pairs'     if there are two unfilled squares in a given row/column/
 *                block with the same state, and just two possibilities,
 *                then all other unfilled squares in the row/column/block
 *                CANNOT be either of these digits.
 *  - 'block'     if the unknown squares in a block all appear in the same
 *                row or column, then all unknown squares outside the block
 *                and in the same row/column cannot be any of the unknown
 *                squares in the block.
 *  - 'common'    if all possible locations for a digit in a block appear
 *                in a row or column, then that digit cannot appear outside
 *                the block in the same row or column.
 *  - 'position2' if the positions of 2 unknown digits in a block match
 *                identically in precisely 2 positions, then those 2 positions
 *                can only contain the 2 unknown digits.
 *
 * Recall that each state bit uses a 1 to prevent a digit from
 * filling that square.
 */

static
void
pairs (int el, int (*idx_fn)(int, int))
{
    int i, j, k, mask, idx;
    for (i = 0; i < 8; ++i) {
        if (7 == digits[i]) { /* 2 digits unknown */
            for (j = i + 1; j < 9; ++j) {
                idx = (*idx_fn)(el, i);
                if (STATE(idx) == STATE((*idx_fn)(el, j))) {
                    /* Found a row/column pair - mask other entries */
                    mask = STATE_MASK ^ (STATE_MASK & board[idx]);
                    for (k = 0; k < i; ++k) {
                        board[(*idx_fn)(el, k)] |= mask;
                    }
                    for (k = i + 1; k < j; ++k) {
                        board[(*idx_fn)(el, k)] |= mask;
                    }
                    for (k = j + 1; k < 9; ++k) {
                        board[(*idx_fn)(el, k)] |= mask;
                    }
                    digits[j] = -1; /* now processed */
                }
            }
        }
    }
}

/* Worker: mask elements outside block */
static
void
exmask (int mask, int block, int el, int (*idx_fn)(int, int))
{
    int i, idx;

    for (i = 0; i < 9; ++i) {
        idx = (*idx_fn)(el, i);
        if (block != BLOCK(idx) && IS_EMPTY(idx)) {
            board[idx] |= mask;
        }
    }
}

/* Worker for block() */
static
void
exblock (int block, int el, int (*idx_fn)(int, int))
{
    int i, idx, mask;

    /* By assumption, all unknown squares in the block appear in the
     * same row/column, so to construct a mask for these squares, it
     * is sufficient to invert the mask for the known squares in the
     * block.
     */
    mask = 0;
    for (i = 0; i < 9; ++i) {
        idx = idx_block(block, i);
        if (!IS_EMPTY(idx)) {
            mask |= DIGIT_STATE(DIGIT(idx));
        }
    }
    exmask(mask ^ STATE_MASK, block, el, idx_fn);
}

static
void
block (int el)
{
    int i, idx, row, col;

    /* Find first unknown square */
    for (i = 0; i < 9 && !IS_EMPTY(idx = idx_block(el, i)); ++i) {
       ;
    }
    if (i < 9) {
        assert(IS_EMPTY(idx));
        row = ROW(idx);
        col = COLUMN(idx);
        for (++i; i < 9; ++i) {
            idx = idx_block(el, i);
            if (IS_EMPTY(idx)) {
                if (ROW(idx) != row) {
                    row = -1;
                }
                if (COLUMN(idx) != col) {
                    col = -1;
                }
            }
        }
        if (0 <= row) {
            exblock(el, row, idx_row);
        }
        if (0 <= col) {
            exblock(el, col, idx_column);
        }
    }
}

static
void
common (int el)
{
    int i, idx, row, col, digit, mask;

    for (digit = 1; digit <= 9; ++digit) {
        mask = DIGIT_STATE(digit);
        row = col = -1;  /* Value '9' indicates invalid */
        for (i = 0; i < 9; ++i) {
            /* Digit possible? */
            idx = idx_block(el, i);
            if (IS_EMPTY(idx) && 0 == (board[idx] & mask)) {
                if (row < 0) {
                    row = ROW(idx);
                } else if (row != ROW(idx)) {
                    row = 9; /* Digit appears in multiple rows */
                }
                if (col < 0) {
                    col = COLUMN(idx);
                } else if (col != COLUMN(idx)) {
                    col = 9; /* Digit appears in multiple columns */
                }
            }
        }
        if (-1 != row && row < 9) {
            exmask(mask, el, row, idx_row);
        }
        if (-1 != col && col < 9) {
            exmask(mask, el, col, idx_column);
        }
    }
}

/* Encoding of positions of a digit (c.f. position2()) - abuse DIGIT_STATE */
static int posn_digit[10];

static
void
position2 (int el)
{
    int digit, digit2, i, mask, mask2, posn, count, idx;

    /* Calculate positions of each digit within block */
    for (digit = 1; digit <= 9; ++digit) {
        mask = DIGIT_STATE(digit);
        posn_digit[digit] = count = posn = 0;
        for (i = 0; i < 9; ++i) {
            if (0 == (mask & board[idx_block(el, i)])) {
                ++count;
                posn |= DIGIT_STATE(i);
            }
        }
        if (2 == count) {
            posn_digit[digit] = posn;
        }
    }
    /* Find pairs of matching positions, and mask */
    for (digit = 1; digit < 9; ++digit) {
        if (0 != posn_digit[digit]) {
            for (digit2 = digit + 1; digit2 <= 9; ++digit2) {
                if (posn_digit[digit] == posn_digit[digit2]) {
                    mask = STATE_MASK
                           ^ (DIGIT_STATE(digit) | DIGIT_STATE(digit2));
                    mask2 = DIGIT_STATE(digit);
                    for (i = 0; i < 9; ++i) {
                        idx = idx_block(el, i);
                        if (0 == (mask2 & board[idx])) {
                            assert(0 == (DIGIT_STATE(digit2) & board[idx]));
                            board[idx] |= mask;
                        }
                    }
                    posn_digit[digit] = posn_digit[digit2] = 0;
                    break;
                }
            }
        }
    }
}

/* Find some moves for the board; starts with a simple approach (finding
 * singles), and if no moves found, starts using more involved strategies
 * until a move is found. The more advanced strategies can mask states
 * in the board, making this an efficient mechanism, but difficult for
 * a human to understand.
 */
static
int
allmoves (void)
{
    int i, n;

    n = findmoves();
    if (0 != n) {
        return n;
    }

    for (i = 0; i < 9; ++i) {
        count_set_digits(i, idx_row);
        pairs(i, idx_row);

        count_set_digits(i, idx_column);
        pairs(i, idx_column);

        count_set_digits(i, idx_block);
        pairs(i, idx_block);
    }
    n = findmoves();
    if (0 != n) {
        return n;
    }
    for (i = 0; i < 9; ++i) {
        block(i);
        common(i);
        position2(i);
    }
    return findmoves();
}

/* Helper: sort based on index */
static
int
cmpindex (const void * a, const void * b)
{
    return GET_INDEX(*((const int *)b)) - GET_INDEX(*((const int *)a));
}

/* Return number of hints. The hints mechanism should attempt to find
 * 'easy' moves first, and if none are possible, then try for more
 * cryptic moves.
 */
int
findhints (void)
{
    int i, n, mutated = 0;

    n = findmoves();
    if (n < 2) {
        /* Each call to pairs() can mutate the board state, making the
         * hints very, very cryptic... so later undo the mutations.
         */
        for (i = 0; i < 9; ++i) {
            count_set_digits(i, idx_row);
            pairs(i, idx_row);

            count_set_digits(i, idx_column);
            pairs(i, idx_column);

            count_set_digits(i, idx_block);
            pairs(i, idx_block);
        }
        mutated = 1;
        n = findmoves();
    }
    if (n < 2) {
        for (i = 0; i < 9; ++i) {
            block(i);
            common(i);
        }
        mutated = 1;
        n = findmoves();
    }

    /* Sort the possible moves, and allow just one hint per square */
    if (0 < n) {
        int i, j;

        qsort(possible, n, sizeof(int), cmpindex);
        for (i = 0, j = 1; j < n; ++j) {
            if (GET_INDEX(possible[i]) == GET_INDEX(possible[j])) {
                /* Let the user make mistakes - do not assume the
                 * board is in a consistent state.
                 */
                if (GET_DIGIT(possible[i]) == GET_DIGIT(possible[j])) {
                    possible[i] |= possible[j];
                }
            } else {
                i = j;
            }
        }
        n = i + 1;
    }

    /* Undo any mutations of the board state */
    if (mutated) {
        reapply();
    }
    return n;
}

/* Deterministic solver; return 0 on success, else -1 on error.
 */
static
int
deterministic (void)
{
    int i, n;

    n = allmoves();
    while (0 < n) {
        ++pass;
        for (i = 0; i < n; ++i) {
            if (-1 == fill(GET_INDEX(possible[i]),
                            GET_DIGIT(possible[i]))) {
                return -1;
            }
        }
        n = allmoves();
    }
    return n;
}

/* Return index of square for choice.
 *
 * If no choice is possible (i.e. board solved or inconsistent),
 * return -1.
 *
 * The current implementation finds a square with the minimum
 * number of unknown digits (i.e. maximum # masked digits).
 */
static
int
cmp (const void * e1, const void * e2)
{
    return GET_DIGIT(*(const int *)e2) - GET_DIGIT(*(const int *)e1);
}

static
int
choice (void)
{
    int i, n;
    for (n = i = 0; i < 81; ++i) {
        if (IS_EMPTY(i)) {
            possible[n] = SET_INDEX(i) | SET_DIGIT(numset(board[i]));

            /* Inconsistency if square unknown, but nothing possible */
            if (9 == GET_DIGIT(possible[n]))
                return -2;
            ++n;
        }
    }
    if (0 == n) {
        return -1;      /* All squares known */
    }
    qsort(possible, n, sizeof(possible[0]), cmp);
    return GET_INDEX(possible[0]);
}

/* Choose a digit for the given square.
 * The starting digit is passed as a parameter.
 * Returns -1 if no choice possible.
 */
static
int
choose (int idx, int digit)
{
    for (; digit <= 9; ++digit) {
        if (!DISALLOWED(idx, digit)) {
            board[idx] = SET_DIGIT(digit);
            update(idx);
            add_move(idx, digit, CHOICE);
            return digit;
        }
    }
    return -1;
}

/* Backtrack to a previous choice point, and attempt to reseed
 * the search. Return -1 if no further choice possible, or
 * the index of the changed square.
 *
 * Assumes that the move history and board are valid.
 */
static
int
backtrack (void)
{
    int digit, idx;

    for (; 0 < --idx_history;) {
        if (history[idx_history] & CHOICE) {
            /* Remember the last choice, and advance */
            idx = GET_INDEX(history[idx_history]);
            digit = GET_DIGIT(history[idx_history]) + 1;
            reapply();
            if (-1 != choose(idx, digit)) {
                return idx;
            }
        }
    }
    return -1;
}

/* Attempt to solve 'board'; return 0 on success else -1 on error.
 *
 * The solution process attempts to fill-in deterministically as
 * much of the board as possible. Once that is no longer possible,
 * need to choose a square to fill in.
 */
static
int
solve (void)
{
    int idx;

    for (;;) {
        if (0 == deterministic()) {
            /* Solved, make a new choice, or rewind a previous choice */
            idx = choice();
            if (-1 == idx) {
                idx = 0;
                break;
            } else if ((idx < 0 || -1 == choose(idx, 1))
                       && -1 == backtrack()) {
                idx = -1;
                break;
            }
        } else {
            /* rewind to a previous choice */
            if (-1 == backtrack()) {
                idx = -1;
                break;
            }
        }
    }
    return idx;
}

/* Find all solutions to a given board, and return the number of
 * solutions (0 if none found).
 */
static
int
number_solutions (void)
{
    int count = 0;
    if (-1 != solve()) {
        do {
            ++count;
        } while (-1 != backtrack() && -1 != solve());
    }
    return count;
}

/* Build/modify internal representation from file
 *
 *  - lines starting with '#' are ignored
 *  - puzzles start with a line beginning with '%' (with optional title)
 *  - two formats are support: compact, verbose
 *  - templates are always compact
 *  - compact boards have no spaces or block separators
 *  - verbose boards have spaces and block separators
 *
 * When is_tmplt is TRUE, a template is read.
 *
 * Return 0 on success; else -1 on error
 */

static char line[80];
static char title[80];

#define COMPACT  0
#define VERBOSE  1

static
int
read_board (FILE * f, int is_tmplt)
{
    char * p, * q;
    int i, row, col, type = COMPACT;

    reset();
    len_tmplt = 0;

    /* Skip lines until a '%' is found */
    line[0] = ' ';
    while ('%' != line[0]) {
        if (0 == tgets(line, sizeof(line), f)) {
            return -1;
        }
    }
    /* Read optional title, and removing trailing whitespace */
    for (p = line + 1; *p && isspace(*p); ++p) {
       ;
    }
    if (*p) {
        for (q = title; '\0' != (*q++ = *p++);) {
           ;
        }
        --q;
        while (isspace(*--q)) {
           ;
        }
        *++q = '\0';
    } else {
        strcpy(title, "(untitled)");
    }
    /* Consume comment lines - no leading spaces allowed */
    line[0] = '#';
    while ('#' == line[0]) {
        if (0 == tgets(line, sizeof(line), f)) {
            return -1;
        }
    }
    /* Analyse first line to determine the 'type' - default is COMPACT */
    if (0 == is_tmplt) {
        for (p = line; *p && COMPACT == type; ++p) {
            if ('|' == *p) {
                type = VERBOSE;
            }
        }
    }
    /* Consume grid - allow leading spaces and comments at end */
    for (row = 0; row < 9; ++row) {
        /* Assume line already loaded into buffer; skip leading spaces */
        for (p = line; *p && isspace(*p); ++p) {
           ;
        }
        for (col = 0; *p && col < 9; ++col, ++p) {
            if (is_tmplt) {
                if ('*' == *p) {
                    tmplt[len_tmplt++] = INDEX(row, col);
                }
            } else {
                if (VERBOSE == type) {
                    while (*p && (isspace(*p) || '|' == *p)) {
                        ++p;
                    }
                }
                if (isdigit(*p)) {
                    if (0 != fill(INDEX(row, col), *p - '0')) {
                        return -1;
                    }
                    board[INDEX(row, col)] |= FIXED;
                }
                /* else assume blank square */
            }
        }
        /* Don't complain about any trailing characters - ignored silently */

        /* Load next line (if needed) */
        if (row < 8) {
            if (0 == tgets(line, sizeof(line), f)) {
                return -1;
            }
            if (VERBOSE == type
                && 2 == row % 3
                && 0 == tgets(line, sizeof(line), f)) {
                return -1; /* Skip separators */
            }
        }
    }

    /* Construct move history for a template */
    if (is_tmplt) {
        idx_history = 0;
        for (i = 0; i < 81; ++i) {
            if (0 != DIGIT(i)) {
                history[idx_history++] = i | (DIGIT(i) << 8);
            }
        }
    }
    /* Finally, markup all of these moves as 'fixed' */
    for (i = 0; i < idx_history; ++i) {
        history[i] |= FIXED;
    }
    return 0;
}

/**
 **  Curses screen interface
 **/

/* Screen geometry */
#define SUDOKU_LINE   2
#define TITLE_LINE    4
#define TOP           6
#define LEFT_LEFT     0
#define LEFT_MIDDLE  15
#define LEFT         27
#define SUDOKU_POS   35
#define BOTTOM       (TOP+3*4)
#define RIGHT        (LEFT+3*8)
#define LINE_SIZE    80
#define STATUS_LINE  20
#define FILE_LINE    21
#define LAST_LINE    23

/* Maintain some global state - current cursor position */
static int curx;
static int cury;

static int have_status;  /* True (non-zero) if status line set */
static int have_hint;    /* True (non-zero) If hint displayed */

static char statusline[LINE_SIZE];  /* Buffer for status line */

/* Render board background - assume 24x80 screen */
static
void
draw_screen (void)
{
    int i;

    wclear(stdscr);
    attron(A_BOLD);
    mvaddstr(SUDOKU_LINE, SUDOKU_POS, "Su-Do-Ku!");
    attroff(A_BOLD);

    for (i = 0; i < 3; ++i) {
        mvaddstr(TOP + 0 + 4 * i, LEFT, "+-------+-------+-------+");
        mvaddstr(TOP + 1 + 4 * i, LEFT, "|       |       |       |");
        mvaddstr(TOP + 2 + 4 * i, LEFT, "|       |       |       |");
        mvaddstr(TOP + 3 + 4 * i, LEFT, "|       |       |       |");
    }
    mvaddstr(TOP + 4 * 3, LEFT, "+-------+-------+-------+");

    mvaddstr(TOP + 2, LEFT_LEFT + 1, "Rules:");
    mvaddstr(TOP + 3, LEFT_LEFT + 2, "Fill the grid so that");
    mvaddstr(TOP + 4, LEFT_LEFT + 2, "every column, row and");
    mvaddstr(TOP + 5, LEFT_LEFT + 2, "3x3 box contains each");
    mvaddstr(TOP + 6, LEFT_LEFT + 2, "of the digits 1 to 9.");
    i = TOP + 7;
    if (0 == opt_restrict) {
        mvaddstr(++i, LEFT_LEFT + 1, "File:");
        mvaddstr(++i, LEFT_LEFT + 2, "s   save board");
        mvaddstr(++i, LEFT_LEFT + 2, "w   write template");
        mvaddstr(++i, LEFT_LEFT + 2, "o   open board");
        mvaddstr(++i, LEFT_LEFT + 2, "t   set board title");
    }

/*    mvaddstr(TOP - 1, RIGHT + 4, "Keys:"); */
    mvaddstr(TOP + 0, RIGHT + 8, "k");
    mvaddstr(TOP + 1, RIGHT + 4, "  h   l move cursor");
    mvaddstr(TOP + 2, RIGHT + 8, "j");
    mvaddstr(TOP + 3, RIGHT + 7, "1-9  place digit");
    mvaddstr(TOP + 4, RIGHT + 7, "0 .  clear digit");
    mvaddstr(TOP + 5, RIGHT + 8, "c   clear board");
    mvaddstr(TOP + 6, RIGHT + 8, "d   redraw the board");
    mvaddstr(TOP + 7, RIGHT + 8, "f   fix squares");
    mvaddstr(TOP + 8, RIGHT + 8, "n   new board");
    mvaddstr(TOP + 9, RIGHT + 8, "q   quit game");
    i = TOP + 9;
    mvaddstr(++i, RIGHT + 8, "r   restart");
    mvaddstr(++i, RIGHT + 8, "u   undo last move");
    mvaddstr(++i, RIGHT + 8, "v   solve");
    mvaddstr(++i, RIGHT + 8, "?   request hint");
}

/* Write board title */
static
void
write_title(const char * title)
{
    move(TITLE_LINE, LEFT_LEFT);
    wclrtoeol(stdscr);
    if (0 != title) {
        mvaddstr(TITLE_LINE,
                 (int)((LINE_SIZE - strlen(title)) / 2),
                 title);
    }
}

/* Move cursor to grid position, and force refresh */
static
void
move_to (int x, int y)
{
    curx = x;
    cury = y;

    move(TOP + 1 + y + y / 3,
         LEFT + 2 + 2 * (x + x / 3));

    wrefresh(stdscr);
}

/* Move cursor to next non-fixed square, and force refresh */
void
static
move_next (void)
{
    int is_loop = 0;
    do {
        if (curx < 8) {
            move_to(curx + 1, cury);
        } else if (cury < 8) {
            move_to(0, cury + 1);
        } else {
            move_to(0, 0);
            if (is_loop++ > 0) {
                /* We can get here if the user attempts to load a fully
                 * populated board. In that case it's better to break.  */
                break;
            }
        }
     } while (IS_FIXED(INDEX(cury, curx)));
}

/* Render status line */
static
void
set_status (const char * txt)
{
    mvaddstr(STATUS_LINE,
             (int)((LINE_SIZE - strlen(txt)) / 2),
             (char *)txt);

    move_to(curx, cury);
    wrefresh(stdscr);
    have_status = 1;
}

static
void
clear_status (void)
{
    move(STATUS_LINE, LEFT_LEFT);
    wclrtoeol(stdscr);
    move_to(curx, cury);
    have_status = 0;
}

/* Show meassge in the status line for 2 seconds */
static
void
status_message (const char * txt)
{
    beep();
    set_status(txt);
    usleep(2000000);
    clear_status();
}

/* Beep and show meassge in the status line for 2 seconds */
static
void
beep_status_message (const char * txt)
{
    beep();
    status_message(txt);
}

/* Render current board */
static
void
render (void)
{
    int i, x, y;

    for (i = 0; i < 81; ++i) {
        x = LEFT + 2 + 2 * (COLUMN(i) + COLUMN(i) / 3);
        y = TOP + 1 + ROW(i) + ROW(i) / 3;
        assert(0 <= DIGIT(i));
        assert(DIGIT(i) <= 9); /* XXX FAILING */
        if (IS_FIXED(i)) {
            attron(A_BOLD);
        }
        if (IS_EMPTY(i)) {
            mvaddch(y, x, '.');
        } else {
            mvaddch(y, x, '0' + DIGIT(i));
        }
        if (IS_FIXED(i)) {
            attroff(A_BOLD);
        }
    }
}

static
void
row_hint (int row)
{
    mvaddch(TOP + 1 + row + row / 3, LEFT - 2, '>');
    mvaddch(TOP + 1 + row + row / 3, RIGHT + 2, '<');
    move_to(curx, cury);
    have_hint = 1;
}

static
void
column_hint (int col)
{
    mvaddch(TOP - 1, LEFT + 2 + 2 * (col + col / 3), 'v');
    mvaddch(BOTTOM + 1, LEFT + 2 + 2 * (col + col / 3), '^');
    move_to(curx, cury);
    have_hint = 1;
}

static
void
block_hint (int block)
{
    int i, j;
    for (i = 0; i < 3; ++i) {
        j = 3 * (block / 3) + i;
        mvaddch(TOP + 1 + j + j / 3, LEFT - 2, '>');
        mvaddch(TOP + 1 + j + j / 3, RIGHT + 2, '<');
        j = 3 * (block % 3) + i;
        mvaddch(TOP - 1, LEFT + 2 + 2 * (j + j / 3), 'v');
        mvaddch(BOTTOM + 1, LEFT + 2 + 2 * (j + j / 3), '^');
    }
    move_to(curx, cury);
    have_hint = 1;
}

static
void
clear_hints (void)
{
    int i;
    for (i = 0; i < 9; ++i) {
        mvaddch(TOP + 1 + i + i / 3, LEFT - 2, ' ');
        mvaddch(TOP + 1 + i + i / 3, RIGHT + 2, ' ');
        mvaddch(TOP - 1, LEFT + 2 + 2 * (i + i / 3), ' ');
        mvaddch(BOTTOM + 1, LEFT + 2 + 2 * (i + i / 3), ' ');
    }
    have_hint = 0;
    move_to(curx, cury);
}

/* Fix all squares - if possible.
 * Returns -1 on error, in which case there is an error in the
 * current board; otherwise returns 0 if all is OK.
 */
static
int
fix (void)
{
    int i;

    if (0 == reapply()) {
        compress(idx_history);
        for (i = 0; i < idx_history; ++i) {
            history[i] |= FIXED;
        }
        reapply();
        render();
        if (idx_history < 81 && IS_FIXED(INDEX(cury, curx))) {
            move_next();
        } else {
            move_to(curx, cury);
        }
        return 0;
    } else {
        return -1;
    }
}

/* Limit factor definitions for the classification. */
#define LIMIT_FACTOR_VERY_EASY  15
#define LIMIT_FACTOR_EASY       11
#define LIMIT_FACTOR_MEDIUM     7
#define LIMIT_FACTOR_HARD       4
#define LIMIT_FACTOR_FIENDISH   0

static const char *     NAME_VERY_EASY  = "very easy";
static const char *     NAME_EASY       = "easy";
static const char *     NAME_MEDIUM     = "medium";
static const char *     NAME_HARD       = "hard";
static const char *     NAME_FIENDISH   = "fiendish";

/* Class limit and name for the classification. */
#define CL(ct)   ((LIMIT_FACTOR_##ct) * pass)
#define CN(ct)    NAME_##ct

static
const char *
class_name_by_score (int score)
{
    const char * name;
    if (CL(VERY_EASY) < score) {
        name = CN(VERY_EASY);
    } else if (CL(EASY) < score) {
        name = CN(EASY);
    } else if (CL(MEDIUM) < score) {
        name = CN(MEDIUM);
    } else if (CL(HARD) < score) {
        name = CN(HARD);
    } else {
        name = CN(FIENDISH);
    }
    return name;
}

/* Classify a SuDoKu, given its solution.
 *
 * The classification is based on the average number of possible moves
 * for each pass of the deterministic solver - it is a rather simplistic
 * measure, but gives reasonable results. Note also that the classification
 * is based on the first solution found (but does handle the pathological
 * case of multiple solutions). Note that the average moves per pass
 * depends just on the number of squares initially set... this simplifies
 * the statistics collection immensely, requiring just the number of passes
 * to be counted.
 *
 * Return 0 on error, else a string classification.
 */

static
const char *
classify (void)
{
    int i, score;

    pass = 0;
    clear_moves();
    if (-1 == solve()) {
        return 0;
    }

    score = 81;
    for (i = 0; i < 81; ++i) {
        if (IS_FIXED(i)) {
            --score;
        }
    }
    assert(81 == idx_history);

    for (i = 0; i < 81; ++i) {
        if (history[i] & CHOICE) {
            score -= 5;
        }
    }
    return  class_name_by_score(score);
}

/* exchange disjoint, identical length blocks of data */
static
void
exchange (int * a, int * b, int len)
{
    int i, tmp;
    for (i = 0; i < len; ++i) {
        tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

/* rotate left */
static
void
rotate1_left (int * a, int len)
{
    int i, tmp;
    tmp = a[0];
    for (i = 1; i < len; ++i) {
        a[i - 1] = a[i];
    }
    a[len - 1] = tmp;
}

/* rotate right */
static
void
rotate1_right (int * a, int len)
{
    int i, tmp;
    tmp = a[len - 1];
    for (i = len - 1; 0 < i; --i) {
        a[i] = a[i - 1];
    }
    a[0] = tmp;
}

/* Generalised left rotation - there is a naturally recursive
 * solution that is best implementation using iteration.
 * Note that it is not necessary to do repeated unit rotations.
 *
 * This function is analogous to 'cutting' a 'pack of cards'.
 *
 * On entry: 0 < idx < len
 */
static
void
rotate (int * a, int len, int idx)
{
    int xdi = len - idx;
    int delta = idx - xdi;

    while (0 != delta && 0 != idx) {
        if (delta < 0) {
            if (1 == idx) {
                rotate1_left(a, len);
                idx = 0;
            } else {
                exchange(a, a + xdi, idx);
                len = xdi;
            }
        } else {
            /* 0 < delta */
            if (1 == xdi) {
                rotate1_right(a, len);
                idx = 0;
            } else {
                exchange(a, a + idx, xdi);
                a += xdi;
                len = idx;
                idx -= xdi;
            }
        }
        xdi = len - idx;
        delta = idx - xdi;
    }
    if (0 < idx) {
        exchange(a, a + idx, idx);
    }
}

/* Shuffle an array of integers */
static
void
shuffle (int * a, int len)
{
    int i, j, tmp;

    i = len;
    while (1 <= i) {
        j = rand() % i;
        tmp = a[--i];
        a[i] = a[j];
        a[j] = tmp;
    }
}

/* Generate a SuDoKu puzzle
 *
 * The generation process selects a random template, and then attempts
 * to fill in the exposed squares to generate a board. The order of the
 * digits and of filling in the exposed squares are random.
 */

/* Select random template; sets tmplt, len_tmplt */
static
void
select_template (void)
{
    int i = n_tmplt > 0 ? rand() % n_tmplt : 0;
    tseek(ftmplt, 0, SEEK_SET);
    while (0 <= i && 0 == read_board(ftmplt, 1)) {
        --i;
    }
}

static
void
generate (void)
{
    static int digits[9];

    int i;

    for (;;) {
        for (i = 0; i < 9; ++i) {
            digits[i] = i + 1;
        }

        rotate(digits, 9, 1 + rand() % 8);
        shuffle(digits, 9);
        select_template();

        rotate(tmplt, len_tmplt, 1 + rand() % (len_tmplt - 1));
        shuffle(tmplt, len_tmplt);

        reset();  /* construct a new board */

        for (i = 0; i < len_tmplt; ++i) {
            fill(tmplt[i], digits[i % 9]);
        }
        if (0 != solve() || idx_history < 81) {
            continue;
        }
        for (i = 0; i < len_tmplt; ++i) {
            board[tmplt[i]] |= FIXED;
        }
        /* Construct fixed squares */
        for (idx_history = i = 0; i < 81; ++i) {
            if (IS_FIXED(i)) {
                history[idx_history++] = SET_INDEX(i)
                                         | SET_DIGIT(DIGIT(i))
                                         | FIXED;
            }
        }
        clear_moves();

        if (0 != solve() || idx_history < 81) {
            continue;
        }
        if (-1 != backtrack() && 0 == solve()) {
            continue;
        }
        if (NULL != requested_class) {
            const char * generated_class = classify();
            sprintf(title, "randomly generated - %s", generated_class);
            write_title(title);
            wrefresh(stdscr);
            if (0 != strcmp(requested_class, generated_class)) {
                continue;
            }
        }

        break;
    }

    strcpy(title, "randomly generated - ");
    strcat(title, classify());

    clear_moves();
    time(&start_time);
}

/* Support for explicitly opened board */
static FILE * opened;

/* Support for precanned board (optional) */
static FILE * precanned;
static int n_precanned;
static int completed;

static
int
open_precanned (const char * filename)
{
    n_precanned = 0;
    precanned = fopen(filename, "r");
    if (0 != precanned) {
        while (0 == read_board(precanned, 0)) {
            ++n_precanned;
        }
    }
    /* return 0 on success, -1 on failure. */
    return (n_precanned > 0) - 1;
}

static
void
open_template (const char * filename)
{
    n_tmplt = 0;
    ftmplt = topen(filename, "r");
    if (0 != ftmplt) {
        while (0 == read_board(ftmplt, 1)) {
            ++n_tmplt;
        }
    }
}

static
int
is_complete (void)
{
    int i;
    for (i = 0; i < 81 && !IS_EMPTY(i); ++i) {
        /* counting the non-empty squares. */
    }
   return (81 == i);
}

/* load a new board - this could be a precanned board,
 * or a randomly generated board - chose between these
 * randomly -- 1 in 3 chance of loading a precanned
 * board (if they exist).
 */
static
void
load_board (void)
{
    int i = 0;
    if (opened) {
        /* Select next board */
        i = read_board(opened, 0);
        if (i--) {
            fclose(opened);
            opened = NULL;
        }
    } else if (precanned &&
               (0 == opt_random ||
                (0 == rand() % 3 && 0 < n_precanned))) {
        /* Select random board */
        i = rand() % n_precanned;
        fseek(precanned, 0, SEEK_SET);
        while (0 <= i && 0 == read_board(precanned, 0)) {
            --i;
        }
    }
    if (i != -1) {
        set_status("generating a random board... (please wait)");
        generate();
        clear_status();
    }

    /* Shameless plug... */
    set_status("Su-Do-Ku by Michael Kennett");

    render();
    write_title(title);

    curx = cury = 8;    /* move_next() takes care of this... */
    move_next();
    completed = is_complete();
    num_hints = -1;
    time(&start_time);
}

/* Manage rich text input (define 'virtual' key codes)
 * See the console(4) manpage.
 */

#define VKEY_IGNORE     (256)
#define VKEY_UP         (256+'A')
#define VKEY_DOWN       (256+'B')
#define VKEY_RIGHT      (256+'C')
#define VKEY_LEFT       (256+'D')
#define VKEY_HOME       (256+'H')
#define VKEY_INSERT     (256+'2')
#define VKEY_DELETE     (256+'3')
#define VKEY_END        (256+'4')
#define VKEY_PGUP       (256+'5')
#define VKEY_PGDOWN     (256+'6')
#define VKEY_BACK       (0x08)


static
int
getkey (void)
{
    int ch = wgetch(stdscr);
    if (0x1b == ch) {
        ch = wgetch(stdscr);
        if ('[' == ch) {
            ch = wgetch(stdscr);
            switch (ch) {
                case 'A': ch = VKEY_UP;     break;
                case 'B': ch = VKEY_DOWN;   break;
                case 'C': ch = VKEY_RIGHT;  break;
                case 'D': ch = VKEY_LEFT;   break;
                case 'H': ch = VKEY_HOME;   break;
                case '2': ch = VKEY_INSERT; break;
                case '3': ch = VKEY_DELETE; break;
                case '4': ch = VKEY_END;    break;
                case '5': ch = VKEY_PGUP;   break;
                case '6': ch = VKEY_PGDOWN; break;
                default:  ch = VKEY_IGNORE;
            }
            if (ch == VKEY_HOME ||
                ch == VKEY_DELETE ||
                ch == VKEY_INSERT ||
                ch == VKEY_END ||
                ch == VKEY_PGUP ||
                ch == VKEY_PGDOWN) {
                int ch4 = wgetch(stdscr);
                if (ch4 != '~') {
                    ch = VKEY_IGNORE;
                }
            }
        } else {
            /* Enabling the Esc Esc sequence to produce one Esc character */
            if (0x1b != ch) {
                ch = VKEY_IGNORE;
            }
        }
    }
    return ch;
}

/* Save board representation - the difficulty is managing the user interface
 * (i.e. finding where to save), with a tad more work involved in allowing
 * the board to be written to an arbitrary process.
 *
 * Persist the user filename.
 */
#if !defined (PATH_MAX)
/* If it is not defined by the operating system, we can safely define it,
 * since the code below has no strong dependence on the current working
 * directory, for which it is solely used. */
#define PATH_MAX 1024
#endif

static char userfile[PATH_MAX];
static char templatefile[PATH_MAX];

typedef struct edit_s {
    char *  eb;         /* edit buffer                                  */
    size_t  ebs;        /* edit buffer size                             */
    size_t  efs;        /* edit field horizontal size                   */
    int     efx;        /* edit field horizontal position               */
    int     efy;        /* edit field vertical position                 */
    int     ecn;        /* actual number of characters (including '\0') */
    int     ecp;        /* edit position in the buffer                  */
    int     x_0;        /* prompt position on the display               */
    int     x_1;        /* first text position on the display           */
    int     x_2;        /* last txt position on display                 */
    int     x_3;        /* continuation marker position on display      */
    int     x_mv;       /* maximal number of visible chars on display   */
    int     m_0;        /* first editable char position in the buffer   */
    int     m_1;        /* first visible char position in the buffer    */
    int     prompt;     /* currently displayed prompt character         */
    int     dprompt;    /* default prompt character                     */
    int     eol_mark;   /* line continuation marker                     */
    int     ins_mode;   /* insert mode                                  */
    int     ch;         /* actual character for processing              */
} edit_t;


/* Init edit text data.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_init (edit_t *   ep,
         int        edit_field_y,
         int        edit_field_x,
         char *     edit_buffer,
         size_t     edit_buffer_size,
         size_t     edit_field_size)
{
    ep->eb          = edit_buffer;
    ep->ebs         = edit_buffer_size;
    ep->efs         = edit_field_size;
    ep->efx         = edit_field_x;
    ep->efy         = edit_field_y;
    ep->ecn         = (int)(strlen(edit_buffer));
    ep->ecp         = ep->ecn;
    ep->x_0         = edit_field_x;
    ep->x_1         = edit_field_x + 1;
    ep->x_2         = (int)edit_field_size - 2;
    ep->x_3         = ep->x_2 + 1;
    ep->x_mv        = ep->x_2 - ep->x_1;
    ep->m_0         = 0;
    ep->m_1         = 0;
    ep->prompt      = '$';
    ep->dprompt     = '$';
    ep->eol_mark    = '*';
    ep->ins_mode    = 1;
    ep->ch          = edit_buffer[0];

    /* Find out what should be the prompt character. */
    if (ep->ch == '>' ||
        ep->ch == '|' ||
        ep->ch == '%') {
        /* Learn the prompt and set the minimal 
         * index of the string buffer to 1.
         */
        ep->dprompt = ep->ch;
        ep->m_0     = 1;
        ep->m_1     = 1;
    }
    /* Calculate the visible part of the pathname. */
    if (ep->x_mv < ep->ecn - ep->m_0) {
        /* set the buffer position of the first visible character */
        ep->m_1 = ep->ecn - ep->x_mv;
    }

    if (ep->m_1 > 1) {
        /* give visual feedback about that there are more characters */
        ep->prompt = '<';
    } else {
        /* set the default prompt */
        ep->prompt = ep->dprompt;
    }
}

/* Functions to classify the actual character.
 * The ep pointer must point to a valid memory location.
 */
static
int
el_is_silent_ignore_char (edit_t *  ep)
{
    return (VKEY_UP == ep->ch ||
            VKEY_DOWN == ep->ch ||
            VKEY_IGNORE == ep->ch);
}

static
int
el_is_loud_ignore_char (edit_t *    ep)
{
    return ('|' == ep->ch ||
            '>' == ep->ch ||
            '<' == ep->ch);
}

static
int
el_is_changeable_prompt_char (edit_t *  ep)
{
    return (('%' != ep->prompt &&
             '$' != ep->prompt)
            && ('|' == ep->ch ||
                '>' == ep->ch));
}

/* Functions to classify the actual cursor position.
 * The ep pointer must point to a valid memory location.
 */
static
int
el_is_cursor_at_rightmost_position (edit_t *   ep)
{
    return (ep->ecp == ep->m_1 &&
            ep->m_1 == 1);
}

/* Functions to classify the actual cursor position.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_adjust_markers (edit_t *   ep)
{
    int first_invisible = ep->m_1 + ep->x_mv;
    if (ep->m_0 == ep->m_1) {
        /* The first visible is the first editable character. */
        if (ep->prompt != ep->dprompt) {
            /* Show the default prompt if it is not shown yet. */
            ep->prompt = ep->dprompt;
            mvaddch(ep->efy, ep->x_0, ep->prompt);
        }
    } else {
        /* The first visible is not the first editable character. */
        if (ep->prompt != '<') {
            ep->prompt = '<';
            /* Show the '<' prompt. */
            mvaddch(ep->efy, ep->x_0, ep->prompt);
        }
    }
    if (ep->ecn > first_invisible) {
        /* The last visible is not the last editable character. */
        if (ep->eol_mark != '<') {
            ep->eol_mark = '<';
            mvaddch(ep->efy, ep->x_3, ep->eol_mark);
        }
    } else {
        if (ep->eol_mark != ' ') {
            ep->eol_mark = ' ';
            mvaddch(ep->efy, ep->x_3, ep->eol_mark);
        }
    }
}

/* Process change the prompt character.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_process_change_prompt (edit_t *  ep)
{
    /* change the prompt */
    ep->dprompt = ep->ch;
    ep->prompt = ep->dprompt;
    /* Display the prompt character. */
    mvaddch(ep->efy, ep->x_0, ep->prompt);
}

/* Show the actual character.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_show_actual_char (edit_t *  ep)
{
    if (ep->m_0 <= ep->m_1 &&
        ep->m_1 <= ep->ecp &&
        ep->ecp <= ep->ecn) {

        int first_invisible = ep->m_1 + ep->x_mv;

        if (ep->ecp < first_invisible) {
            /* Display the character */
            mvaddch(ep->efy,
                    ep->ecp - ep->m_1 + ep->x_1,
                    ep->eb[ep->ecp]);
        } else {
            if (ep->ecp != first_invisible ||
                ep->eb[ep->ecp] != '\0') {
                /* This should never happen. */
                beep();
            }
        }
    } else {
        /* This should never happen. */
        beep();
    }
}

/* Highlight the actual character in overwrite mode.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_highlight_in_overwrite_mode (edit_t *    ep)
{
    if (!ep->ins_mode) {
        /* Overwrite mode, highlight the character at the current position. */
        attron(A_BOLD);
        el_show_actual_char(ep);
        attroff(A_BOLD);
    }
}

/* Remove the highlight at the current position in overwrite mode.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_remove_highlight_in_overwrite_mode (edit_t *    ep)
{
    if (!ep->ins_mode) {
        /* Overwrite mode, remove the highlight at the current position. */
        el_show_actual_char(ep);
    }
}

/* Show the actual character and highlight in overwrite mode.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_show_actual_char_with_highlight (edit_t *    ep)
{
    if (ep->ins_mode) {
        /* Insert mode */
        el_show_actual_char(ep);
    } else {
        /* Overwrite mode */
        el_highlight_in_overwrite_mode(ep);
    }
}

/* Process toggle insert mode.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_process_toggle_insert_mode (edit_t *    ep)
{
    /* toggle insert mode. */
    ep->ins_mode = !(ep->ins_mode);
    el_show_actual_char_with_highlight(ep);
}

/* Process delete character at the current position.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_process_delete_ch_at_current_position (edit_t *    ep)
{
    /* check if there is a character to delete at the current position */
    if (ep->m_0 <= ep->ecp && ep->ecp < ep->ecn) {
        /* delete character at the current position */
        --ep->ecn;
        if (ep->ecp < ep->ecn && ep->ecn > ep->m_0) {
            int first_invisible = ep->m_1 + ep->x_mv;
            int is_scroll_from_left = (ep->m_0 < ep->m_1 &&
                                       ep->ecn < first_invisible);
            /* The character is in middle of the string:
             *  remove the character at the cursor from the buffer
             */
            memmove(&(ep->eb[ep->ecp]),
                    &(ep->eb[ep->ecp + 1]),
                    (ep->ecn - ep->ecp) + 1);

            /* partial display of the right side
             * scroll left if it is not the last character,
             * or scroll right if it was the last
             */
            if (is_scroll_from_left) {
                /* Decrease the left limit, show one more from left. */
                --ep->m_1;
                /* redisplay the left side */
                if (ep->x_mv < ep->ecn - ep->m_1) {
                    /* partial display of the right side */
                    char che = ep->eb[ep->ecp];
                    ep->eb[ep->ecp] = 0;
                    mvaddstr(ep->efy, ep->x_1, &(ep->eb[ep->m_1]));
                    ep->eb[ep->ecp] = che;
                    el_show_actual_char(ep);
                } else {
                    mvaddstr(ep->efy, ep->x_1, &(ep->eb[ep->m_1]));
                }
            } else {
                /* redisplay the right side */
                if (ep->x_mv < ep->ecn - ep->m_1) {
                    /* partial display of the right side */
                    int first_invisible = ep->m_1 + ep->x_mv;
                    char che = ep->eb[first_invisible];
                    ep->eb[first_invisible] = 0;

                    mvaddstr(ep->efy,
                             ep->ecp - ep->m_1 + ep->x_1,
                             &(ep->eb[ep->ecp]));

                    ep->eb[first_invisible] = che;
                } else {
                    mvaddstr(ep->efy,
                             ep->ecp - ep->m_1 + ep->x_1,
                             &(ep->eb[ep->ecp]));
                }
            }
            /* Higliht the char in overwrite mode at the current position */
            el_highlight_in_overwrite_mode(ep);
        }
        ep->eb[ep->ecn] = '\0';
        if (ep->x_mv > ep->ecn - ep->m_1) {
            /* clear the deleted last character on the display */
            mvaddch(ep->efy, ep->ecn - ep->m_1 + ep->x_1, ' ');
        }
        /* Make sure that the proper markers displayed. */
        el_adjust_markers (ep);
    } else {
        beep();
    }
}

/* Process delete character at the current position.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_process_move_cursor_left_or_right (edit_t *    ep)
{
    if ((ep->m_0 < ep->ecp && VKEY_LEFT == ep->ch) ||
        (ep->ecp < ep->ecn && VKEY_RIGHT == ep->ch)) {
        /* Remove the highlight at the previous position if the removal is needed. */
        int first_invisible = ep->m_1 + ep->x_mv;
        el_remove_highlight_in_overwrite_mode(ep);
        ep->ecp += (VKEY_RIGHT == ep->ch) - (VKEY_LEFT == ep->ch);
        if (ep->ecp < ep->m_1
            || ep->ecp > first_invisible
            || (ep->ecp == first_invisible && ep->ecp != ep->ecn)) {
            /* scroll right or left as needed */
            ep->m_1 += (ep->ecp >= first_invisible) - (ep->ecp < ep->m_1);
            first_invisible = ep->m_1 + ep->x_mv;
            if (first_invisible < (int)ep->ebs) {
                char chx = ep->eb[first_invisible];
                ep->eb[first_invisible] = 0;
                mvaddstr(ep->efy, ep->x_1, &(ep->eb[ep->m_1]));
                ep->eb[first_invisible] = chx;
            } else {
                mvaddstr(ep->efy, ep->x_1, &(ep->eb[ep->m_1]));
            }
        }
        /* Higliht the char in overwrite mode at the current position */
        el_highlight_in_overwrite_mode(ep);
        /* Make sure that the proper markers displayed. */
        el_adjust_markers (ep);
    } else {
        beep();
    }
}

/* Process destructive backspace.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_process_destructive_backspace (edit_t *   ep)
{
    if (ep->m_0 < ep->ecp) {
        int first_invisible = ep->m_1 + ep->x_mv;
        int is_scroll_from_left = (ep->m_0 < ep->m_1);
        assert(ep->ecn >= ep->ecp);
        --ep->ecn;
        --ep->ecp;
        if (is_scroll_from_left) {
            /* Decrease the left limit, show one more from left. */
            --ep->m_1;
            --first_invisible;
        }
        if (ep->ecp <= ep->ecn && ep->ecn > ep->m_0) {
            /* Remove the deleted character. */

            memmove(&(ep->eb[ep->ecp]),
                    &(ep->eb[ep->ecp + 1]),
                    (ep->ecn - ep->ecp) + 1);

            if (is_scroll_from_left ||
                first_invisible < ep->ecn) {
                /* There is still a hidden part on the right side.
                 * Remove the highlight and show the modified string.
                 */
                if (is_scroll_from_left) {
                    /* scroll from left, redraw the left side */
                    char chk = ep->eb[ep->ecp];
                    ep->eb[ep->ecp] = 0;

                    mvaddstr(ep->efy,
                             ep->x_1,
                             &(ep->eb[ep->m_1]));

                    ep->eb[ep->ecp] = chk;

                    mvaddch(ep->efy,
                            ep->x_1 + ep->ecp - ep->m_1,
                            chk);

                } else {
                    /* Learn the first invisible character and replace
                     * it with a terminating 0.
                     */
                    char chr = ep->eb[first_invisible];
                    ep->eb[first_invisible] = 0;
                    /* Scroll from right, redraw the right side. */
                    mvaddstr(ep->efy,
                            ep->ecp - ep->m_1 + ep->x_1,
                            &(ep->eb[ep->ecp]));
                    /* Restore the hidden character in the buffer. */
                    ep->eb[first_invisible] = chr;
                }
            } else {
                /* The whole string is visible redraw the right side. */
                mvaddstr(ep->efy,
                        ep->ecp - ep->m_1 + ep->x_1,
                        &(ep->eb[ep->ecp]));
            }

            /* Higliht the char in overwrite mode at the current position */
            el_highlight_in_overwrite_mode(ep);
        }

        ep->eb[ep->ecn] = '\0';
        if (ep->ecn <= first_invisible) {
            /* Clear the earlier last character position if it is visible */
            mvaddch(ep->efy,
                    ep->ecn - ep->m_1 + ep->x_1,
                    ' ');
        }
        /* Make sure that the proper markers displayed. */
        el_adjust_markers (ep);
    } else {
        beep();
    }
}

/* Process visible character.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_put_visible_char_in_the_middle (edit_t *    ep)
{
    int first_invisible     = ep->m_1 + ep->x_mv;
    int is_more_on_left     = (first_invisible < ep->ecn || ep->ins_mode);
    int is_scroll_to_left   = (first_invisible <= ep->ecp + is_more_on_left);
    int nsl                 = !(is_scroll_to_left);

    if (ep->ins_mode) {
        /* Right-shift the previous content. */
        memmove(&ep->eb[ep->ecp + 1],
                &ep->eb[ep->ecp],
                (ep->ecn - ep->ecp) + 1);
        ++ep->ecn;
    }
    ep->eb[ep->ecp] = (char)ep->ch;
    ep->eb[ep->ecn] = '\0';
    el_show_actual_char(ep);
    ++ep->ecp;

    if (is_scroll_to_left) {
        /* Scroll to left */
        ++ep->m_1;
        ++first_invisible;
    }
    /* Remove the previous highlight if there
     * was any, and show the modified text.
     */
    if (first_invisible < ep->ecn) {
        /* partial display from the cursor
         * or from the beginning of the visible part
         */
        char    che = ep->eb[first_invisible];

        ep->eb[first_invisible] = 0;

        mvaddstr(ep->efy,
                 nsl * (ep->ecp - ep->m_1) + ep->x_1,
                 &(ep->eb[nsl * (ep->ecp - ep->m_1) + ep->m_1]));

        ep->eb[first_invisible] = che;
    } else {
        /* display the entire right side from the cursor
         * or from the beginning of the visible part
         */
        mvaddstr(ep->efy,
                 nsl * (ep->ecp - ep->m_1) + ep->x_1,
                 &(ep->eb[nsl * (ep->ecp - ep->m_1) + ep->m_1]));
    }

    /* Higliht the char in overwrite mode at the current position */
    el_highlight_in_overwrite_mode(ep);
    /* Make sure that the proper markers displayed. */
    el_adjust_markers (ep);
}

/* Process a visible character.
 * The ep pointer must point to a valid memory location.
 */
static
void
el_process_visible_char (edit_t *    ep)
{
    if (ep->ecn < (int)ep->ebs - 1) {
        /* There is enough room in the input buffer. */
        if (ep->ecp < ep->ecn) {
            /* In the middle of the text. */
            assert(ep->ecp >= ep->m_1 && ep->m_1 >= ep->m_0);
            el_put_visible_char_in_the_middle (ep);
        } else if (ep->ecp == ep->ecn) {
            /* At the end of the text, insert and overwrite does the same. */
            int first_invisible = ep->m_1 + ep->x_mv;

            if (ep->ecp >= first_invisible) {
                if (ep->ecp == first_invisible) {
                    /* Scroll left. */
                    ++ep->m_1;
                    ep->eb[ep->ecp] = (char)ep->ch;
                    ep->ecn = ++ep->ecp;
                    ep->eb[ep->ecn] = '\0';
                    /* Display it. */
                    mvaddstr(ep->efy,
                             ep->x_1,
                             &(ep->eb[ep->m_1]));
                } else {    
                    /* Attempt to write way beyond the end of the visible area.
                     * This should never happen.
                     */
                    beep();
                }
            } else {
                /* There is enough room on the display. */
                ep->eb[ep->ecp] = (char)ep->ch;
                el_show_actual_char(ep);
                ep->ecn = ++ep->ecp;
                ep->eb[ep->ecn] = '\0';
            }
            /* Make sure that the proper markers displayed. */
            el_adjust_markers (ep);
        } else {
            /* Attempt to write beyond the end of the text.
             * This should never happen.
             */
            beep();
        }
    } else {
        /* The input buffer is full, no more character can be stored.
         * This might happen.
         */
        beep();
    }
}

/* Edit text at a given screen position.
 * The edit_buffer must point to a valid memory location.
 *
 * The first character has a special role, it can specify the prompt.
 * By default we are using the '$' character for the prompt.
 * If the text is longer than the edit filed size, than we change the
 * prompt to '<' and only the end of the text will be displayed first.
 */
static
int
edit_line (int      edit_field_y,
           int      edit_field_x,
           char *   edit_buffer,
           size_t   edit_buffer_size,
           size_t   edit_field_size)
{
    edit_t es;

    el_init (&es,
             edit_field_y,
             edit_field_x,
             edit_buffer,
             edit_buffer_size,
             edit_field_size);

    move(es.efy, es.efx);
    wclrtoeol(stdscr);
    /* Display the prompt character. */
    mvaddch(es.efy, es.x_0, es.prompt);

    /* Display the visible part of the pathname. */
    move(es.efy, es.x_1);
    mvaddstr(es.efy, es.x_1, &es.eb[es.m_1]);

    es.ch = VKEY_IGNORE;
    while ('\r' != es.ch && es.ch != '\n') {
        /* Break if Esc pressed
         * Esc should be pressed twice, otherwise we cannot get it.
         */
        if (es.ch == 0x1B) {
            /* Use the zero length to propagate the end condition. */
            es.ecn = 0;
            break;
        }
        if (el_is_silent_ignore_char(&es)) {
            /* Silently ignore these characters. */
        } else if (el_is_loud_ignore_char(&es)) {
            if (el_is_changeable_prompt_char(&es) && 
                el_is_cursor_at_rightmost_position(&es)) {
                /* change the prompt */
                el_process_change_prompt(&es);
            } else {
                /* Loudly ignore these characters. */
                beep();
            }
        } else if (VKEY_INSERT == es.ch) {
            /* toggle insert mode. */
            el_process_toggle_insert_mode(&es);
        } else if (VKEY_DELETE == es.ch) {
            /* delete character at the current position */
            el_process_delete_ch_at_current_position(&es);
        } else if (VKEY_LEFT == es.ch ||
                   VKEY_RIGHT == es.ch) {
            /* nondestructive left or right move */
            el_process_move_cursor_left_or_right(&es);
        } else if (VKEY_BACK == es.ch || 0x7F == es.ch) {
            /* destructive backspace */
            el_process_destructive_backspace(&es);
        } else if (isprint(es.ch)) {
            /* visible character */
            el_process_visible_char(&es);
        } else {
            beep();
        }
        move(es.efy, es.ecp - es.m_1 + es.x_1);
        wrefresh(stdscr);
        es.ch = getkey();
    }
    es.eb[es.ecn] = '\0';
    return es.ecn;
}

typedef
void
(*pathname_function) (char *        working_buffer,
                      size_t        working_buffsize,
                      const char *  default_name);

/* Get string. There is no formal check performed.
 * The buffer is assumed to represent a valid memory location,
 * violation asserted, but checked against the null pointer value.
 *
 * The returned filename might contain a prompt character, therefore
 * it should be first trimmed by an 'trim_string' function.
 */
static
int
get_string (char *                  buffer,
           size_t                   buffsize,
           const char *             edit_field_name,
           const char *             default_name,
           const char *             operation_name,
           char                     default_prompt,
           pathname_function        get_pathname)
{
    int i = 0;
    char * temp_str = NULL;
    int line;

    if (buffsize > 0) {
        temp_str = (char *)calloc(buffsize, sizeof(char));
    }
    assert (buffer != NULL && buffsize > 0);
    if (buffer != NULL && temp_str != NULL) {
        #if (LINE_SIZE - LEFT_MIDDLE + 1) < 60
        #error "Bad parameters, there isn't enough room for status display."
        #endif
        char abort_message[LINE_SIZE - LEFT_MIDDLE + 1];
        size_t  storage_size = sizeof(abort_message);
        const char * first_part = "Press Esc key twice to abort the '";
        const char * last_part  = "' operation.";

        strncpy(abort_message, first_part, storage_size);
        abort_message[sizeof(abort_message) - 1]= '\0';
        storage_size -= strlen(abort_message) + 1;
        strncat (abort_message, operation_name, storage_size);
        storage_size = sizeof(abort_message) - strlen(abort_message) - 1;
        strncat (abort_message, last_part, storage_size);

        strncpy (temp_str, buffer, buffsize);
        /* Just a safety measure. */
        temp_str[buffsize - 1]= '\0';

        if (get_pathname != NULL) {
            get_pathname (temp_str,
                          buffsize,
                          default_name);
        }

        if ('\0' == *temp_str) {
            size_t dns = ((default_name != NULL) ? strlen(default_name) : 0);
            temp_str[0] = default_prompt;
            if (0 < dns && dns < buffsize - 1) {
                strcpy(temp_str + 1, default_name);
            }
        }

        clear_status();
        mvaddstr(STATUS_LINE, LEFT_LEFT, edit_field_name);
        mvaddstr(STATUS_LINE, LEFT_MIDDLE, abort_message);

        i = edit_line(FILE_LINE,
                      LEFT_LEFT,
                      temp_str,
                      buffsize,
                      LINE_SIZE - LEFT_LEFT - 3);

        if (i > 0) {
            /* We have a nonzero length string, and the user
             * didn't abort by pressing the Esc key twice.
             * Let's copy the new filename to the output buffer. */
            strcpy (buffer, temp_str);
        } else {
            i = 0;
        }

        /* Reset screen */
        clear_status();
        for (line = FILE_LINE; line <= LAST_LINE; ++line) {
            /* Clear the filename and below from the seen. */
            move(line, LEFT_LEFT);
            wclrtoeol(stdscr);
        }
        move_to(curx, cury);
    }
    free(temp_str);
    return (i > 0) - 1;
}

/* Get title string. There is no formal check performed.
 * The buffer is assumed to represent a valid memory location,
 * violation asserted, but checked against the null pointer value.
 *
 * The returned filename might contain the '>' prompt therefore
 * it should be first trimmed by the 'trim_title' function.
 */
static
int
get_title (char *           buffer,
           size_t           buffsize,
           const char *     default_name,
           const char *     operation_name)
{
    return get_string (buffer,
                       buffsize,
                       "Title:",
                       default_name,
                       operation_name,
                       '%',
                       NULL);
}

static
void
get_filepath (char *            working_buffer,
              size_t            working_buffsize,
              const char *      default_name)
{
    char *          wbp = working_buffer;
    size_t          wbs = working_buffsize;
    const char *    dn  = default_name;
    /* Just for the sake of having shorter names. */
    assert (wbp != NULL && wbs > 0);
    if (wbp != NULL && wbs > 0) {

        if ('\0' == *wbp) {
            wbp[0] = '>';
            assert(dn != NULL && strlen(dn) < 6 + 3);
            if (0 == getcwd(wbp + 1,
                            (int)wbs - 1 - 1 - 6 - 3)) {
                /* The path is probably too long, use the filename alone. */
                strcpy(wbp + 1, dn);
            } else {
                if (dn != NULL) {
                    /* If the path would make it longer than the line length
                     * we can use just the filename. If the file name itself
                     * is too long, than we cannot avoid the wrap around, so
                     * it's better to be prepared for handling that.
                     */
                    if (strlen(wbp) + strlen(dn) > wbs) {
                        wbp[1] = '\0';
                    } else {
                        strcat(wbp, "/");
                    }
                    strcat(wbp, dn);
                }
            }
            wbp[wbs - 1] = '\0';
        }
    }
}

/* Get a file name. There is no formal check performed.
 * The buffer is assumed to represent a valid memory location,
 * violation asserted, but checked against the null pointer value.
 *
 * The returned filename might contain the '>' prompt therefore
 * it should be first trimmed by the 'trim_filename' function.
 */
static
int
get_filename (char *        buffer,
              size_t        buffsize,
              const char *  default_name,
              const char *  operation_name)
{
    return get_string(buffer,
                      buffsize,
                      "Filename:",
                      default_name,
                      operation_name,
                      '$',
                      get_filepath);
}

static
const char * 
trim_string (const char *   buffer,
             size_t         buffsize,
             const char *   prefixes)
{
    const char * p = buffer;
    if (p && buffsize > 0) {
        if (prefixes != NULL &&
            strchr(prefixes,*p) != NULL) {
            ++p;
            --buffsize;
        }
        while (buffsize > 0 && ' ' == *p) {
            ++p;
            --buffsize;
        }
    }
    if (buffsize == 0) {
        p = NULL;
    }
    return p;
}

static
const char * 
trim_titlename (const char *    buffer,
                size_t          buffsize)
{
    return trim_string(buffer, buffsize, "%");
}

static
const char * 
trim_filename (const char *     buffer,
               size_t           buffsize)
{
    return trim_string(buffer, buffsize, ">|");
}

static
void
save_board (void)
{
    const char * p;
    FILE * f;

    /* Read character input (raw processing mode) */

    if (0 == get_filename(userfile,
                          sizeof(userfile),
                          DEFAULT_BOARD_NAME,
                          "Save board")) {

        p = trim_filename(userfile,
                          sizeof(userfile));

        if (p != NULL && *p) {
            if ('|' == userfile[0]) {
                f = popen(p, "w");
                if (0 != f) {
                    print(f, title);
                    pclose(f);
                }
            } else {
                /* Append for standard formats, else separate file */
                switch (opt_format) {
                    case fCompact:
                    case fStandard:
                        f = fopen(p, "a");
                        break;
                    default:
                        f = fopen(p, "w");
                        break;
                }
                if (0 != f) {
                    print(f, title);
                    fclose(f);
                }
            }
            if (0 == f) {
                set_status("Error: failed to write the file!");
            }
        } else {
            set_status("Error: no valid file name found!");
        }
    } else {
        status_message("Save board operation aborted by the user.");
    }
}

/* Write default template to a user-given filename
 */
static
void
write_template (void)
{
    if (0 == get_filename(templatefile,
                            sizeof(templatefile),
                            TEMPLATE_FALLBACK,
                            "Write template")) {

            const char * p = trim_filename(templatefile,
                                            sizeof(templatefile));

        if (p != NULL && *p) {
            if (0 == write_default_template(p)) {
                status_message("Template file successfully created!");
            } else {
                beep_status_message("Template file exists"
                                    " or write error occurred!");
            }
        } else {
            beep_status_message("No valid file name found!");
        }
    } else {
        status_message("Write default template operation aborted by the user.");
    }
}

/* Open saved board(s)
 */
static
void
open_board (void)
{
    if (0 == get_filename(userfile,
                          sizeof(userfile),
                          DEFAULT_BOARD_NAME,
                          "Open board")) {

        const char * p = trim_filename(userfile,
                                       sizeof(userfile));

        if (p != NULL && *p) {
            if (opened != NULL) {
                fclose(opened);
                opened = NULL;
            }
            opened = fopen(p, "r");
            if (opened != NULL) {
                load_board();
            } else {
                set_status("Error: failed to open the board!");
            }
        } else {
            beep_status_message("No valid file name found!");
        }
    } else {
        status_message("Open board operation aborted by the user.");
    }
}

/* Rename the board title.
 * Returns a process exit code.
 */
static
void
rename_board_title (void)
{
    char new_title[80];
    new_title[0]='\0';
    if (0 == get_title(new_title,
                        sizeof(new_title),
                        title,
                        "Rename the board title")) {
        const char *    nt;
        size_t          ts = sizeof(new_title);
        if (ts > sizeof(title)) {
            ts = sizeof(title);
        }
        nt = trim_titlename(new_title, ts);
        if (nt != NULL && *nt != '\0') {
            strcpy(title, nt);
            write_title(title);
        }
        move_to(curx, cury);
    }
}

/* Generate statistics from boards in 'filename', and/or solve them.
 * Returns a process exit code.
 */
static
int
gen_statistics (void)
{
    int retval = 0;
    if (0 == precanned) {
        fprintf(stderr, "Error: no precanned boards loaded\n");
        retval = -1;
    } else {
        const char * classification;

        fseek (precanned, 0, SEEK_SET);
        while (0 == read_board(precanned, 0)) {
            /* Ignore insoluble boards */
            if (-1 == solve()) {
                printf("Board '%s' has no solution\n", title);
                continue;
            }

            /* If statistics only, ignore boards with multiple solutions */
            if (0 == opt_solve && -1 != backtrack() && 0 == solve()) {
                printf("Board '%s' has multiple solutions\n", title);
                continue;
            }

            classification = classify();
            if (0 == opt_solve) {
                printf("%2d %-12s : %s\n", pass, classification, title);
            } else {
                printf("Solution(s) to '%s' [%s]\n", title, classification);
                clear_moves();
                if (-1 != solve()) {
                    do {
                        print(stdout, title);
                        if (opt_describe) {
                            printf("Solution history:\n");
                            describe(stdout);
                        }
                    } while (-1 != backtrack() && -1 != solve());
                }
            }
        }
    }
    return retval;
}

/* cleanup curses */
static
void
cleanup_curses_and_more (void)
{
    if (opened != NULL) {
        fclose(opened);
        opened = NULL;
    }
    if (precanned != NULL) {
        fclose(precanned);
        precanned = NULL;
    }
    tclose(ftmplt);
    ftmplt = NULL;
    move(LAST_LINE, 0);
    wrefresh(stdscr);
    endwin();
}

/* Signal catchers - cleanup curses and terminate */
static
void
cleanup (int ignored)
{
    ignored = 1;
    cleanup_curses_and_more();
    exit(ignored);
}

/* Establish signal handlers */
static
void
signals (void)
{
    sigset_t sigset;
    struct sigaction sighandler;
    struct termios tp;

    sigemptyset(&sigset);
    sighandler.sa_handler = cleanup;
    sighandler.sa_mask = sigset;
    sighandler.sa_flags = 0;

    sigaction(SIGINT,  &sighandler, 0);
    sigaction(SIGABRT, &sighandler, 0);
    sigaction(SIGTERM, &sighandler, 0);
#ifdef SIGHUP
    sigaction(SIGHUP,  &sighandler, 0);
#endif
#ifdef SIGQUIT
    sigaction(SIGQUIT, &sighandler, 0);
#endif
#ifdef SIGKILL
    sigaction(SIGKILL, &sighandler, 0);
#endif

    /* Re-enable signal processing */
    if (0 == tcgetattr(0, &tp)) {
        tp.c_lflag |= ISIG;
        tcsetattr(0, TCSANOW, &tp);
    }
}

static
void
usage (void)
{
    fprintf(stderr,
            "Usage: %s [options] [<filename>]\n"
            "Supported options:\n"
            "    -c<class>    generate a board until it finds a board of the\n"
            "                 specified class. Supported classes are:\n"
            "                    %s, %s, %s, %s, and %s\n"
            "    -d           describe solution steps (with -v)\n"
            "    -f<format>   set output format; supported formats are:\n"
            "                    standard   (std)    <default format>\n"
            "                    compact\n"
            "                    csv                 [comma separated file]\n"
            "                    postscript (ps)\n"
            "                    html\n"
            "    -g[<num>]    generate <num> board(s), and print on stdout\n"
            "    -n           no random boards (requires precanned boards)\n"
            "    -r           restricted: don't allow boards to be saved\n"
            "    -s           calculate statistics for precanned boards\n"
            "    -t<filename> template file\n"
            "    -v           solve precanned boards\n"
            "    -w           write out the default template\n"
            "                 to the current directory\n"
            "    <filename>   'precanned' sudoku boards\n",
            program,
            CN(VERY_EASY),
            CN(EASY),
            CN(MEDIUM),
            CN(HARD),
            CN(FIENDISH));
}

void request_hint (void)
{
    ++req_hints;
    if (have_hint) {
        clear_hints();
    }
    if (-1 == num_hints) {
        last_hint = -1;
        num_hints = findhints();
    }
    if (0 == num_hints) {
        set_status("No hints available!");
    } else {
        int i, idx;
        int n = 0;

        if (1 < num_hints) {
            do {
                i = rand() % num_hints;
            } while (i == last_hint);
        } else {
            i = 0;
        }
        idx = GET_INDEX(possible[last_hint = i]);

        /* Count # possible ways of expressing hint */
        if (0 != (HINT_ROW & possible[i])) {
            ++n;
        }
        if (0 != (HINT_COLUMN & possible[i])) {
            ++n;
        }
        if (0 != (HINT_BLOCK & possible[i])) {
            ++n;
        }
        assert(0 < n);
        if (1 < n) {
            n = 1 + rand() % n;
        }
        if (0 != (HINT_ROW & possible[i])) {
            if (0 == --n) {
                row_hint(ROW(idx));
            }
        }
        if (0 != (HINT_COLUMN & possible[i])) {
            if (0 == --n) {
                column_hint(COLUMN(idx));
            }
        }
        if (0 != (HINT_BLOCK & possible[i])) {
            if (0 == --n) {
                block_hint(IDX_BLOCK(ROW(idx), COLUMN(idx)));
            }
        }
        if (opt_spoilerhint) {
            /* Useful for testing... */
            sprintf(statusline, "%d @ row %d, column %d",
                        GET_DIGIT(possible[i]),
                        ROW(idx)+1, COLUMN(idx)+1);
            set_status(statusline);
        } else if (10 < req_hints || 2 * num_hints < req_hints) {
            sprintf(statusline, "(try the digit %d)",
                        GET_DIGIT(possible[i]));
            set_status(statusline);
        }
    }
}

int
evaluate_options (int argc, char **argv)
{
    int ret_err = 0;
    if (1 < argc) {
        char * arg;
        while (0 < --argc) {
            arg = *++argv;
            if ('-' != *arg) {
                if (0 != precanned) {
                    fprintf(stderr, "Error: only 1 precanned file allowed\n");
                    ret_err = 1;
                    /* will exit */
                    break;
                }
                open_precanned(arg);
                strcpy(userfile, arg);  /* Save the filename */
                if (0 == precanned || 0 == n_precanned) {
                    fprintf(stderr, "Error: failed to open '%s'\n", arg);
                    ret_err = 1;
                    /* will exit */
                    break;
                }
            } else {
                while ('\0' != *++arg) {
                    switch (*arg) {
                        case 'c':
                            /* No need for strdup, since the underlying
                             * memory in the environment will not go away. */
                            requested_class = &arg[1];
                            if( 0 != strcmp(requested_class, CN(VERY_EASY)) &&
                                0 != strcmp(requested_class, CN(EASY))      &&
                                0 != strcmp(requested_class, CN(MEDIUM))    &&
                                0 != strcmp(requested_class, CN(HARD))      &&
                                0 != strcmp(requested_class, CN(FIENDISH))) {
                                fprintf(stderr,
                                        "Error:"
                                        " the class must be one of"
                                        " '%s', '%s', '%s', '%s', '%s'\n",
                                        CN(VERY_EASY),
                                        CN(EASY),
                                        CN(MEDIUM),
                                        CN(HARD),
                                        CN(FIENDISH));
                                ret_err = 1;
                                /* will exit */
                            }
                            /* dummy to force termination */
                            arg = "x";
                            break;
                        case 'd':
                            opt_describe = 1;
                            break;
                        case 'f':
                            if ('\0' == arg[1]) {
                                arg = *++argv;
                                --argc;
                            } else {
                                ++arg;
                            }
                            if (0 == strcmp("compact", arg)) {
                                opt_format = fCompact;
                            } else if (0 == strcmp("standard", arg)
                                    || 0 == strcmp("std", arg)) {
                                opt_format = fStandard;
                            } else if (0 == strcmp("csv", arg)) {
                                opt_format = fCSV;
                            } else if (0 == strcmp("postscript", arg) ||
                                       0 == strcmp("ps", arg)) {
                                    opt_format = fPostScript;
                            } else if (0 == strcmp("html", arg)) {
                                opt_format = fHTML;
                            } else {
                                fprintf(stderr,
                                        "Error: '%s' is an unknown format\n",
                                        arg);
                                ret_err = 1;
                                /* will exit */
                            }
                            /* dummy to force termination */
                            arg = "x";
                            break;

                        case 'g':
                            opt_generate = 1;
                            if (isdigit(arg[1])) {
                                num_generate = atoi(arg + 1);
                                /* dummy to force termination */
                                arg = "x";
                            } else if ('\0' == arg[1]
                                      && 0 != *(argv+1)
                                      && isdigit(**(argv+1))) {
                                num_generate = atoi(*++argv);
                                --argc;
                            }
                            break;
                        case 'h': opt_spoilerhint = 1; break;
                        case 'n': opt_random = 0; break;
                        case 'r': opt_restrict = 1; break;
                        case 's': opt_statistics = 1; break;
                        case 't':
                            if ('\0' == arg[1]) {
                                if (0 == *(argv+1)) {
                                    fprintf(stderr,
                                            "Error:"
                                            " expected argument after '-t'\n");
                                    ret_err = 1;
                                    /* will exit */
                                    /* dummy to force termination */
                                    arg = "x";
                                    break;
                                }
                                arg = *++argv;
                                open_template(arg);
                                --argc;
                            } else {
                                open_template(++arg);
                            }
                            if (0 == ftmplt) {
                                fprintf(stderr,
                                        "Error:"
                                        " failed to open template file '%s'\n",
                                        arg);
                                ret_err = 1;
                                /* will exit */
                            }
                            /* dummy to force termination */
                            arg = "x";
                            break;
                        case 'v':
                            opt_solve = 1;
                            break;
                        case 'w':
                            write_default_template(TEMPLATE_FALLBACK);
                            break;
                        default:
                            usage();
                            ret_err = 1;
                            /* will exit */
                            /* dummy to force termination */
                            arg = "x";
                            break;
                    }
                }
            }
            if (ret_err != 0) {
                /* will force the exit */
                break;
            }
        }
    }
    return ret_err;
}

static
void
play_the_game (void)
{
    int ch = ' ';
    while ('q' != ch) {
        wrefresh(stdscr);
        ch = getkey();

        if (have_status) {
            clear_status();
        }
 
       switch (ch) {
            case '.':
                ch = '0';
                /* Drop thru' to digit handler */
            case '0':
                /* clear location, or add digit to board */
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': /* clear location, or add digit to board */
                if (!IS_FIXED(INDEX(cury, curx))) {
                    num_hints = -1;
                    fillx(INDEX(cury, curx), ch - '0');
                    addch('0' == ch ? '.' : ch);
                    move_to(curx, cury);
                    if (have_hint) {
                        clear_hints();
                    }
                    req_hints = 0;
                } else {
                    beep();
                }
                break;
            case ' ':  /* move to next */
                move_next();
                break;
            case 'h':  /* move left */
            case VKEY_LEFT:
                if (0 < curx) {
                    move_to(curx - 1, cury);
                } else {
                      move_to(8, cury);
                }
                break;
            case 'j': /* move down */
            case VKEY_DOWN:
                if (cury < 8) {
                    move_to(curx, cury + 1);
                } else {
                    move_to(curx, 0);
                }
                break;
            case 'k': /* move up */
            case VKEY_UP:
                if (0 < cury) {
                    move_to(curx, cury - 1);
                } else {
                    move_to(curx, 8);
                }
                break;
            case 'l': /* move right */
            case VKEY_RIGHT:
                if (curx < 8) {
                    move_to(curx + 1, cury);
                } else {
                    move_to(0, cury);
                }
                break;
            case 'u': /* undo last move */
                if (idx_history <= 0
                    || history[idx_history - 1] & FIXED) {
                    assert(idx_history >= 0);
                    beep();
                } else {
                    int i;
                    /* Don't ignore last reference (if exists) */
                    for (i = --idx_history - 1; 0 <= i; --i) {
                        if (GET_INDEX(history[i])
                            == GET_INDEX(history[idx_history])) {
                            history[i] &= ~IGNORED;
                            break;
                        }
                    }
                    num_hints = -1;
                    if (have_hint) {
                        clear_hints();
                    }
                    req_hints = 0;
                    reapply();
                    render();
                    if (history[idx_history - 1] & FIXED) {
                        curx = cury = 8;
                        move_next();
                    } else {
                        move_to (COLUMN(GET_INDEX(history[idx_history])),
                                 ROW(GET_INDEX(history[idx_history])));
                    }
                }
                break;
            case 'c': /* clear board */
                completed = 0;
                num_hints = -1;
                if (have_hint) {
                    clear_hints();
                }
                reset();
                render();
                write_title(0);
                move_to(0, 0);
                break;
            case 'd': /* refresh screen */
                wrefresh(stdscr);
                redrawwin(stdscr);
                break;
            case 'f': /* fix squares (if possible) */
                if (0 == idx_history || 0 != (history[0] & FIXED)) {
                    break;
                }
                if (0 != fix()) {
                    set_status("There is an error - no solution possible!");
                    beep();
                }
                break;
            case 'n': /* load new board */
                if (have_hint) {
                    clear_hints();
                }
                reset();
                render();
                write_title(0);
                wrefresh(stdscr);
                load_board();
                break;
            case 'v': /* show solution */
                clear_moves();
                if (have_hint) {
                    clear_hints();
                }
                num_hints = -1;
                if (0 == solve()) {
                    completed = 1;
                } else {
                    beep_status_message("This board has no solution!");
                }
                render();
                move_to(curx, cury);
                break;

            case 'r': /* restart the same board */
                clear_moves();
                render();
                curx = 8;
                cury = 8;
                /* move_next() takes care of this... */
                move_next();
                break;

            case 'w': /* write default template */
                if (0 == opt_restrict) {
                    write_template();
                }
                break;
            case 'o': /* open saved board */
                if (0 == opt_restrict) {
                    open_board();
                }
                break;

            /* The main difficulty with saving a board is managing the
             * user interface.
             */
            case 's':
                if (0 == opt_restrict) {
                    save_board();
                }
                break;

            case 't':
                if (0 == opt_restrict) {
                    rename_board_title();
                }
                break;

            case '?': /* Request hint */
                request_hint();
                break;
            case 'q': /* quit */
                break;
            default:
                beep();
        }

        /* Check for a solution */
        if (0 == completed) {
            if (is_complete() && 0 == fix()) {
                time_t end_time;
                time( &end_time );
                end_time -= start_time;
                sprintf(statusline, "Well done"
                                    " - you've completed the puzzle!"
                                    " (%02d:%02d:%02d)",
                                    (int) (end_time/60/60),
                                    (int) (end_time/60)%60,
                                    (int) (end_time%60));
                beep();
                set_status(statusline);
                completed = 1;
            }
        }
    }
}

int
main (int argc, char **argv)
{
    program = argv[0];

    /* Limited support for options */
    if (0 != evaluate_options(argc, argv)) {
        cleanup_curses_and_more();
        exit(1);
    }
    if (0 != opt_statistics && 0 != opt_generate) {
        fprintf(stderr, "Error: Cannot set both -g and -s options\n");
        cleanup_curses_and_more();
        return -1;
    }

    if (0 == precanned) {
        open_precanned(PRECANNED);

        /* Fallback - try current working directory */
        if (0 == precanned) {
            open_precanned(PRECANNED_FALLBACK);
        }
        /* Else, can continue happily without any precanned files... */
    }

    if (0 != opt_statistics || 0 != opt_solve) {
        int retval = gen_statistics();
        cleanup_curses_and_more();
        return retval;
    }

    srand((unsigned int)(time(0) ^ getpid()));

    if (0 == ftmplt) {
        open_template(TEMPLATE);

        /* Fallback - try current working directory */
        if (0 == ftmplt) {
            open_template(TEMPLATE_FALLBACK);
            if (0 == ftmplt) {
                fprintf(stderr, "Error: failed to open template file\n");
                exit(1);
            } else if (0 >= n_tmplt) {
                fprintf(stderr, "Error:"
                                " no valid template found"
                                " in the template file\n");
                cleanup_curses_and_more();
                exit(1);
            }
        }
    }

    if (0 != opt_generate) {
        /* -g0 generates many boards */
        if (0 == num_generate) {
            /* changed from --num_generate, to make it
             * independent from the actual size of int
             * 10000 seems to qualify for being 'many'
             */
            num_generate = 10000;
        }
        while (0 != num_generate--) {
            generate();
            print(stdout, title);
        }
        cleanup_curses_and_more();
        return 0;
    }

    if (0 == opt_random && 0 == precanned) {
        fprintf(stderr, "Error: option -n requires precanned boards\n");
        cleanup_curses_and_more();
        return 1;
    }

    if (!isatty(0) || !isatty(1)) {
        fprintf(stderr, "Error: stdin/out cannot be redirected\n");
        cleanup_curses_and_more();
        return 1;
    }

    /* Establish process environment */
    if (0 == initscr()) {
        fprintf(stderr,
                 "Error: failed to initialise curses screen library\n");
        cleanup_curses_and_more();
        return 1;
    }

    /* Any signal will now shutdown curses cleanly */
    signals();

    draw_screen();
    noecho();
    cbreak();
    load_board();
    play_the_game();
    cleanup_curses_and_more();
    return 0;
}
