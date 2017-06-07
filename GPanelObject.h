#pragma once


class GPanelObject
{
public:
    GPanelObject();
    virtual ~GPanelObject();

    bool    virtual HandleEvent(EVENTTYPE A, MEVENT &event) = 0;
    void    virtual Draw() = 0;
    WINDOW  virtual *GetWindow() = 0;

    GPanelObject    *GetNext() { return Next; };

private:
    GPanelObject    *Next;

};

