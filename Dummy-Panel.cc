
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

/* This is the server providing a way of interacting with the GPIO on
 *  the RASPBERRY PI it must be run before the emulation is started. It is
 *  very crude and only the start of the work
 * 
 * The modified qemu is https://github.com/flypie/flypie-pi-qemu
 * The file modified in qemu are 
 * hw/gpio/bcm2835_gpio.h
 * hw/gpio/bcm2835_gpio.c
 * 
 * added are
 * 
 * util/PanelEmu.c
 * include/qemu/PanelEmu.h
 */


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef __CYGWIN__
#include <termios.h>
#endif
#include <curses.h>

#define BUF_SIZE 255
#define	DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "45567"

typedef struct 
{
	SOCKET ClientSocket;
} ThreadData;

typedef enum
{
	MACHINEDESC = 0,
	PINSTATE = 1,
	READSTATE = 2,
	READRESPONSE =3
} PacketType;

#define MAXPACKET   255

#define PACKETLEN   0  //Includes Packet Length
#define PACKETTYPE  1

typedef struct
{
	unsigned short int Data[MAXPACKET];
} CommandPacket;


#define NUMPIPINS	54

BOOL	GPIOStatus[NUMPIPINS] = { 0 };

#ifdef __CYGWIN__
static struct termios oldc, newc;

/* Initialize newc terminal i/o settings */
void initTermios(int echo) 
{
  tcgetattr(0, &oldc); /* grab oldc terminal i/o settings */
  newc = oldc; /* make newc settings same as oldc settings */
  newc.c_lflag &= ~ICANON; /* disable buffered i/o */
  newc.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newc); /* use these newc terminal i/o settings now */
}

/* Restore oldc terminal i/o settings */
void resetTermios(void) 
{
  tcsetattr(0, TCSANOW, &oldc);
}

/* Read 1 character - echo defines echo mode */
char mygetch_(int echo)
{
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
char mygetch(void)
{
	return mygetch_(0);
}

#endif

typedef struct _Button
{
	int		x, y, w, h;
	char	*Text;
	char	*BG;
	BOOL	Selected;
	struct	_Button *Next;
	int		iData;
	WINDOW	*Win;
	BOOL	Out;
} Button;

//Button *ButtonList = 0;

typedef struct
{
    WINDOW  *Outer;
    WINDOW  *Inner;

	Button *ButtonList;

} COMPLEXWINDOW;

COMPLEXWINDOW   *log_win=(COMPLEXWINDOW   *)-1;
COMPLEXWINDOW   *panel_win=(COMPLEXWINDOW   *)-1;

int gstartx, gstarty, gwidth, gheight;

BOOL	NoPanel = TRUE;

short color_table[] =
{
    COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN,
    COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE
};


void addbox(COMPLEXWINDOW *local_win)
{	
    box(local_win->Outer, 0 , 0);		
    wrefresh(local_win->Outer);		/* Show that box */
}


void add_button(COMPLEXWINDOW *Win, Button *In)
{
	Button	*Cur;

	In->Next = NULL;
	if(!Win->ButtonList)
	{
		Win->ButtonList = In;
	}
	else
	{
		Cur = Win->ButtonList;
		while (Cur->Next != 0)
		{
			Cur = Cur->Next;
		};
		Cur->Next = In;
	}
}

Button *find_button(COMPLEXWINDOW *Win, int x,int y)
{
	Button	*Cur=NULL;
	BOOL	NotFound = true;


	if (!Win->ButtonList)
	{
	}
	else
	{
		Cur = Win->ButtonList;

		if (x >= Cur->x &&
			y >= Cur->y &&
			x < Cur->x+Cur->w-1 &&
			y < Cur->y+Cur->h-1)
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
					NotFound=FALSE;
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

Button *find_button_data(COMPLEXWINDOW *Win, int data)
{
	Button	*Cur = NULL;
	BOOL	NotFound = true;


	if (!Win->ButtonList)
	{
	}
	else
	{
		Cur = Win->ButtonList;

		if (data==Cur->iData)
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



void draw_button(Button *Cur)
{
	if(Cur->Selected)
	{
		wattron(Cur->Win, COLOR_PAIR(4) | A_REVERSE);
	}

	wmove(Cur->Win, 1, 1);
	wprintw(Cur->Win, Cur->BG);

	wmove(Cur->Win, 1, 1+((Cur->w - 2) - strlen(Cur->Text)) / 2);
	wprintw(Cur->Win, Cur->Text);

	if (Cur->Selected)
	{
		wattroff(Cur->Win, COLOR_PAIR(0) | A_REVERSE);
	}
	wrefresh(Cur->Win);
}


COMPLEXWINDOW *create_newwin(int height, int width, int starty, int startx)
{	
	COMPLEXWINDOW *local_win=new COMPLEXWINDOW;

	local_win->Outer = newwin(height, width, starty, startx);
       
	local_win->Inner = newwin(height-2, width-2, starty+1, startx+1);
    	wrefresh(local_win->Inner);		/* Show that box 		*/
    
	addbox(local_win);

	nodelay(local_win->Outer, TRUE);
	nodelay(local_win->Inner, TRUE);

	keypad(local_win->Inner, TRUE);

	local_win->ButtonList = NULL;

	return local_win;
}

void removebox(COMPLEXWINDOW *local_win)
{	
	wborder(local_win->Outer, ' ', ' ', ' ',' ',' ',' ',' ',' ');
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

void destroy_win(COMPLEXWINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
    
	wrefresh(local_win->Inner);
	delwin(local_win->Inner);
        
	removebox(local_win);
    
	wrefresh(local_win->Outer);
	delwin(local_win->Outer);
        
	delete local_win;
}

#define wresize resize_window 

void wcomplexresize(COMPLEXWINDOW *win,int height,int width)
{
    wresize(win->Outer, height, width);
    wresize(win->Inner, height-2, width-2);
    wclear(win->Outer); 
    addbox(win); 
    wrefresh(win->Outer);
    wrefresh(win->Inner);
}

void    checkandresize()
{
    int width, height;

    getmaxyx(stdscr,height,width);
    
    if(height!=gheight || width!=gwidth)
    {    
        gheight=height;
        gwidth=width;

        wclear(stdscr);
      
        wcomplexresize(log_win,gheight/2,gwidth);
  
        if(log_win!=(COMPLEXWINDOW *)-1)
        {
            wcomplexresize(log_win,gheight/2,gwidth);

            mvwin(log_win->Outer, gheight/2, 0);
            mvwin(log_win->Inner, gheight/2+1, 0);
        }
        wprintw(log_win->Inner,"Resize\n");

        wrefresh(log_win->Inner);
    }
    else
    {
       wrefresh(log_win->Inner);       
    }
}

typedef struct
{
	BOOL LED;

} PANELSTATUS;

PANELSTATUS	Panel;

void DoSpinner()
{
	static int count = 0;

	count++;

	wmove(panel_win->Outer,0,0);

	wattron(panel_win->Outer, COLOR_PAIR(count % 8));

	switch (count % 4)
	{
	case 0:
		wprintw(panel_win->Outer, "|");
		break;
	case 1:
		wprintw(panel_win->Outer, "\\");
		break;
	case 2:
		wprintw(panel_win->Outer, "-");
		break;
	case 3:
		wprintw(panel_win->Outer, "/");
		break;
	}

	wattroff(panel_win->Outer, COLOR_PAIR(count % 8));
	wrefresh(panel_win->Outer);
}


void DisplayPanel()
{
	MEVENT event;
	int c;
	
	c = wgetch(panel_win->Inner);

	switch (c)
	{
		case KEY_MOUSE:
			if (nc_getmouse(&event) == OK)
			{	/* When the user clicks left mouse button */
				if(	event.x>=panel_win->Inner->_begx &&
					event.y >= panel_win->Inner->_begy &&
					event.x < panel_win->Inner->_maxx &&
					event.y < panel_win->Inner->_maxy
					)
				{ //We are in the window
					if (event.bstate & BUTTON1_DOUBLE_CLICKED || event.bstate & BUTTON1_CLICKED)
					{
						Button *But;
						But=find_button(panel_win, event.x, event.y);

						if(But && !But->Out)
						{
							But->Selected = !But->Selected;
							draw_button(But);
							GPIOStatus[But->iData] = But->Selected;
							wprintw(log_win->Inner, "Mouse: X %d Y %d Id %s\n", event.x, event.y,But->Text);
							wrefresh(log_win->Inner);
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
			wprintw(log_win->Inner, "wgetch %d\n", c);
			wrefresh(log_win->Inner);
			break;

	}

	wrefresh(panel_win->Inner);
}

void CreateButtons()
{
	for (int i = 0; i < NUMPIPINS; i++)
	{
		Button *But = new Button;

		But->iData = i;
		But->x = 2 + 6 * (i%16);
		But->y = 1+(i/16)*3;
		But->h = 3;
		But->w = 5;
		But->Out = false;

		But->BG = new char[10]{ "   " };
		But->Text = new char[10];
		sprintf_s(But->Text, 10, "%d", i);


		But->Selected = GPIOStatus[i];
		But->Win = newwin(3, 5, But->y, But->x);

		if (But->Out)
			wattron(But->Win, COLOR_PAIR(4) | A_REVERSE);

		box(But->Win, 0, 0);

		if (But->Out)
			wattroff(But->Win, COLOR_PAIR(4) | A_REVERSE);

		add_button(panel_win, But);
		draw_button(But);
	}

wrefresh(panel_win->Inner);
}


DWORD WINAPI SlaveThread(LPVOID lpParam)
{
    ThreadData *TData;

    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    CommandPacket	*CurrentPkt;
        
	uint64_t Data;
	uint64_t state;

	// Cast the parameter to the correct data type.
    // The pointer is known to be valid because 
    // it was checked for NULL before the thread was created.

    TData = (ThreadData *)lpParam;
    // Receive until the peer shuts down the connection
    
	NoPanel = FALSE;

    checkandresize();    
       
    wprintw(log_win->Inner,"Slave Thread Started:\n");

    wrefresh(log_win->Inner);
    
    do 
    {
        checkandresize();
        
        iResult = recv(TData->ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            if (iResult < DEFAULT_BUFLEN)
            {
                    recvbuf[iResult] = 0;
            }
            else
            {
                    recvbuf[DEFAULT_BUFLEN - 1] = 0;
            }
            wprintw(log_win->Inner,"Bytes received: %d\n", iResult);
            wrefresh(log_win->Inner);
            CurrentPkt = (CommandPacket *)recvbuf;

            for (int i = 0; i < iResult; i += CurrentPkt->Data[PACKETLEN], CurrentPkt = (CommandPacket *)((char *)CurrentPkt+(CurrentPkt->Data[PACKETLEN])))
            {
                switch (CurrentPkt->Data[PACKETTYPE])
                {
                    case MACHINEDESC:
                        break;

                    case PINSTATE:
						Data = (uint64_t)CurrentPkt->Data[2] | (uint64_t)CurrentPkt->Data[3] << 16 | (uint64_t)CurrentPkt->Data[4] <<32| (uint64_t)CurrentPkt->Data[5] << 48;

						for (int j = 0; j < NUMPIPINS && Data; j++)
						{
							state = Data & 0x1;

							if (state )
							{
								Button *But;
								But = find_button_data(panel_win, j);


								if (!But->Out)
								{
									But->Out = true;
									
									wattron(But->Win, COLOR_PAIR(4) | A_REVERSE);

									box(But->Win, 0, 0);

									wattroff(But->Win, COLOR_PAIR(4) | A_REVERSE);
								}

								if(But)
								{
//									But->Out = true;
									But->Selected = CurrentPkt->Data[6];
									GPIOStatus[But->iData] = CurrentPkt->Data[6];
									draw_button(But);
								}
							}
							Data = Data >> 1;
						}
                        break;

                    case READSTATE:
                        CommandPacket	Send;
						Data = 0;

						for (int j = 0; j < NUMPIPINS;j++)
						{
							Data = Data << 1;
							if (GPIOStatus[NUMPIPINS-1-j])
							{
								Data |= 0x01;
							}
						}

						Send.Data[2] = (unsigned short)Data;
                        Send.Data[3] = (unsigned short)(Data>>16);
                        Send.Data[4] = (unsigned short)(Data>>32);
                        Send.Data[5] = (unsigned short)(Data>>48);

                        Send.Data[PACKETLEN] = sizeof(Send.Data[0])*6;
                        Send.Data[PACKETTYPE] = READRESPONSE;

                        send(TData->ClientSocket, (char *)&Send, Send.Data[PACKETLEN], 0);

                        wprintw(log_win->Inner,"Read State received.\n");
                        break;

                    default:
                        wprintw(log_win->Inner,"Unknown Packet Type\n");
                        break;
                }
            }
			DisplayPanel();
			wrefresh(log_win->Inner);
		}
        else if (iResult == 0)
        {
            wprintw(log_win->Inner,"Connection closing...\n");
        }
        else 
        {
            wprintw(log_win->Inner,"recv failed with error: %d\n", WSAGetLastError());
            closesocket(TData->ClientSocket);
        }
        wrefresh(log_win->Inner);
    } while (iResult > 0);


    // cleanup
    closesocket(TData->ClientSocket);

    delete (ThreadData*)lpParam;

//    destroy_win(log_win);
//    log_win=(COMPLEXWINDOW *)-1;
    
	NoPanel = TRUE;

    return 0;
}

#define DELAYSIZE 200



int main(int argc, char**argv) 
{
    int i;

    WSADATA wsaData;
    int     iResult;
    BOOL    EveythingOK=true;

    SOCKET  ListenSocket = INVALID_SOCKET;

    struct  addrinfo *result = NULL;
    struct  addrinfo hints;

    HANDLE  hThread;
    DWORD   ThreadId;

	int     startx, starty;

#ifdef XCURSES
    Xinitscr(argc, argv);
#else
    initscr();
#endif

	mousemask(ALL_MOUSE_EVENTS, 0);    /* The old events mask                */

	nodelay(stdscr, TRUE);
    noecho();
	curs_set(0);

    if (has_colors())
        start_color();

    for (i = 0; i < 8; i++)
    {
        init_pair(i, color_table[i], COLOR_BLACK);
    }
    
	/* Resize the terminal to something larger than the physical screen */
	resize_term(2000, 2000);

    getmaxyx(stdscr,gheight,gwidth);		/* get the number of rows and columns */

	resize_term(gheight/2, 16*6+3);

	getmaxyx(stdscr, gheight, gwidth);		/* get the number of rows and columns */

    starty = gheight / 2;	/* Calculating for a center placement */
    startx = 0;	/* of the window */

    log_win = create_newwin(gheight/2, gwidth, starty, startx);
    scrollok(log_win->Inner, TRUE);

	panel_win = create_newwin(gheight / 2, gwidth, 0, 0);
	scrollok(panel_win->Inner, TRUE);

	CreateButtons();

	if (has_colors())
    {
        wprintw(log_win->Inner,"has_colors() = TRUE\n");
    }
    else
    {
        wprintw(log_win->Inner,"has_colors() = FALSE\n");
    }
	wrefresh(log_win->Inner);
    
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if (iResult != 0) 
    {
        wprintw(log_win->Inner,"WSAStartup failed with error: %d\n", iResult);
        EveythingOK=false;
    }
    else
    {
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        // Resolve the server address and port
        iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
        if (iResult != 0) 
        {
                wprintw(log_win->Inner,"getaddrinfo failed with error: %d\n", iResult);
                EveythingOK=false;                    
        }
        else
        {
            // Create a SOCKET for connecting to server
            ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if (ListenSocket == INVALID_SOCKET) 
            {
                    wprintw(log_win->Inner,"socket failed with error: %ld\n", WSAGetLastError());
                    freeaddrinfo(result);
                    EveythingOK=false;                    
            }
            else
            {
                // Setup the TCP listening socket
                iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
                if (iResult == SOCKET_ERROR) 
                {
                        wprintw(log_win->Inner,"bind failed with error: %d\n", WSAGetLastError());
                        wrefresh(log_win->Inner);

                        freeaddrinfo(result);
                        EveythingOK=false;  
                }
                else
                {
                    freeaddrinfo(result);

                    iResult = listen(ListenSocket, SOMAXCONN);
                    if (iResult == SOCKET_ERROR) 
                    {
                            wprintw(log_win->Inner,"listen failed with error: %d\n", WSAGetLastError());
                            EveythingOK=false;                         }
                    else
                    {
						fd_set set;
						struct timeval timeout;
						int rv;


						timeout.tv_sec = 0;
						timeout.tv_usec = 20000;

						wprintw(log_win->Inner, "About to Wait:\n");
						wrefresh(log_win->Inner);

						for (;EveythingOK;)
                        {
                            ThreadData *Data = new ThreadData;

							FD_ZERO(&set); /* clear the set */
							FD_SET(ListenSocket, &set); /* add our file descriptor to the set */

							rv = select(0, &set, NULL, NULL, &timeout);
							if (rv == -1)
							{
								wprintw(log_win->Inner, "Select Error %d\n",rv); /* an error accured */
								EveythingOK = false;
							}
							else if (rv == 0)
							{
								/* a timeout occured */
								DoSpinner();
								DisplayPanel();
							}
							else
							{
								wprintw(log_win->Inner,"About to Call Accept:\n");
		                        wrefresh(log_win->Inner);

								// Accept a client socket
								Data->ClientSocket = accept(ListenSocket, NULL, NULL);

								if (Data->ClientSocket == INVALID_SOCKET)
								{
                                    wprintw(log_win->Inner,"accept failed with error: %d\n", WSAGetLastError());
                                    EveythingOK=false;  
								}
								else
								{
									hThread = CreateThread(
										NULL,                   // default security attributes
										0,                      // use default stack size  
										SlaveThread,            // thread function name
										Data,                   // argument to thread function 
										0,                      // use default creation flags 
										&ThreadId);             // returns the thread identifier
								}
								wprintw(log_win->Inner, "About to Wait:\n");
								wrefresh(log_win->Inner);
							}
                        }
                    }
                }
                // No longer need server socket
                closesocket(ListenSocket);
            }
        }

        WSACleanup();
    }

    if(!EveythingOK)
    {
        wprintw(log_win->Inner,"Press  any key to exit\n");
        wrefresh(log_win->Inner);
#ifdef __CYGWIN__
        mygetch();
#else
		getch();
#endif
    }

    destroy_win(log_win);
    
    endwin();			/* End curses mode		  */
        
    return EveythingOK?0:1;
}

