#pragma once


class Button
{
	friend class ComplexWindow;

public:
	Button();
	Button(int inx, int  iny, int  inw, int  inh, int  ini);


	void draw();
	void SetSelected(BOOLEAN In);
	void SetOut(BOOLEAN In);

	BOOLEAN GetSelected();
	BOOLEAN GetOut();
	int		GetiData();

private:

	int		x, y, w, h;
	char	*Text;
	char	*BG;
	BOOL	Selected;
	Button *Next;
	int		iData;
	WINDOW	*Win;
	BOOL	Out;
};



