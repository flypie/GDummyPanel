#pragma once


class Button
{
	friend class ComplexWindow;

public:
	Button();
	Button(int inx, int  iny, int  inw, int  inh, int  ini);


	void draw();
	void SetSelected(bool In);
	void SetOut(bool In);

	bool GetSelected();
	bool GetOut();
	int		GetiData();

private:

	int		x, y, w, h;
	char	*Text;
	char	*BG;
	bool	Selected;
	Button *Next;
	int		iData;
	WINDOW	*Win;
	bool	Out;
};



