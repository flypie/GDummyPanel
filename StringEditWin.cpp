#include "Fudge.h"

#include <ctype.h>
#include <curses.h>
#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "WindowObject.h"
#include "StringEditWin.h"

StringEditWin::StringEditWin(WindowObject *WinObj, char *descIn, char *bufIn, int MaxLenIn)
{
    int i, l, mmax = 0;

    defdisp = TRUE;
    insert = FALSE;
    Numeric = false;

    desc = descIn;
    Buffer = bufIn;
    MaxLen = MaxLenIn;

    CallingObj = WinObj;

//    getstrings

    if ((l = strlen(desc)) > mmax)
        mmax = l;

    nlines = 3;
    ncols = mmax + MaxLenIn + 4;

    LOCKMUTEX(curseslock);

    getyx(stdscr, oldy, oldx);

    getmaxyx(stdscr, maxy, maxx);

    StartY = (maxy - nlines) / 2;

    wmove(stdscr, StartY, (maxx - ncols) / 2);

    int cury, curx, begy, begx;

    getyx(stdscr, cury, curx);
    getbegyx(stdscr, begy, begx);

    winput = newwin(nlines, ncols, begy + cury, begx + curx);

    colorbox(winput, BLACKONRED, 1);

    mvwprintw(winput, 1, 2, "%s", desc);

//    mvweditstr 

    bp = Buffer;

    strcpy(org, Buffer);   /* save original */

    wrefresh(winput);

    getyx(winput, cury, curx);
    getbegyx(winput, begy, begx);

    wedit = subwin(winput, 1, MaxLenIn, begy + cury, begx + curx);

    colorbox(wedit, REDONBLACK, 0);

    keypad(wedit, TRUE);
    curs_set(0);

    Draw();

// End

    Panel = new GPanel;
    Panel->Add(this);

    Panel->SetTop();

    update_panels();

    /* Show it on the screen */
    doupdate();

    UNLOCKMUTEX(curseslock);
}

StringEditWin::~StringEditWin()
{
    LOCKMUTEX(curseslock);
    delwin(wedit);
    delwin(winput);

    wtouchln(stdscr, StartY, nlines, true);

    UNLOCKMUTEX(curseslock);
}

void    StringEditWin::Draw()
{
#ifndef PDCURSES
    int maxy;
#endif
    int maxx;

    int x = bp - Buffer;

    LOCKMUTEX(curseslock);

#ifdef PDCURSES
    maxx = getmaxx(wedit);
#else
    getmaxyx(win, maxy, maxx);
#endif
    werase(wedit);
    mvwprintw(wedit, 0, 0, "%s", padstr(Buffer, maxx));
    wmove(wedit, 0, x);
    wrefresh(wedit);

    UNLOCKMUTEX(curseslock);
}

WINDOW  *StringEditWin::GetWindow()
{
    return winput;
}

bool StringEditWin::HandleEvent(EVENTTYPE c, MEVENT &event)
{
    LOCKMUTEX(curseslock);

    switch (c) {
    case ERR:
        break;

    case KEY_ESC:
        Panel->SetStop(TRUE);
        break;

    case '\n':
#ifdef KEY_A2
    case KEY_A2:
#endif
    case KEY_UP:
#ifdef KEY_C2
    case KEY_C2:
#endif
    case KEY_DOWN:
        CallingObj->SetIntValue(atoi(Buffer));
        Panel->SetStop(TRUE);
        break;

#ifdef KEY_B1
    case KEY_B1:
#endif
    case KEY_LEFT:
        if (bp > Buffer)
            bp--;
        break;

#ifdef KEY_B3
    case KEY_B3:
#endif
    case KEY_RIGHT:
        defdisp = FALSE;
        if (bp - Buffer < (int)strlen(Buffer))
            bp++;
        break;

    case '\t':            /* TAB -- because insert
                          is broken on HPUX */
    case KEY_IC:          /* enter insert mode */
    case KEY_EIC:         /* exit insert mode */
        defdisp = FALSE;
        insert = !insert;

        curs_set(insert ? 2 : 1);
        break;

    case KEY_MOUSE:
        /*Filter this out as isprint goes crasy*/
        break;

    default:
        if (c == erasechar())       /* backspace, ^H */
        {
            if (bp > Buffer) {
                memmove((void *)(bp - 1), (const void *)bp, strlen(bp) + 1);
                bp--;
            }
        }
        else if (c == killchar())   /* ^U */
        {
            bp = Buffer;
            *bp = '\0';
        }
        else if (c == wordchar())   /* ^W */
        {
            tp = bp;

            while ((bp > Buffer) && (*(bp - 1) == ' '))
                bp--;
            while ((bp > Buffer) && (*(bp - 1) != ' '))
                bp--;

            memmove((void *)bp, (const void *)tp, strlen(tp) + 1);
        }
        else if ((c < KEY_OFFSET || c > KEY_MAX) && iswprint(c) && (!Numeric || iswxdigit(c))) {
            if (defdisp) {
                bp = Buffer;
                *bp = '\0';
                defdisp = FALSE;
            }

            if (insert) {
                if ((int)strlen(Buffer) < MaxLen - 1) {
                    memmove((void *)(bp + 1), (const void *)bp,
                        strlen(bp) + 1);

                    *bp++ = c;
                }
            }
            else if (bp - Buffer < MaxLen - 1) {
                /* append new string terminator */

                if (!*bp)
                    bp[1] = '\0';

                *bp++ = c;
            }
        }
    }

    Draw();

    UNLOCKMUTEX(curseslock);

    return true;
}
