#pragma once


#include "GPanel.h"
#include "GPanelObject.h"

#define KEY_ESC    0x1b     /* Escape */

class StringEditWin :
    public GPanelObject
{
public:
    StringEditWin(WindowObject *A, char *descIn, char *bufIn, int MaxLenIn);
    ~StringEditWin();

    bool    HandleEvent(EVENTTYPE c, MEVENT &event);
    void    Draw();
    WINDOW  *GetWindow();

private:
    WINDOW *winput;
    WINDOW  *wedit;

    GPanel  *Panel;

    int oldy, oldx, maxy, maxx, nlines, ncols;
    char org[MAXSTRLEN], *tp,*bp;

    bool defdisp, insert;

    char *Buffer;


    bool    Numeric;
    int     StartY;

    char *desc;
    int MaxLen;

    WindowObject *CallingObj;
};

