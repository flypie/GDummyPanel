
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __CYGWIN__
#include <termios.h>
#endif
#include <curses.h>
#include "Button.h"

Button::Button()
{
	Next = NULL;
	Out = false;

	BG = new char[10]{ "   " };
	Text = new char[10];
}

Button::Button(int inx, int  iny, int  inh, int  inw, int  ini)
{
	Next = NULL;
	Out = false;

	BG = new char[10]{ "   " };
	Text = new char[10];

	x = inx;
	y = iny;
	h = inh;
	w = inw;
	iData = ini;

	sprintf_s(Text, 10, "%d", iData);

	Win = newwin(h, w, y,x);

}

void Button::draw()
{
	if (Selected)
	{
		wattron(Win, COLOR_PAIR(4) | A_REVERSE);
	}

	wmove(Win, 1, 1);
	wprintw(Win, BG);

	wmove(Win, 1, 1 + ((w - 2) - (int)strlen(Text)) / 2);
	wprintw(Win, Text);

	if (Selected)
	{
		wattroff(Win, COLOR_PAIR(0) | A_REVERSE);
	}
	wrefresh(Win);
}


void Button::SetSelected(BOOLEAN In)
{
	Selected =In;
}


void Button::SetOut(BOOLEAN In)
{
	Out = In;

	wattron(Win, COLOR_PAIR(4) | A_REVERSE);
	box(Win, 0, 0);
	wattroff(Win, COLOR_PAIR(4) | A_REVERSE);
}

BOOLEAN Button::GetOut()
{
	return Out;
}

BOOLEAN Button::GetSelected()
{
	return Selected;
}

int		Button::GetiData()
{
	return iData;
}
