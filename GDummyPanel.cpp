
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

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#endif

#ifdef _POSIX_VERSION
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef _POSIX_VERSION
#include <pthread.h>
#include <termios.h>
#else
#include <process.h>
#include <conio.h>
#endif

#include "curses.h"

#include "Fudge.h"

#include "Button.h"
#include "ComplexWindow.h"

bool    GlobalExit = false;

#define BUF_SIZE 255
#define	DEFAULT_BUFLEN 1024

#ifdef _POSIX_VERSION
#define DEFAULT_PORT 45567
#else
#define DEFAULT_PORT "45567"
#endif

typedef struct
{
	SOCKET ClientSocket;
} ThreadData;

typedef enum
{
	MACHINEDESC = 0,
	PINSTATE = 1,
	READSTATE = 2,
	READRESPONSE = 3
} PacketType;

#define MAXPACKET   255

#define PACKETLEN   0  //Includes Packet Length
#define PACKETTYPE  1

typedef struct
{
	unsigned short int Data[MAXPACKET];
} CommandPacket;


#define NUMPIPINS	54
#define LOGWINHEIGHT    10
bool	GPIOStatus[NUMPIPINS] = { 0 };

ComplexWindow   *log_win = (ComplexWindow   *)-1;
static ComplexWindow   *panel_win = (ComplexWindow   *)-1;

int gstartx, gstarty, gwidth, gheight;

short color_table[] =
{
	COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_CYAN,
	COLOR_RED, COLOR_MAGENTA, COLOR_YELLOW, COLOR_WHITE
};


#ifdef _POSIX_VERSION
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

#define MYBUTTONWIDTH 5
#define MYBUTTONHEIGHT 3
#define BUTTONSPERROW  10
#define BUTTONWINDOWHEIGHT (MYBUTTONHEIGHT + (MYBUTTONHEIGHT*NUMPIPINS / BUTTONSPERROW)) + 1

void    checkandresize()
{
	int width, height;

	getmaxyx(stdscr, height, width);

	if (height != gheight || width != gwidth)
	{
		gheight = height;
		gwidth = width;

		wclear(stdscr);

		panel_win->complexresize(BUTTONWINDOWHEIGHT, gwidth);
		log_win->complexresize(gheight-BUTTONWINDOWHEIGHT, gwidth);
		log_win->mvwin(BUTTONWINDOWHEIGHT);
		
		log_win->printw("Resize\n");
		log_win->refresh();
	}
}



void CreateButtons()
{
	for (int i = 0; i < NUMPIPINS; i++)
	{
		Button *But = new Button(1 + MYBUTTONWIDTH * (i % BUTTONSPERROW), 1 + (i / BUTTONSPERROW) * MYBUTTONHEIGHT, MYBUTTONHEIGHT, MYBUTTONWIDTH, i);
		But->SetSelected(GPIOStatus[i]);
		panel_win->add_button(But);
		But->draw();
	}

	panel_win->refresh();
}


DWORD SlaveThread(void *lpParam)
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

			for (int i = 0; i < iResult; i += CurrentPkt->Data[PACKETLEN], CurrentPkt = (CommandPacket *)((char *)CurrentPkt + (CurrentPkt->Data[PACKETLEN])))
			{
				switch (CurrentPkt->Data[PACKETTYPE])
				{
				case MACHINEDESC:
					break;

				case PINSTATE:
					Data = (uint64_t)CurrentPkt->Data[2] | (uint64_t)CurrentPkt->Data[3] << 16 | (uint64_t)CurrentPkt->Data[4] << 32 | (uint64_t)CurrentPkt->Data[5] << 48;

					for (int j = 0; j < NUMPIPINS && Data; j++)
					{
						state = Data & 0x1;

						if (state)
						{
							Button *But;

							But = panel_win->find_button_data(j);

							if (!But->GetOut())
							{
								But->SetOut(true);
							}

							if (But)
							{
								But->SetSelected(CurrentPkt->Data[6] != 0);
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

					for (int j = 0; j < NUMPIPINS; j++)
					{
						Data = Data << 1;
						if (GPIOStatus[NUMPIPINS - 1 - j])
						{
							Data |= 0x01;
						}
					}

					Send.Data[2] = (unsigned short)Data;
					Send.Data[3] = (unsigned short)(Data >> 16);
					Send.Data[4] = (unsigned short)(Data >> 32);
					Send.Data[5] = (unsigned short)(Data >> 48);

					Send.Data[PACKETLEN] = sizeof(Send.Data[0]) * 6;
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
	} while (iResult > 0 && !GlobalExit);

	// cleanup
	closesocket(TData->ClientSocket);

	delete (ThreadData*)lpParam;

	log_win->printw("Slave Thread Exited:\n");
	log_win->refresh();

	return 0;
}

int main(int argc, char**argv)
{
	int i;

#ifndef _POSIX_VERSION        
	WSADATA wsaData;
#endif
	int     iResult;
	bool    EveythingOK = true;

	SOCKET  ListenSocket = INVALID_SOCKET;

#ifdef _POSIX_VERSION
	struct  sockaddr_in *result = NULL;
	struct  sockaddr_in hints;
#else
	struct  addrinfo *result = NULL;
	struct  addrinfo hints;
#endif
	HANDLE  hThread;

#ifdef _POSIX_VERSION
	pthread_t *ThreadId;
#else        
	DWORD   ThreadId;
#endif

	WINDOW	*Master;

#ifdef XCURSES
	Master = Xinitscr(argc, argv);
#else
	Master = initscr();
#endif

	if (Master)
	{
		mousemask(ALL_MOUSE_EVENTS, 0);    /* The old events mask                */

		nodelay(stdscr, TRUE);
		noecho();

		curs_set(0);

		if (has_colors())
		{
			start_color();
		}

		for (i = 0; i < 8; i++)
		{
			init_pair(i, color_table[i], COLOR_BLACK);
		}



		resize_term(BUTTONWINDOWHEIGHT + LOGWINHEIGHT, BUTTONSPERROW * MYBUTTONWIDTH + 2);

		getmaxyx(stdscr, gheight, gwidth);		/* get the number of rows and columns */

		panel_win = new ComplexWindow(BUTTONWINDOWHEIGHT, gwidth, 0, 0);
		log_win = new ComplexWindow(LOGWINHEIGHT, gwidth, BUTTONWINDOWHEIGHT, 0);

		if (log_win&&panel_win)
		{
			CreateButtons();

			if (has_colors())
			{
				log_win->printw("has_colours() = TRUE\n");
			}
			else
			{
				log_win->printw("has_colours() = FALSE\n");
			}
			log_win->printw("Presss 'X' to exit\n");

			log_win->refresh();

#ifdef _POSIX_VERSION 
			iResult = 0;
#else
			iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

			if (iResult == 0)
			{
#ifdef _POSIX_VERSION                      
				hints.sin_family = AF_INET;
				hints.sin_port = htons(DEFAULT_PORT);
				hints.sin_addr.s_addr = INADDR_ANY;
				bzero(&(hints.sin_zero), 8);
				iResult = 0;
#else
				ZeroMemory(&hints, sizeof(hints));

				hints.ai_family = AF_INET;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				hints.ai_flags = AI_PASSIVE;
				// Resolve the server address and port
				iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
#endif
				if (iResult == 0)
				{
#ifdef _POSIX_VERSION                            
					ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
#else  
					ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
#endif
					if (ListenSocket != INVALID_SOCKET)
					{
						// Setup the TCP listening socket
#ifdef _POSIX_VERSION                                                               
						iResult = bind(ListenSocket, (struct sockaddr *)&hints, sizeof(struct sockaddr));
#else
						iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
#endif
						if (iResult != SOCKET_ERROR)
						{
#ifdef _POSIX_VERSION                                                               
#else
							freeaddrinfo(result);
#endif
							iResult = listen(ListenSocket, SOMAXCONN);
							if (iResult != SOCKET_ERROR)
							{
								fd_set set;
								struct timeval timeout;
								int rv;

								timeout.tv_sec = 0;
								timeout.tv_usec = 20000;

								log_win->printw("About to Wait:\n");
								log_win->refresh();

								for (; EveythingOK && !GlobalExit;)
								{
									ThreadData *Data = new ThreadData;

									FD_ZERO(&set); /* clear the set */
									FD_SET(ListenSocket, &set); /* add our file descriptor to the set */

									rv = select(0, &set, NULL, NULL, (timeval *)&timeout);
									if (rv == 0)
									{
										/* a timeout occured */
										log_win->DoSpinner();
										panel_win->Display();
									}
									else if (rv != -1)
									{
										log_win->printw("About to Call Accept:\n");
										log_win->refresh();

										// Accept a client socket
										Data->ClientSocket = accept(ListenSocket, NULL, NULL);

										if (Data->ClientSocket != INVALID_SOCKET)
										{
#ifdef _POSIX_VERSION
											hThread = pthread_create(ThreadId, NULL, (void* (*)(void*))SlaveThread, Data);
#else
											hThread = CreateThread(
												NULL,                   // default security attributes
												0,                      // use default stack size  
												SlaveThread,            // thread function name
												Data,                   // argument to thread function 
												0,                      // use default creation flags 
												&ThreadId);             // returns the thread identifier
#endif
											log_win->printw("About to Wait:\n");
											log_win->refresh();
										}
										else
										{
											log_win->printw("accept failed with error: %d\n", WSAGetLastError());
											EveythingOK = false;
										}
									}
									else
									{
										if(WSAGetLastError()!=EINTR)
										{
	                                        log_win->printw("Select Error %d\n", WSAGetLastError()); /* an error accured */
											EveythingOK = false;
										}
									}
								}
							}
							else
							{
								log_win->printw("listen failed with error: %d\n", WSAGetLastError());
								EveythingOK = false;
							}
						}
						else
						{
							log_win->printw("bind failed with error: %d\n", WSAGetLastError());
							EveythingOK = false;
						}
						// No longer need server socket
						closesocket(ListenSocket);
					}
					else
					{
						log_win->printw("socket failed with error: %ld\n", WSAGetLastError());
						EveythingOK = false;
					}                                    
				}
				else
				{
					log_win->printw("getaddrinfo failed with error: %d\n", iResult);
					EveythingOK = false;
				}
#ifdef __WINDOWS__                                
				WSACleanup();
#endif			
			}
			else
			{
				log_win->printw("WSAStartup failed with error: %d\n", iResult);
				EveythingOK = false;
			}

			if (!EveythingOK)
			{
				log_win->printw("Press any key to exit\n");
				log_win->refresh();
#ifdef _POSIX_VERSION
				mygetch();
#else
				_getch();
#endif
			}

			delete log_win;
			delete panel_win;
		}
		else
		{
			EveythingOK = false;
			printf("Complex WIndow Creation failed\n");
			printf("Press any key to exit\n");
#ifdef _POSIX_VERSION
			mygetch();
#else
			_getch();
#endif

		}
		endwin();			/* End curses mode		  */
	}
	else
	{
		EveythingOK = false;
		printf("initscr failed\n");
		printf("Press any key to exit\n");
#ifdef _POSIX_VERSION
		mygetch();
#else
		_getch();
#endif
	}
	return EveythingOK ? 0 : 1;
}

