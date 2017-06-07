#include "Fudge.h"


void rectangle(WINDOW *Win, int y1, int x1, int y2, int x2)
{
    wmove(Win, y1, x1 + 1);
    whline(Win, 0, x2 - x1 - 1);

    wmove(Win, y2, x1 + 1);
    whline(Win, 0, x2 - x1 - 1);

    wmove(Win, y1 + 1, x1);
    wvline(Win, 0, y2 - y1 - 1);

    wmove(Win, y1 + 1, x2);
    wvline(Win, 0, y2 - y1 - 1);

    wmove(Win, y1, x1);
    waddch(Win, ACS_ULCORNER);

    wmove(Win, y2, x1);
    waddch(Win, ACS_LLCORNER);

    wmove(Win, y1, x2);
    waddch(Win, ACS_URCORNER);

    wmove(Win, y2, x2);
    waddch(Win, ACS_LRCORNER);
}


char *padstr(char *s, int length)
{
    static char buf[MAXSTRLEN];
    char fmt[10];

    sprintf(fmt, (int)strlen(s) > length ? "%%.%ds" : "%%-%ds", length);
    sprintf(buf, fmt, s);

    return buf;
}

void colorbox(WINDOW *win, chtype color, int hasbox)
{
    int maxy;
#ifndef PDCURSES
    int maxx;
#endif

    wattrset(win, color);

#ifdef A_COLOR
    if (has_colors())
        wbkgd(win, color);
    //    else
#endif
    //        wbkgd(win, attr);

    werase(win);

#ifdef PDCURSES
    maxy = getmaxy(win);
#else
    getmaxyx(win, maxy, maxx);
#endif
    if (hasbox && (maxy > 2))
        box(win, 0, 0);

    touchwin(win);
    wrefresh(win);
}
