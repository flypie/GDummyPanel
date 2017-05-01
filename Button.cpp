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

	BG = new char[10];
	strcpy(BG, "  ");
	Text = new char[10];
}

Button::Button(int inx, int  iny, int  inh, int  inw, int  ini)
{
	Next = NULL;
	Out = false;

	BG = new char[10];
	strcpy(BG, "  ");
	Text = new char[10];

	x = inx;
	y = iny;
	h = inh;
	w = inw;
	iData = ini;
	Win = newwin(h, w, y, x);

	box(Win, 0, 0);

	snprintf(Text, 10, "%d", iData);

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


void Button::SetSelected(bool In)
{
	Selected = In;
}


void Button::SetOut(bool In)
{
	Out = In;

	wattron(Win, COLOR_PAIR(4) | A_REVERSE);
	box(Win, 0, 0);
	wattroff(Win, COLOR_PAIR(4) | A_REVERSE);
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
