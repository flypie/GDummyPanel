
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __CYGWIN__
#include <termios.h>
#endif
#include "curses.h"
#include "Button.h"
#include "ComplexWindow.h"


extern BOOL	GPIOStatus[];


ComplexWindow::ComplexWindow(int height, int width, int starty, int startx)
{
	//	ComplexWindow *local_win=new ComplexWindow;

	Outer = newwin(height, width, starty, startx);

	Inner = newwin(height - 2, width - 2, starty + 1, startx + 1);
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
	box(Outer, 0, 0);

	wrefresh(Outer);		/* Show that box */
}


void ComplexWindow::add_button(Button *In)
{
	Button	*Cur;


	if (!ButtonList)
	{
		ButtonList = In;
	}
	else
	{
		Cur = ButtonList;
		while (Cur->Next != 0)
		{
			Cur = Cur->Next;
		};
		Cur->Next = In;
	}
}

Button *ComplexWindow::find_button(int x, int y)
{
	Button	*Cur = NULL;
	BOOL	NotFound = true;


	if (!ButtonList)
	{
	}
	else
	{
		Cur = ButtonList;

		if (x >= Cur->x &&
			y >= Cur->y &&
			x < Cur->x + Cur->w - 1 &&
			y < Cur->y + Cur->h - 1)
		{
			NotFound = FALSE;
		}
		else
		{
			while (Cur->Next != 0 && NotFound)
			{
				Cur = Cur->Next;
				if (x >= Cur->x &&
					y >= Cur->y &&
					x < Cur->x + Cur->w - 1 &&
					y < Cur->y + Cur->h - 1)
				{
					NotFound = FALSE;
				}
			};
		}
	}

	if (NotFound)
	{
		Cur = NULL;
	}

	return Cur;
}

Button *ComplexWindow::find_button_data(int data)
{
	Button	*Cur = NULL;
	BOOL	NotFound = true;


	if (!ButtonList)
	{
	}
	else
	{
		Cur = ButtonList;

		if (data == Cur->iData)
		{
			NotFound = FALSE;
		}
		else
		{
			while (Cur->Next != 0 && NotFound)
			{
				Cur = Cur->Next;
				if (data == Cur->iData)
				{
					NotFound = FALSE;
				}
			};
		}
	}

	if (NotFound)
	{
		Cur = NULL;
	}

	return Cur;
}


void ComplexWindow::removebox()
{
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

}

void ComplexWindow::complexresize(int height, int width)
{
	wresize(Outer, height, width);
	wresize(Inner, height - 2, width - 2);
	wclear(Outer);
	addbox();
	wrefresh(Outer);
	wrefresh(Inner);
}

void ComplexWindow::mvwin(int height)
{
	::mvwin(Outer, height / 2, 0);
	::mvwin(Inner, height / 2 + 1, 1);
}

void ComplexWindow::DoSpinner()
{
	static int count = 0;

	count++;

	wmove(Outer, 0, 0);

	wattron(Outer, COLOR_PAIR(count % 8));

	switch (count % 4)
	{
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
}

int ComplexWindow::_getch()
{
	return wgetch(Inner);
}



void ComplexWindow::Display()
{
	MEVENT event;
	int c;

	c = wgetch(Inner);

	switch (c)
	{
	case KEY_MOUSE:
		if (nc_getmouse(&event) == OK)
		{	/* When the user clicks left mouse button */
			if (event.x >= Inner->_begx &&
				event.y >= Inner->_begy &&
				event.x < Inner->_maxx &&
				event.y < Inner->_maxy
				)
			{ //We are in the window
				if (event.bstate & BUTTON1_DOUBLE_CLICKED || event.bstate & BUTTON1_CLICKED)
				{
					Button *But;
					But = find_button(event.x, event.y);

					if (But && !But->Out)
					{
						But->Selected = !But->Selected;
						But->draw();

						GPIOStatus[But->iData] = But->Selected;

//						wprintw(Inner, "Mouse: X %d Y %d Id %s\n", event.x, event.y, But->Text);
//						wrefresh(Inner);
					}
				}
			}
			else
			{
				//					wprintw(log_win->Inner, "Not in WIndow %d\n");
				//					wrefresh(log_win->Inner);
			}
		}
		break;
	case ERR:
		//			wprintw(log_win->Inner, choice);
		break;

	default:
//		wprintw(log_win->Inner, "wgetch %d\n", c);
//		wrefresh(log_win->Inner);
		break;

	}

//	wrefresh(panel_win->Inner);
}

void ComplexWindow::refresh()
{
	wrefresh(Inner);
}

void ComplexWindow::printw(const char *format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	::wprintw(Inner, buffer);
	va_end(args);

}
