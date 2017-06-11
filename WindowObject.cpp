
#include "Fudge.h"

#include "GPanelObject.h"
#include "WindowObject.h"
#include "ComplexWindow.h"
#include "Dummy-Panel.h"

int WindowObject::GHandle = 0;

WindowObject::WindowObject()
{
    Next = NULL;
    Out = false;
    In = false;
    Enabled = true;

    Handle = GHandle++;

    BG = new char[10];
    strcpy(BG, "         ");
    Text = new char[10];

}

WindowObject::WindowObject(ComplexWindow *PWin,int inx, int  iny, int  inw, int  inh) :WindowObject()
{
    x = inx+1; //All windows have a border of 1x & 1y
    y = iny+1;
    h = inh;
    w = inw;

    BG[inw - 2] = 0;

    LOCKMUTEX(curseslock);

    Win = PWin->GetInner();

    rectangle(Win, y, x,y+h-1,x+w-1);

    UNLOCKMUTEX(curseslock);
}

WindowObject::~WindowObject()
{
}

void WindowObject::Draw()
{
    LOCKMUTEX(curseslock);

    if (!Enabled) {
        wattrset(Win, BLACKONRED);
    }
    else if (In) {
        wattrset(Win, BLACKONGREEN);
    }
    else if (Out) {
        wattrset(Win, BLACKONBLUE);
    }
    else {
        wattrset(Win, CURSESWHITEONBLACK);
    }

    rectangle(Win, y, x, y + h - 1, x + w - 1);

    if (!Selected) {

        wattrset(Win, CURSESWHITEONBLACK);
    }

    wmove(Win,y + 1,x + 1);
    wprintw(Win, BG);

    wmove(Win,y + 1,x + 1 + ((w - 2) - (int)strlen(Text)) / 2);
    wprintw(Win, Text);

    wrefresh(Win);

    update_panels();

    UNLOCKMUTEX(curseslock);
}

void WindowObject::Refresh()
{
    wrefresh(Win);
}