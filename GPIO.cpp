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

#include <stdio.h>
#include <string.h>

#include "curses.h"

#include "Fudge.h"

#include "Button.h"
#include "ComplexWindow.h"

#include "GPIO.h"

GPIO::GPIO(int Size)
{
    NeedsSend = true;
    Status = new bool[Size];
    memset(Status, false, sizeof(Status[0])*Size);
}

GPIO::~GPIO()
{
    NeedsSend = true;
}


void GPIO::SetStatus(int i, bool StatusIn, bool FromPanel)
{
    if (Status[i] != StatusIn) {
        Status[i] = StatusIn;
        if (FromPanel) {
            NeedsSend = true;
        }
    }
};

bool GPIO::NeedSending()
{
    return NeedsSend;
};