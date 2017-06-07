#pragma once
class WindowObject
{
    friend class NumBox;
    friend class Button;
    friend class ComplexWindow;

public:
    WindowObject();
    WindowObject(class ComplexWindow *PWin, int inx, int  iny, int  inw, int  inh);

    virtual ~WindowObject();

    int     GetHandle() { return Handle; };

    virtual bool HandleEvent(EVENTTYPE A, MEVENT &event) = 0;

    virtual void Draw();
    virtual void Refresh();

    virtual void SetIntValue(int A) = 0;

private:
    struct _win	*Win;

    bool	Selected;
    WindowObject *Next;

    bool	Out;
    bool	In;

    int		x, y, w, h;
    char	*Text;
    char	*BG;

    bool	Enabled;

    int     Handle;

    static int GHandle;
};

