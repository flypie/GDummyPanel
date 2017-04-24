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

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>

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
char getch_(int echo) 
{
  char ch;
  initTermios(echo);
  ch = getchar();
  resetTermios();
  return ch;
}

/* Read 1 character without echo */
char getch(void) 
{
  return getch_(0);
}

DWORD WINAPI SlaveThread(LPVOID lpParam)
{
    ThreadData *Data;

    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Cast the parameter to the correct data type.
    // The pointer is known to be valid because 
    // it was checked for NULL before the thread was created.

    Data = (ThreadData *)lpParam;
    // Receive until the peer shuts down the connection

    printf("Server Thread Started:\n");

    CommandPacket	*CurrentPkt;

    do 
    {
        iResult = recv(Data->ClientSocket, recvbuf, recvbuflen, 0);
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
            printf("Bytes received: %d\n", iResult);
            CurrentPkt = (CommandPacket *)recvbuf;

            for (int i = 0; i < iResult; i += CurrentPkt->Data[PACKETLEN], CurrentPkt = (CommandPacket *)((char *)CurrentPkt+(CurrentPkt->Data[PACKETLEN])))
            {
                switch (CurrentPkt->Data[PACKETTYPE])
                {
                    case MACHINEDESC:
                        break;

                    case PINSTATE:
                        printf("Data: Pin %d State %d\n", CurrentPkt->Data[2], CurrentPkt->Data[3]);
                        break;

                    case READSTATE:
                        CommandPacket	Send;

                        Send.Data[2] = 6; //pins;
                        Send.Data[3] = 1;
                        Send.Data[4] = 0;
                        Send.Data[5] = 1;
                        Send.Data[6] = 0;
                        Send.Data[7] = 1;
                        Send.Data[8] = 2; //undefined pin

                        Send.Data[PACKETLEN] = sizeof(Send.Data[0])*(Send.Data[2] + 3);
                        Send.Data[PACKETTYPE] = READRESPONSE;

                        send(Data->ClientSocket, (char *)&Send, Send.Data[PACKETLEN], 0);

                        printf("Read State received.\n");
                        break;

                    default:
                        printf("Unkown PAcket Type\n");
                        break;
                }
            }

        }
        else if (iResult == 0)
        {
            printf("Connection closing...\n");
        }
        else 
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(Data->ClientSocket);
        }
    } while (iResult > 0);


    // cleanup
    closesocket(Data->ClientSocket);

    delete (ThreadData*)lpParam;

    return 0;
}

int main(int argc, char**argv) 
{
	WSADATA wsaData;
	int iResult;
        BOOL    EveythingOK=true;

	SOCKET ListenSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	HANDLE  hThread;
	DWORD   ThreadId;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) 
        {
		printf("WSAStartup failed with error: %d\n", iResult);
                EveythingOK=false;
//		return 1;
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
                    printf("getaddrinfo failed with error: %d\n", iResult);
                    EveythingOK=false;                    
            }
            else
            {
                // Create a SOCKET for connecting to server
                ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
                if (ListenSocket == INVALID_SOCKET) 
                {
                        printf("socket failed with error: %ld\n", WSAGetLastError());
                        freeaddrinfo(result);
                        EveythingOK=false;                    
                }
                else
                {
                    // Setup the TCP listening socket
                    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
                    if (iResult == SOCKET_ERROR) 
                    {
                            printf("bind failed with error: %d\n", WSAGetLastError());
                            freeaddrinfo(result);
                            EveythingOK=false;  
                    }
                    else
                    {
                        freeaddrinfo(result);

                        iResult = listen(ListenSocket, SOMAXCONN);
                        if (iResult == SOCKET_ERROR) 
                        {
                                printf("listen failed with error: %d\n", WSAGetLastError());
                                EveythingOK=false;                         }
                        else
                        {
                            for (;EveythingOK;)
                            {
                                ThreadData *Data = new ThreadData;

                                printf("About to Call Accept:\n");

                                // Accept a client socket
                                Data->ClientSocket = accept(ListenSocket, NULL, NULL);

                                if (Data->ClientSocket == INVALID_SOCKET)
                                {
                                        printf("accept failed with error: %d\n", WSAGetLastError());
                                        EveythingOK=false;  
                                }
                                else
                                {
                                    hThread = CreateThread(
                                        NULL,			// default security attributes
                                        0,                      // use default stack size  
                                        SlaveThread,            // thread function name
                                        Data,			// argument to thread function 
                                        0,                      // use default creation flags 
                                        &ThreadId);		// returns the thread identifier
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
            printf("Press  any key to exit\n");
            getch();
        }
	return EveythingOK?0:1;
}
