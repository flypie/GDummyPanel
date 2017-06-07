#pragma once


#include "WindowObject.h"

class NumBox : public WindowObject
{
public:
    NumBox();
    NumBox(ComplexWindow *PWin, int startx, int starty, int width, int height,int val);

    virtual ~NumBox();

    void    Draw();

    bool HandleEvent(EVENTTYPE A, MEVENT &event);

    void SetIntValue(int A) {
        Value = A;
        Draw();
        doupdate();
    };

    static int GetNumObjects() {
        return NumObjects;
    };

private:
    int Value;

    GPanelObject    *OpenEditWin;

    static int NumObjects;
};

