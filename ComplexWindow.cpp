/*
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

//#define NCURSES_INTERNALS 1 //Need to see some of the internals.

#include "Fudge.h"

#include "GPanelObject.h"

#include "Button.h"
#include "NumBox.h"
#include "ComplexWindow.h"
#include "GPIO.h"
#include "Dummy-Panel.h"

#include <typeinfo>

ComplexWindow::ComplexWindow(int height,int width,int starty,int startx)
{
    Inner = newwin(height,width,starty,startx);
    wrefresh(Inner); /* Show that box 		*/

    rectangle(Inner,0,0,height - 1,width - 1);

    nodelay(Inner,TRUE);

    keypad(Inner,TRUE);

    ObjectList = NULL;

    scrollok(Inner,TRUE);

    StrArr = new char*[height - 2];
    Pos = 0;

    this->startx = startx;
    this->starty = starty;
    this->height = height;
    this->width = width;
}

ComplexWindow::~ComplexWindow()
{
    wrefresh(Inner);
    delwin(Inner);

    delete StrArr;
}

void ComplexWindow::add_object(WindowObject *In)
{
    WindowObject *Cur;
    LOCKMUTEX(curseslock);

    if (!ObjectList) {
        ObjectList = In;
    } else {
        Cur = ObjectList;
        while (Cur->Next != 0) {
            Cur = Cur->Next;
        };
        Cur->Next = In;
    }

    UNLOCKMUTEX(curseslock);
}

WindowObject *ComplexWindow::find_object(int x,int y)
{
    WindowObject *Cur = NULL;
    bool NotFound = true;

    LOCKMUTEX(curseslock);

    if (!ObjectList) {
    } else {
        Cur = ObjectList;

        if (x >= Cur->x &&
            y >= Cur->y &&
            x < Cur->x + Cur->w &&
            y < Cur->y + Cur->h) {
            NotFound = FALSE;
        } else {
            while (Cur->Next != 0 && NotFound) {
                Cur = Cur->Next;
                if (x >= Cur->x &&
                    y >= Cur->y &&
                    x < Cur->x + Cur->w &&
                    y < Cur->y + Cur->h) {
                    NotFound = FALSE;
                }
            }
        }
    }

    if (NotFound) {
        Cur = NULL;
    }

    UNLOCKMUTEX(curseslock);

    return Cur;
}

WindowObject *ComplexWindow::find_object_handle(int Handle)
{
    WindowObject *Cur = NULL;
    bool NotFound = true;

    LOCKMUTEX(curseslock);

    Cur = ObjectList;

    while (Cur != 0 && NotFound) {
        if (Handle == Cur->Handle) {
            NotFound = FALSE;
        } else {
            Cur = Cur->Next;
        }
    }

    UNLOCKMUTEX(curseslock);

    return Cur;
}

void ComplexWindow::DeleteObjects(size_t Type,int Num)
{
    WindowObject *Cur,*Del;

    LOCKMUTEX(curseslock);

    if (Num == -1) {
        Cur = ObjectList;
        while (Cur != 0) {
            if (Type == typeid (*Cur).hash_code()) {
                if (Cur == ObjectList) {
                    ObjectList = Cur->Next;
                }
                Del = Cur;
                Cur = Cur->Next;

                delete Del;
            } else {
                Cur = Cur->Next;
            }
        };
    } else {
        for (int i = 0; i < Num; i++) {
            Cur = ObjectList;
            Del = 0;
            WindowObject *TPrev = 0;
            WindowObject *Prev = 0;

            while (Cur != 0) {
                if (Type == typeid (*Cur).hash_code()) {
                    Prev = TPrev;
                    Del = Cur;
                }
                TPrev = Cur;
                Cur = Cur->Next;
            };

            if (Del) {
                if (Del == ObjectList) {
                    ObjectList = Del->Next;
                } else {
                    Prev->Next = Del->Next;
                }
                delete Del;
            }
        }
    }

    UNLOCKMUTEX(curseslock);
}

void ComplexWindow::complexresize(int height,int width)
{
    LOCKMUTEX(curseslock);

    wresize(Inner,height,width);
    rectangle(Inner,starty,startx,starty + height - 1,startx + width - 1);
    wrefresh(Inner);
    UNLOCKMUTEX(curseslock);
}

void ComplexWindow::mvwin(int height)
{
    LOCKMUTEX(curseslock);

    ::mvwin(Inner,height / 2,0);

    UNLOCKMUTEX(curseslock);

}

void ComplexWindow::DoSpinner()
{
    static int count = 0;

    LOCKMUTEX(curseslock);

    count++;

    wattron(Inner,COLOR_PAIR(2 + count % 6));

    switch (count % 4)
    {
        case 0:
            mvwprintw(Inner,0,0,"|");
            break;
        case 1:
            mvwprintw(Inner,0,0,"\\");
            break;
        case 2:
            mvwprintw(Inner,0,0,"-");
            break;
        case 3:
            mvwprintw(Inner,0,0,"/");
            break;
    }

    wattroff(Inner,COLOR_PAIR(count % 8));
    wrefresh(Inner);

    UNLOCKMUTEX(curseslock);
}

void ComplexWindow::Draw()
{
    WindowObject *Obj = ObjectList;

    while (Obj != NULL) {
        if (Button * But = dynamic_cast<Button*>(Obj)) {
            But->Draw();
        } else if (NumBox * NumB = dynamic_cast<NumBox*>(Obj)) {
            NumB->Draw();
        }
        Obj = Obj->Next;
    }
}

int ComplexWindow::_getch()
{
    int val;
    LOCKMUTEX(curseslock);

    val = wgetch(Inner);

    UNLOCKMUTEX(curseslock);
    return val;
}

bool ComplexWindow::HandleEvent(EVENTTYPE c,MEVENT &event)
{
    bool Handled = false;

    LOCKMUTEX(curseslock);

    switch (c)
    {
        case KEY_MOUSE:
            /* When the user clicks left mouse button */
            if (event.x >= startx &&
                event.y >= starty &&
                event.x <= startx + width &&
                event.y <= starty + height
                ) { //We are in the window
                if (event.bstate & BUTTON1_DOUBLE_CLICKED || event.bstate & BUTTON1_CLICKED) {
                    WindowObject *Obj;
                    Obj = find_object(event.x - startx,event.y - starty);
                    if (Obj) {
                        MEVENT LEvent = event;
                        LEvent.x = event.x - startx;
                        LEvent.y = event.y - starty;

                        if (Button * But = dynamic_cast<Button*>(Obj)) {
                            Handled = But->HandleEvent(c,LEvent);
                        } else if (NumBox * NumB = dynamic_cast<NumBox*>(Obj)) {
                            Handled = NumB->HandleEvent(c,LEvent);
                        }
                    }
                }
            } else {
            }
            break;

        case 'X':
        case 'x':
            log_win->printw("KEY_CODE_YES\n");
            log_win->refresh();
            GlobalExit = true;
            Handled = true;
            break;

        case ERR:
            //		wprintw(log_win->Inner, choice);
            break;

        default:
            break;
    }

    UNLOCKMUTEX(curseslock);

    return Handled;
}

void ComplexWindow::refresh()
{
    LOCKMUTEX(curseslock);

    //    wrefresh(Outer);
    wrefresh(Inner);

    WindowObject *Obj = ObjectList;

    while (Obj != NULL) {
        Obj->Refresh();
        Obj = Obj->Next;
    }

    UNLOCKMUTEX(curseslock);

}

void ComplexWindow::printw(const char *format,...)
{
    char buffer[256];
    va_list args;
    va_start(args,format);

    LOCKMUTEX(curseslock);

    wattrset(Inner,CURSESWHITEONBLACK);

    vsprintf(buffer,format,args);

    if (Pos == height - 2) {
        delete StrArr[0];
        for (int i = 1; i < Pos; i++) {
            StrArr[i - 1] = StrArr[i];
        }
        Pos--;
    }
    StrArr[Pos] = new char[strlen(buffer) + 1];
    strcpy(StrArr[Pos++],buffer);

    for (int i = 0; i < Pos; i++) {
        mvwaddstr(Inner,i + 1,1,StrArr[i]);
        waddnstr(Inner,"                                                             ",width - strlen(StrArr[i]) - 2);
    }

    UNLOCKMUTEX(curseslock);

    va_end(args);
}

void ComplexWindow::Touchln(int y,int numy)
{
    WindowObject *Cur = NULL;
    bool NotFound = true;

    LOCKMUTEX(curseslock);

    wtouchln(Inner,y,numy,true);

    Cur = ObjectList;
    /*
        while (Cur) {
            int a = max(Cur->y, y);
            int b = min(Cur->y + Cur->h, y + numy);

            if (a <= b) {
                touchwin(Cur->Win);
            }
            Cur = Cur->Next;
        }
     */
    UNLOCKMUTEX(curseslock);
}