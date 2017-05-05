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

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __CYGWIN__
#include <termios.h>
#endif

#include "curses.h"

#include "Fudge.h"
#include "Button.h"


Button::Button()
{
    Next = NULL;
    Out = false;
    In = false;
    Enabled = true;

    BG = new char[10];
    strcpy(BG, "  ");
    Text = new char[10];
}

Button::Button(int inx, int  iny, int  inh, int  inw, int  ini):Button()
{
    Button(); // Call the basic constructor.

    x = inx;
    y = iny;
    h = inh;
    w = inw;
    iData = ini;
    Win = newwin(h, w, y, x);

    box(Win, 0, 0);

    snprintf(Text, 10, "%03d", iData);

}

void Button::draw()
{
    if (Selected) {
        if (Out) {
            wattron(Win, COLOR_PAIR(4) | A_REVERSE);
        }
        else {
            wattron(Win, COLOR_PAIR(2) | A_REVERSE);
        }
    }

    wmove(Win, 1, 1);
    wprintw(Win, BG);

    wmove(Win, 1, 1 + ((w - 2) - (int)strlen(Text)) / 2);
    wprintw(Win, Text);

    if (Selected) {
        wattroff(Win, A_REVERSE);
    }

    wattron(Win, COLOR_PAIR(7));

    wrefresh(Win);
}


void Button::SetSelected(bool In)
{
    Selected = In;
}


void Button::SetOutput(bool In)
{
    Out = In;

    wattron(Win, COLOR_PAIR(4) | A_REVERSE);
    box(Win, 0, 0);
    wattroff(Win, COLOR_PAIR(4) | A_REVERSE);
}

void Button::SetInput(bool InIn)
{
    In = InIn;

    wattron(Win, COLOR_PAIR(2) | A_REVERSE);
    box(Win, 0, 0);
    wattroff(Win, COLOR_PAIR(2) | A_REVERSE);
}

void Button::SetEnabled(bool InIn)
{
    Enabled = InIn;

    if (!Enabled) {
        wattron(Win, COLOR_PAIR(6) | A_REVERSE);
        box(Win, 0, 0);
        wattroff(Win, COLOR_PAIR(6) | A_REVERSE);
    }
}


bool Button::GetOut()
{
    return Out;
}

bool Button::GetSelected()
{
    return Selected;
}

int Button::GetiData()
{
    return iData;
}
