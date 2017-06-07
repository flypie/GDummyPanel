#pragma once

#include "curses.h"
#include <panel.h>

#include "GPanelObject.h"

typedef struct _PANEL_DATA {
    int hide;	/* TRUE if panel is hidden */
}PANEL_DATA;



class GPanel
{
public:
    GPanel();
    virtual ~GPanel();

    virtual bool HandleEvent(EVENTTYPE A, MEVENT &event);

    GPanel* GetNext() { return Next; }

    void Add(GPanelObject *In);
    void SetTop();

    static GPanel *List;

    bool    GetStop() { return   stop; };
    void    SetStop(bool instop) { stop = instop; };

private:
    PANEL       *Panel;
    PANEL_DATA  Panel_data;

    GPanel      *Next;

    bool    stop;

    GPanelObject    *Objects;
};

