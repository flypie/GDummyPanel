#include "Fudge.h"


#include "GPanel.h"


GPanel *GPanel::List = NULL;

GPanel::GPanel()
{
    GPanel *Temp;

    Objects = 0;
    stop = false;

    Panel = 0;

    if (List == NULL) {
        List = this;
        Next = 0;
    }
    else {
        Temp = List;
        List = this;
        List->Next = Temp;
    }
}


GPanel::~GPanel()
{
    GPanel *Cur, *Prev;
    if (List == NULL) {
        /* Error bug time*/
    }
    else if (List = this) {
        List = this->Next;

    } else {
        Prev = List;
        Cur = List->Next;
        while (Cur!=this) {
            Prev = Cur;
            Cur = Cur->Next;
        }
        Prev->Next = Cur->Next;
    }

    GPanelObject    *OCur,*Tmp;

    OCur = Objects;

    while (OCur) {
        Tmp = OCur->GetNext();
        delete OCur;
        OCur = Tmp;
    }

    if (Panel) {
        del_panel(Panel);
    }

    update_panels();

    /* Show it on the screen */
    doupdate();

}

void GPanel::Add(GPanelObject *In)
{
    if (Panel == 0) {
        Panel = new_panel(In->GetWindow());
    }

    if (Objects == 0) {
        Objects=In;
    }
}

void GPanel::SetTop()
{
    top_panel(Panel);
}



bool GPanel::HandleEvent(EVENTTYPE A,MEVENT &event)
{
    bool Handled = false;
    if (Objects) {
        Handled = Objects->HandleEvent(A, event);
    }
    return Handled; /* We have handled nothing */
}