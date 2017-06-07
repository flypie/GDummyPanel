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

#include "Fudge.h"

#include "GPanelObject.h"
#include "Button.h"
#include "ComplexWindow.h"
#include "GPIO.h"
#include "Dummy-Panel.h"

int Button::NumObjects = 0;

Button::Button()
{
    NumObjects++;
}

Button::Button(ComplexWindow *PWin, int inx, int  iny, int  inw, int  inh,int ini):WindowObject(PWin,inx, iny, inw, inh)
{
    Value = ini;
    snprintf(Text, 10, "%03d", Value);
    NumObjects++;
}

Button::~Button()
{
    NumObjects--;
}

void Button::SetSelected(bool In)
{
    Selected = In;
}


void Button::SetOutput(bool In)
{
    Out = In;
/*
    LOCKMUTEX
    
    wattron(Win, COLOR_PAIR(4) | A_REVERSE);
    box(Win, 0, 0);
    wattroff(Win, COLOR_PAIR(4) | A_REVERSE);

    UNLOCKMUTEX
*/
}

void Button::SetInput(bool InIn)
{
    In = InIn;
/*
    LOCKMUTEX
        
    wattron(Win, COLOR_PAIR(2) | A_REVERSE);
    box(Win, 0, 0);
    wattroff(Win, COLOR_PAIR(2) | A_REVERSE);

    UNLOCKMUTEX     
*/
}

void Button::SetEnabled(bool InIn)
{
    Enabled = InIn;
/*

    if (!Enabled) {
        LOCKMUTEX
 
        wattron(Win, COLOR_PAIR(6) | A_REVERSE);
        box(Win, 0, 0);
        wattroff(Win, COLOR_PAIR(6) | A_REVERSE);
    
        UNLOCKMUTEX
    }
*/
}


bool Button::GetOut()
{
    return Out;
}

bool Button::GetSelected()
{
    return Selected;
}

bool Button::HandleEvent(EVENTTYPE A, MEVENT &event)
{
    if (!Out && Enabled) {
        SetInput(true);
        Selected = !Selected;
        
        Draw();

        GPIO::GPIOs->SetStatus(GetIntValue(), Selected, true);

        log_win->printw("Button:Mouse: X %d Y %d Id %s", event.x, event.y, Text);
        log_win->refresh();
    }

    return true; /* We Have Handled it */
}
