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

#define NCURSES_INTERNALS 1 //Need to see some of the internals.

#include "Fudge.h"

#include "Button.h"
#include "ComplexWindow.h"
#include "GPIO.h"
#include "Dummy-Panel.h"


ComplexWindow::ComplexWindow(int height, int width, int starty, int startx)
{
    Outer = newwin(height, width, starty, startx);

    Inner = newwin(height - 3, width - 2, starty + 1, startx + 1);
    wrefresh(Inner);		/* Show that box 		*/

    addbox();

    nodelay(Outer, TRUE);
    nodelay(Inner, TRUE);

    keypad(Inner, TRUE);

    ButtonList = NULL;

    scrollok(Inner, TRUE);
}

ComplexWindow::~ComplexWindow()
{
    /* box(local_win, ' ', ' '); : This won't produce the desired
    * result of erasing the window. It will leave it's four corners
    * and so an ugly remnant of window.
    */

    wrefresh(Inner);
    delwin(Inner);

    removebox();

    wrefresh(Outer);
    delwin(Outer);
}



void ComplexWindow::addbox()
{
    LOCKMUTEX

    box(Outer, 0, 0);

    wrefresh(Outer);		/* Show that box */
    
    UNLOCKMUTEX    
}


void ComplexWindow::add_button(Button *In)
{
    Button	*Cur;
    LOCKMUTEX

    if (!ButtonList) {
        ButtonList = In;
    }
    else {
        Cur = ButtonList;
        while (Cur->Next != 0) {
            Cur = Cur->Next;
        };
        Cur->Next = In;
    }

    UNLOCKMUTEX    
}

Button *ComplexWindow::find_button(int x, int y)
{
    Button	*Cur = NULL;
    bool	NotFound = true;

    LOCKMUTEX

    if (!ButtonList) {
    }
    else {
        Cur = ButtonList;

        if (x >= Cur->x &&
            y >= Cur->y &&
            x < Cur->x + Cur->w - 1 &&
            y < Cur->y + Cur->h - 1) {
            NotFound = FALSE;
        }
        else {
            while (Cur->Next != 0 && NotFound) {
                Cur = Cur->Next;
                if (x >= Cur->x &&
                    y >= Cur->y &&
                    x < Cur->x + Cur->w - 1 &&
                    y < Cur->y + Cur->h - 1) {
                    NotFound = FALSE;
                }
            }
        }
    }

    if (NotFound) {
        Cur = NULL;
    }

    UNLOCKMUTEX    

    return Cur;
}

Button *ComplexWindow::find_button_data(int data)
{
    Button	*Cur = NULL;
    bool	NotFound = true;

    LOCKMUTEX

    Cur = ButtonList;

    while (Cur != 0 && NotFound) {
        if (data == Cur->iData) {
            NotFound = FALSE;
        }
        else {
            Cur = Cur->Next;
        }
    }

    UNLOCKMUTEX    

    return Cur;
}

void ComplexWindow::DeleteButtons()
{
    Button	*Cur, *Del;

    LOCKMUTEX

    Cur = ButtonList;
    while (Cur != 0) {
        Del = Cur;
        Cur = Cur->Next;

        delete Del;
    };
    ButtonList = NULL;
    
    UNLOCKMUTEX    
}

void ComplexWindow::removebox()
{
    LOCKMUTEX

    wborder(Outer, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    /* The parameters taken are
    * 1. win: the window on which to operate
    * 2. ls: character to be used for the left side of the window
    * 3. rs: character to be used for the right side of the window
    * 4. ts: character to be used for the top side of the window
    * 5. bs: character to be used for the bottom side of the window
    * 6. tl: character to be used for the top left corner of the window
    * 7. tr: character to be used for the top right corner of the window
    * 8. bl: character to be used for the bottom left corner of the window
    * 9. br: character to be used for the bottom right corner of the window
    */

    UNLOCKMUTEX    
}

void ComplexWindow::complexresize(int height, int width)
{
    LOCKMUTEX

    wresize(Outer, height, width);
    wresize(Inner, height - 2, width - 2);
    wclear(Outer);
    addbox();
    wrefresh(Outer);
    wrefresh(Inner);

    UNLOCKMUTEX    
}

void ComplexWindow::mvwin(int height)
{
    LOCKMUTEX

    ::mvwin(Outer, height / 2, 0);
    ::mvwin(Inner, height / 2 + 1, 1);

    UNLOCKMUTEX    

}

void ComplexWindow::DoSpinner()
{
    static int count = 0;

    LOCKMUTEX

    count++;

    wmove(Outer, 0, 0);

    wattron(Outer, COLOR_PAIR(count % 8));

    switch (count % 4) {
    case 0:
        wprintw(Outer, "|");
        break;
    case 1:
        wprintw(Outer, "\\");
        break;
    case 2:
        wprintw(Outer, "-");
        break;
    case 3:
        wprintw(Outer, "/");
        break;
    }

    wattroff(Outer, COLOR_PAIR(count % 8));
    wrefresh(Outer);

    UNLOCKMUTEX    
}

int ComplexWindow::_getch()
{
    int val;
    LOCKMUTEX

    val=wgetch(Inner);

    UNLOCKMUTEX    
    return val;
}

void ComplexWindow::Display()
{
    MEVENT event;
    int c;
    LOCKMUTEX

    c = wgetch(Inner);

    switch (c) {
    case KEY_MOUSE:
#ifdef _POSIX_VERSION
        if (getmouse(&event) == OK) {
#else
        if (nc_getmouse(&event) == OK) {
#endif                
            /* When the user clicks left mouse button */
            if (event.x >= Inner->_begx &&
                event.y >= Inner->_begy &&
                event.x < Inner->_maxx &&
                event.y < Inner->_maxy
                ) { //We are in the window
                if (event.bstate & BUTTON1_DOUBLE_CLICKED || event.bstate & BUTTON1_CLICKED) {
                    Button *But;
                    But = find_button(event.x, event.y);

                    if (But && !But->Out && But->Enabled) {
                        But->SetInput(true);
                        But->Selected = !But->Selected;
                        But->draw();

                        GPIOs->SetStatus(But->iData, But->Selected, true);

                        log_win->printw("Mouse: X %d Y %d Id %s\n", event.x, event.y, But->Text);
                        log_win->refresh();
                    }
                }
            }
            else {
                log_win->printw("Not in WIndow\n");
                log_win->refresh();
            }
        }
        break;
    case 'X':
    case 'x':
        log_win->printw("KEY_CODE_YES\n");
        log_win->refresh();
        GlobalExit = true;
        break;
    case ERR:
        //		wprintw(log_win->Inner, choice);
        break;

    default:
        break;
    }

    UNLOCKMUTEX    
}

void ComplexWindow::refresh()
{
    LOCKMUTEX

    wrefresh(Inner);

    UNLOCKMUTEX    

}

void ComplexWindow::printw(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);

    LOCKMUTEX

    vsprintf(buffer, format, args);
    ::wprintw(Inner, buffer);
    va_end(args);

    UNLOCKMUTEX    
}
