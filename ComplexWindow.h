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

#ifdef _POSIX_VERSION
#include <pthread.h>
#else
#include <process.h>
#endif

class ComplexWindow
{
public:
	ComplexWindow();
	ComplexWindow(int height, int width, int starty, int startx);

	~ComplexWindow();
	void	addbox();
	void	add_button(Button *In);
	Button	*find_button(int x, int y);
	Button	*find_button_data(int data);
	void	removebox();
	void	complexresize(int height, int width);
	void	mvwin(int height);
	void	DoSpinner();
	int     _getch();
	void	Display();
	void	refresh();
	void	printw(const char *, ...);
	void	DeleteButtons();
private:
	WINDOW  *Outer;
	WINDOW  *Inner;

	Button *ButtonList;
};
