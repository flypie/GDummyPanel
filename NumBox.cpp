#include "Fudge.h"

#include <string>     // std::string, std::stoi

#include "GPanelObject.h"
#include "NumBox.h"
#include "ComplexWindow.h"
#include "StringEditWin.h"
#include "GPIO.h"
#include "Dummy-Panel.h"

int NumBox::NumObjects = 0;

NumBox::NumBox()
{
    Value = 0;
    NumObjects++;
}

NumBox::NumBox(ComplexWindow *PWin, int startx, int starty, int width, int height,int val) : WindowObject(PWin, startx, starty, width ,height)
{
    Value = val;
    NumObjects++;
}

NumBox::~NumBox()
{
    NumObjects--;
}

void NumBox::Draw()
{
    LOCKMUTEX(curseslock);

    wattrset(Win, CURSESWHITEONBLACK);

    wmove(Win, 1+y, 1+x);
    waddch(Win,ACS_UARROW);

    snprintf(Text, 10, "%03d", Value);
    wmove(Win,y + 1,x + 1 + ((w - 2) - (int)strlen(Text)) / 2);
    wprintw(Win, Text);

    wmove(Win,y + 1,x + w-2);
    waddch(Win, ACS_DARROW);

    UNLOCKMUTEX(curseslock);
}

bool NumBox::HandleEvent(EVENTTYPE A,MEVENT &event)
{
    if (!Out && Enabled) {
        int xchar = event.x - x;

        if (xchar == 1) {
            Value++;
        }
        else if (xchar == w - 2) {
            Value--;
        }
        else if (xchar > 1 && xchar < w - 2) {
            OpenEditWin = new StringEditWin(this, "Enter Value:", Text, 4);
        }
        
        Draw();
        
        update_panels();
    }
    return true; /* We have handled it */
}
