
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
#include "Button.h"
#include "ComplexWindow.h"


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

ComplexWindow   *log_win=(ComplexWindow   *)-1;
ComplexWindow   *panel_win=(ComplexWindow   *)-1;

int gstartx, gstarty, gwidth, gheight;

short color_table[] =
{
    COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN,
    COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE
};


void    checkandresize()
{
    int width, height;

    getmaxyx(stdscr,height,width);
    
    if(height!=gheight || width!=gwidth)
    {    
        gheight=height;
        gwidth=width;

        wclear(stdscr);
      
		log_win->complexresize(gheight/2,gwidth);
  
        if(log_win!=(ComplexWindow *)-1)
        {
			log_win->complexresize(gheight/2,gwidth);

			log_win->mvwin(gheight/2);
        }
        log_win->printw("Resize\n");
        log_win->refresh();
    }
}


void CreateButtons()
{
	for (int i = 0; i < NUMPIPINS; i++)
	{
		Button *But = new Button(2 + 6 * (i % 16), 1 + (i / 16) * 3,3,5,i);
		But->SetSelected(GPIOStatus[i]);
		panel_win->add_button(But);
		But->draw();
	}

	panel_win->refresh();
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
    
    checkandresize();    
       
	log_win->printw("Slave Thread Started:\n");
	log_win->refresh();
    
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
            log_win->printw("Bytes received: %d\n", iResult);
            log_win->refresh();

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

								But = panel_win->find_button_data(j);

								if (!But->GetOut())
								{
									But->SetOut(true);
								}

								if(But)
								{
									But->SetSelected(CurrentPkt->Data[6]!=0);
									GPIOStatus[But->GetiData()] = CurrentPkt->Data[6];
									But->draw();
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

                        log_win->printw("Read State received.\n");
                        break;

                    default:
                        log_win->printw("Unknown Packet Type\n");
                        break;
                }
            }
			panel_win->Display();
			log_win->refresh();
		}
        else if (iResult == 0)
        {
            log_win->printw("Connection closing...\n");
        }
        else 
        {
            log_win->printw("recv failed with error: %d\n", WSAGetLastError());
            closesocket(TData->ClientSocket);
        }
        log_win->refresh();
    } while (iResult > 0);


    // cleanup
    closesocket(TData->ClientSocket);

    delete (ThreadData*)lpParam;
    
    return 0;
}



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

    log_win = new ComplexWindow(gheight/2, gwidth, starty, startx);
	panel_win = new ComplexWindow(gheight / 2, gwidth, 0, 0);

	CreateButtons();

	if (has_colors())
    {
		log_win->printw("has_colors() = TRUE\n");
    }
    else
    {
		log_win->printw("has_colors() = FALSE\n");
    }
	log_win->refresh();
    
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    
    if (iResult != 0) 
    {
		log_win->printw("WSAStartup failed with error: %d\n", iResult);
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
				log_win->printw("getaddrinfo failed with error: %d\n", iResult);
                EveythingOK=false;                    
        }
        else
        {
            // Create a SOCKET for connecting to server
            ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if (ListenSocket == INVALID_SOCKET) 
            {
					log_win->printw("socket failed with error: %ld\n", WSAGetLastError());
                    freeaddrinfo(result);
                    EveythingOK=false;                    
            }
            else
            {
                // Setup the TCP listening socket
                iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
                if (iResult == SOCKET_ERROR) 
                {
						log_win->printw("bind failed with error: %d\n", WSAGetLastError());
						log_win->refresh();

                        freeaddrinfo(result);
                        EveythingOK=false;  
                }
                else
                {
                    freeaddrinfo(result);

                    iResult = listen(ListenSocket, SOMAXCONN);
                    if (iResult == SOCKET_ERROR) 
                    {
							log_win->printw("listen failed with error: %d\n", WSAGetLastError());
                            EveythingOK=false;                         }
                    else
                    {
						fd_set set;
						struct timeval timeout;
						int rv;


						timeout.tv_sec = 0;
						timeout.tv_usec = 20000;

						log_win->printw("About to Wait:\n");
						log_win->refresh();

						for (;EveythingOK;)
                        {
                            ThreadData *Data = new ThreadData;

							FD_ZERO(&set); /* clear the set */
							FD_SET(ListenSocket, &set); /* add our file descriptor to the set */

							rv = select(0, &set, NULL, NULL, &timeout);
							if (rv == -1)
							{
								log_win->printw("Select Error %d\n",rv); /* an error accured */
								EveythingOK = false;
							}
							else if (rv == 0)
							{
								/* a timeout occured */
								log_win->DoSpinner();
								panel_win->Display();
							}
							else
							{
								log_win->printw("About to Call Accept:\n");
								log_win->refresh();

								// Accept a client socket
								Data->ClientSocket = accept(ListenSocket, NULL, NULL);

								if (Data->ClientSocket == INVALID_SOCKET)
								{
									log_win->printw("accept failed with error: %d\n", WSAGetLastError());
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
								log_win->printw("About to Wait:\n");
								log_win->refresh();
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
		log_win->printw("Press  any key to exit\n");
		log_win->refresh();
#ifdef __CYGWIN__
        mygetch();
#else
		getch();
#endif
    }

    delete log_win;
    
    endwin();			/* End curses mode		  */
        
    return EveythingOK?0:1;
}

