#pragma once


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
	void	destroy_win();
	void	complexresize(int height, int width);
	void	mvwin(int height);
	void	DoSpinner();
	int		_getch();
	void	Display();
	void	refresh();
	void	printw(const char *, ...);


private:
	WINDOW  *Outer;
	WINDOW  *Inner;

	Button *ButtonList;

};
