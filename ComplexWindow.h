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

#pragma once

class ComplexWindow: public GPanelObject
{
public:
	ComplexWindow();
	ComplexWindow(int height, int width, int starty, int startx);

	~ComplexWindow();

    void	add_object(WindowObject *In);
    WindowObject	*find_object(int x, int y);
    WindowObject	*find_object_handle(int data);
    void	DeleteObjects(size_t Type,int Num);

	void	complexresize(int height, int width);
	void	mvwin(int height);
	void	DoSpinner();
	int     _getch();
    bool    HandleEvent(EVENTTYPE A,MEVENT &event);
	void	refresh();
	void	printw(const char *, ...);

    WINDOW  *GetInner() { return Inner; };
    WINDOW  *GetWindow() { return Inner; };

    void    Draw();
    void    Touchln(int x, int y);

private:
	WINDOW  *Inner;

    WindowObject *ObjectList;

    char    **StrArr;
    int     Pos;

 
};

