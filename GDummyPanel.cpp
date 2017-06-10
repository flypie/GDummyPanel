
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

#include "Fudge.h"

#ifdef _POSIX_VERSION
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <algorithm>    // std::max
#else
#endif

using namespace std;

#include "GPanelObject.h"

#include "Button.h"
#include "NumBox.h"
#include "ComplexWindow.h"
#include "GPIO.h"
#include "GPanel.h"

#include <typeinfo>

#define BUF_SIZE 255
#define	DEFAULT_BUFLEN 1024

#ifdef _POSIX_VERSION
#define DEFAULT_PORT 45567
#else
#define DEFAULT_PORT "45567"
#endif

#define NUMPIPINS	54
#define LOGWINHEIGHT    10


#define MAXBUTTONS	64
#define MYBUTTONWIDTH   5
#define MYBUTTONHEIGHT  3
#define BUTTONSPERROW   10

#define MAXNUMBOXES	5
#define BOXWIDTH    5
#define BOXHEIGHT   3
#define BOXSPERROW  5

#define BUTTONWINDOWHEIGHT MYBUTTONHEIGHT + (MYBUTTONHEIGHT*MAXBUTTONS / BUTTONSPERROW) 
#define BOXWINDOWHEIGHT (BOXHEIGHT*MAXNUMBOXES / BOXSPERROW) 

#define PANELWINDOWHEIGHT BUTTONWINDOWHEIGHT + BOXWINDOWHEIGHT + 2

#define MAXPACKET   255

#define PACKETLEN   0  //Includes Packet Length
#define PACKETTYPE  1

int ButtonHandles[NUMPIPINS];

typedef struct ThreadDataT
{
    SOCKET ClientSocket;
} ThreadData;

typedef enum
{
    PROTOCOLDESCFROMQEMU = 0,
    PROTOCOLDESCFROMPANEL = 1,
    PINSTOPANEL = 2,
    READREQ = 3,
    PINCOUNT = 4,
    ENABLEMAP = 5,
    INPUTMAP = 6,
    OUTPUTMAP = 7,
    PINSTOQEMU = 8,
    MULTIBITFROMQEMU = 9,
    MULTIBITTOOQEMU = 10
} PacketType;

#define MYMAXPROTOCOL   0
#define MYMINPROTOCOL   0

typedef struct
{
    unsigned short int Data[MAXPACKET];
} CommandPacket;

bool    GlobalExit = false;

#define MAXMULTIBYTES   8
uint32_t    MultiByte[MAXMULTIBYTES];

ComplexWindow   *log_win = (ComplexWindow   *)-1;
ComplexWindow   *panel_win = (ComplexWindow   *)-1;

static int gwidth, gheight;

short color_table[] =
{
    COLOR_RED, COLOR_GREEN, COLOR_BLUE,
    COLOR_CYAN, COLOR_MAGENTA, COLOR_YELLOW
};

int ProtocolInUse = -1;

uint64_t EnabledMask = 0xFFFFFFFF;
uint64_t InputMask;
uint64_t OutputMask;

WINDOW	*Master;

#ifdef _POSIX_VERSION
pthread_mutex_t curseslock; //NCurses not thread  safe/
pthread_mutex_t threadlock; //NCurses not thread  safe/
#else
HANDLE  curseslock;
HANDLE  threadlock;
#endif

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

void CreateButtons(int Num)
{
    for (int i = 0; i < Num; i++) {
        int BIndex = Button::GetNumObjects();
        Button *But = new Button(panel_win, MYBUTTONWIDTH * (Button::GetNumObjects() % BUTTONSPERROW), (Button::GetNumObjects() / BUTTONSPERROW) * MYBUTTONHEIGHT, MYBUTTONWIDTH, MYBUTTONHEIGHT, Button::GetNumObjects());
        ButtonHandles[BIndex] = But->GetHandle();
        But->SetIntValue(BIndex);
        But->SetSelected(GPIO::GPIOs->GetStatus(BIndex));
        panel_win->add_object(But);
    }
}

void CreateNumBoxes(int Num)
{
    int StartY = BUTTONWINDOWHEIGHT;

    for (int i = 0; i < Num; i++) {
        NumBox *Box = new NumBox(panel_win, MYBUTTONWIDTH * 2 * (NumBox::GetNumObjects() % BUTTONSPERROW), StartY + (NumBox::GetNumObjects() / BUTTONSPERROW) * MYBUTTONHEIGHT, MYBUTTONWIDTH * 2, MYBUTTONHEIGHT, 0);
        panel_win->add_object(Box);
    }
}

void SendPinStates(ThreadData *TData)
{
    uint64_t Data;
    uint64_t state;

    CommandPacket	Send;
    Data = 0;

    for (int j = 0; j < NUMPIPINS; j++) {
        Data = Data << 1;
        if (GPIO::GPIOs->GetStatus(NUMPIPINS - 1 - j)) {
            Data |= 0x01;
        }
    }

    GPIO::GPIOs->Sent(); //This set a flag to say we have sent latest changes

    Send.Data[2] = (unsigned short)Data;
    Send.Data[3] = (unsigned short)(Data >> 16);
    Send.Data[4] = (unsigned short)(Data >> 32);
    Send.Data[5] = (unsigned short)(Data >> 48);

    Send.Data[PACKETLEN] = sizeof(Send.Data[0]) * 6;
    Send.Data[PACKETTYPE] = PINSTOQEMU;

    send(TData->ClientSocket, (char *)&Send, Send.Data[PACKETLEN], 0);

    log_win->printw("Pin States Sent.");
}

void SendMultiBit(ThreadData *TData)
{
    uint32_t Data;
    uint64_t state;

    CommandPacket	Send;

    Data = 0xFF;

    Send.Data[PACKETLEN] = sizeof(Send.Data[0]) * 6;
    Send.Data[PACKETTYPE] = MULTIBITTOOQEMU;
    Send.Data[2] = 0; /* Multibit ID Number */
    Send.Data[3] = 8;
    Send.Data[4] = (unsigned short)Data;


    send(TData->ClientSocket, (char *)&Send, Send.Data[PACKETLEN], 0);

    log_win->printw("Multi Bit Sent.");
}

void ReceiveMultiBit(ThreadData *TData, CommandPacket	*CurrentPkt)
{
    int IDNum;
    int Bits;

    IDNum = CurrentPkt->Data[2];

    if (IDNum < MAXMULTIBYTES) {
        Bits = CurrentPkt->Data[3];
        MultiByte[IDNum] = CurrentPkt->Data[4];
    }
    else {
        log_win->printw("Multi Bit ReceiveID Out of range.");
    }
}

void SendProtocol(ThreadData *TData)
{
    CommandPacket	Send;

    Send.Data[PACKETLEN] = sizeof(Send.Data[0]) * 3;
    Send.Data[PACKETTYPE] = PROTOCOLDESCFROMPANEL;

    Send.Data[2] = (unsigned short)ProtocolInUse;

    send(TData->ClientSocket, (char *)&Send, Send.Data[PACKETLEN], 0);

    log_win->printw("SendProtocol Sent.");
}

void SetProtocol(ThreadData *TData, CommandPacket	*CurrentPkt)
{
    int QMinP, QMaxP;
    int CMinP, CMaxP;

    QMinP = CurrentPkt->Data[2];
    QMaxP = CurrentPkt->Data[3];

    CMaxP = min(QMaxP, MYMAXPROTOCOL);
    CMinP = max(QMinP, MYMINPROTOCOL);

    if (CMaxP >= CMinP) {
        ProtocolInUse = CMaxP;
        log_win->printw("Protocol Agreed %d", ProtocolInUse);
    }
    else {
        ProtocolInUse = -1;
        log_win->printw("No Common Protocol");
    }
    SendProtocol(TData);
}

void SetPinCount(ThreadData *TData, CommandPacket	*CurrentPkt)
{
    int Count;

    Count = CurrentPkt->Data[2];

    if (Count != Button::GetNumObjects()) {
        if (Count < Button::GetNumObjects()) {
            panel_win->DeleteObjects(typeid(Button).hash_code(), Button::GetNumObjects() - Count);
        }
        else {
            CreateButtons(Count - Button::GetNumObjects());
        }

//        panel_win->DeleteObjects(typeid(NumBox).hash_code());
//        CreateNumBoxes(2);
        panel_win->Draw();

        update_panels();
        doupdate();
    }
}

void SetPinStates(ThreadData *TData, CommandPacket	*CurrentPkt, bool FromPanel)
{
    uint64_t Data;
    uint64_t state;

    CommandPacket	Send;

    Data = (uint64_t)CurrentPkt->Data[2] | (uint64_t)CurrentPkt->Data[3] << 16 | (uint64_t)CurrentPkt->Data[4] << 32 | (uint64_t)CurrentPkt->Data[5] << 48;

    for (int j = 0; j < NUMPIPINS && Data; j++) {
        state = Data & 0x1;

        if (state) {
            Button *But;

            But = dynamic_cast<Button*> (panel_win->find_object_handle(ButtonHandles[j]));

            if (But) {
                if (!But->GetOut()) {
                    But->SetOutput(true);
                }

                But->SetSelected(CurrentPkt->Data[6] != 0);
                GPIO::GPIOs->SetStatus(But->GetIntValue(), CurrentPkt->Data[6], FromPanel);
                But->Draw();
            }
        }
        Data = Data >> 1;
    }
}

void SetEnableMap(ThreadData *TData, CommandPacket	*CurrentPkt)
{
    uint64_t Data;
    uint64_t state;

    CommandPacket	Send;

    EnabledMask = Data = (uint64_t)CurrentPkt->Data[2] | (uint64_t)CurrentPkt->Data[3] << 16 | (uint64_t)CurrentPkt->Data[4] << 32 | (uint64_t)CurrentPkt->Data[5] << 48;

    for (int j = 0; j < NUMPIPINS && Data; j++) {
        Button *But;

        But = dynamic_cast<Button*> (panel_win->find_object_handle(ButtonHandles[j]));

        if (But) {
            state = Data & 0x1;

            if (state) {
                But->SetEnabled(true);
            }
            else {
                But->SetEnabled(false);
            }
            But->Draw();
        }
        Data = Data >> 1;
    }
}

void SetInputMap(ThreadData *TData, CommandPacket	*CurrentPkt)
{
    uint64_t Data;
    uint64_t state;

    CommandPacket	Send;

    InputMask = Data = (uint64_t)CurrentPkt->Data[2] | (uint64_t)CurrentPkt->Data[3] << 16 | (uint64_t)CurrentPkt->Data[4] << 32 | (uint64_t)CurrentPkt->Data[5] << 48;

    for (int j = 0; j < NUMPIPINS && Data; j++) {
        Button *But;

        But = dynamic_cast<Button*> (panel_win->find_object_handle(ButtonHandles[j]));

        if (But) {
            state = Data & 0x1;

            if (state) {
                But->SetInput(true);
            }
            But->Draw();
        }
        Data = Data >> 1;
    }
}

void SetOutputMap(ThreadData *TData, CommandPacket	*CurrentPkt)
{
    uint64_t Data;
    uint64_t state;

    CommandPacket	Send;

    OutputMask = Data = (uint64_t)CurrentPkt->Data[2] | (uint64_t)CurrentPkt->Data[3] << 16 | (uint64_t)CurrentPkt->Data[4] << 32 | (uint64_t)CurrentPkt->Data[5] << 48;

    for (int j = 0; j < NUMPIPINS && Data; j++) {
        Button *But;

        But = dynamic_cast<Button*> (panel_win->find_object_handle(ButtonHandles[j]));

        if (But) {
            state = Data & 0x1;

            if (state) {
                But->SetOutput(true);
            }
            But->Draw();
        }
        Data = Data >> 1;
    }
}

#ifdef _POSIX_VERSION 
static void *SlaveThread(void *lpParam)
#else
DWORD WINAPI SlaveThread(LPVOID lpParam)
#endif
{
    ThreadData *TData;

    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    CommandPacket	*CurrentPkt;

    fd_set set;
    struct timeval timeout;

    LOCKMUTEX(threadlock);

    TData = (ThreadData *)lpParam;

    log_win->printw("Slave Thread Started:1 Socket %d", TData->ClientSocket);
    log_win->refresh();


    do {
        timeout.tv_sec = 0;
        timeout.tv_usec = 20000;

        FD_ZERO(&set); /* clear the set */
        FD_SET(TData->ClientSocket, &set); /* add our file descriptor to the set */

        iResult = select(FD_SETSIZE, &set, NULL, NULL, (timeval *)&timeout);
        if (iResult == 0) {
            /* a timeout occured */
            if (GPIO::GPIOs->NeedSending()) {
                SendPinStates(TData);
            }
        }
        else if (iResult != -1) {
            iResult = recv(TData->ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                if (iResult < DEFAULT_BUFLEN) {
                    recvbuf[iResult] = 0;
                }
                else {
                    recvbuf[DEFAULT_BUFLEN - 1] = 0;
                }

                CurrentPkt = (CommandPacket *)recvbuf;

                for (int i = 0; i < iResult; i += CurrentPkt->Data[PACKETLEN], CurrentPkt = (CommandPacket *)((char *)CurrentPkt + (CurrentPkt->Data[PACKETLEN]))) {
                    switch (CurrentPkt->Data[PACKETTYPE]) {
                    case PROTOCOLDESCFROMQEMU:
                        SetProtocol(TData, CurrentPkt);
                        break;

                    case PINSTOPANEL:
                        SetPinStates(TData, CurrentPkt, false);
                        break;

                    case READREQ:
                        log_win->printw("READREQ");
                        SendPinStates(TData);
                        break;

                    case PINCOUNT:
                        log_win->printw("PINCOUNT");
                        SetPinCount(TData, CurrentPkt);
                        break;

                    case ENABLEMAP:
                        log_win->printw("ENABLEMAP");
                        SetEnableMap(TData, CurrentPkt);
                        break;

                    case INPUTMAP:
                        log_win->printw("INPUTMAP");
                        SetInputMap(TData, CurrentPkt);
                        break;

                    case OUTPUTMAP:
                        log_win->printw("OUTPUTMAP");
                        SetOutputMap(TData, CurrentPkt);
                        break;

                    case MULTIBITFROMQEMU:
                        log_win->printw("MULTIBITFROMQEMU");
                        ReceiveMultiBit(TData, CurrentPkt);
                        break;

                    default:
                        log_win->printw("Unknown Packet Type");
                        break;
                    }
                }
                log_win->refresh();
            }
            else if (iResult == 0) {
                log_win->printw("Connection closing...");
            }
            else {
                log_win->printw("recv failed with error: %d", WSAGetLastError());
                closesocket(TData->ClientSocket);
            }
            log_win->refresh();
        }
    }
    while ((iResult >= 0 || (iResult == -1 && WSAGetLastError() == EINTR)) && !GlobalExit);

    // cleanup
    closesocket(TData->ClientSocket);

    delete (ThreadData*)lpParam;

    log_win->printw("Slave Thread Exited:");
    log_win->refresh();

    UNLOCKMUTEX(threadlock);

#ifdef _POSIX_VERSION
#else
    return 0;
#endif
}

void HandleEvents()
{
    EVENTTYPE c;
    MEVENT event;
    GPanel *Cur;

    LOCKMUTEX(curseslock);

    c = getch();

    if (c != -1) {
        if (c == KEY_MOUSE) {
#ifdef _POSIX_VERSION
            if (getmouse(&event) == OK) {
#else
            if (nc_getmouse(&event) == OK) {
#endif  
            }
        }

        bool Handled = false;
        GPanel *Cur = GPanel::List;

        while (Cur && !Handled) {
            if (Cur->GetStop()) {
                GPanel *Tmp;
                Tmp = Cur->GetNext();
                delete Cur;
                Cur = Tmp;
            }
            else {
                Handled = Cur->HandleEvent(c, event);
                Cur = Cur->GetNext();
            }
        };
    }
    else {
        Cur = GPanel::List;

        while (Cur) {
            if (Cur->GetStop()) {
                GPanel *Tmp;
                Tmp = Cur->GetNext();
                delete Cur;
                Cur = Tmp;
            }
            else {
                Cur = Cur->GetNext();
            }
        }
    }

    UNLOCKMUTEX(curseslock);
}

int main(int argc, char**argv)
{
    int i;
#ifdef _POSIX_VERSION
    pthread_mutexattr_t Attr;

    pthread_mutexattr_init(&Attr);
    pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);

    pthread_mutex_init(&curseslock, &Attr);
#else
    curseslock = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
    if (curseslock == NULL) {
        printf("CreateMutex error: %d", GetLastError());
        return 1;
    }
#endif

#ifdef _POSIX_VERSION
    pthread_mutex_init(&threadlock, &Attr);
#else
    threadlock = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);             // unnamed mutex
    if (threadlock == NULL) {
        printf("CreateMutex error: %d", GetLastError());
        return 1;
    }
#endif

#ifdef _POSIX_VERSION
#else
    WSADATA wsaData;
#endif
    int     iResult;
    bool    EveythingOK = true;

    SOCKET  ListenSocket = INVALID_SOCKET;

#ifdef _POSIX_VERSION
    struct  sockaddr_in *result = NULL;
    struct  sockaddr_in hints;
    pthread_t ThreadId;
#else
    struct  addrinfo *result = NULL;
    struct  addrinfo hints;
    DWORD   ThreadId;
#endif
    HANDLE  hThread;

    ThreadData *Data;

    fd_set set;
    struct timeval timeout;

#ifdef XCURSES
    Master = Xinitscr(argc, argv);
#else
    Master = initscr();
#endif

    if (Master) {
        nodelay(stdscr, TRUE);
        keypad(stdscr, TRUE);
        noecho();

        curs_set(0);

        if (has_colors()) {
            start_color();
        }

        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(1, COLOR_BLACK, COLOR_WHITE);

        for (i = 0; i < 6; i++) {
            init_pair(i + 2, color_table[i], COLOR_BLACK);
            init_pair(i + 8, COLOR_BLACK, color_table[i]);
            init_pair(i + 14, color_table[i], COLOR_WHITE);
            init_pair(i + 20, COLOR_WHITE, color_table[i]);
        }

        resize_term(PANELWINDOWHEIGHT + LOGWINHEIGHT, BUTTONSPERROW * MYBUTTONWIDTH + 2);

        getmaxyx(stdscr, gheight, gwidth);		/* get the number of rows and columns */

        panel_win = new ComplexWindow(PANELWINDOWHEIGHT, gwidth, 0, 0);

        GPanel  *Panel1 = new GPanel;
        Panel1->Add(panel_win);

        log_win = new ComplexWindow(LOGWINHEIGHT, gwidth, PANELWINDOWHEIGHT, 0);

        GPanel  *Panel2 = new GPanel;
        Panel2->Add(log_win);

        Panel1->SetTop();

        update_panels();

        if (log_win&&panel_win) {

            GPIO::GPIOs = new GPIO(MAXBUTTONS);

            CreateButtons(10);
            CreateNumBoxes(2);
            panel_win->Draw();

            log_win->printw("Presss 'X' to exit");

            update_panels();
            doupdate();

            mousemask(ALL_MOUSE_EVENTS, 0);    /* The old events mask                */

#ifdef _POSIX_VERSION 
            iResult = 0;
#else
            iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
            if (iResult == 0) {
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
                if (iResult == 0) {
#ifdef _POSIX_VERSION                            
                    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
#else  
                    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
#endif
                    if (ListenSocket != INVALID_SOCKET) {
                        // Setup the TCP listening socket
#ifdef _POSIX_VERSION                                                               
                        iResult = bind(ListenSocket, (struct sockaddr *)&hints, sizeof(struct sockaddr));
#else
                        iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
#endif
                        if (iResult != SOCKET_ERROR) {
#ifdef _POSIX_VERSION                                                               
#else
                            freeaddrinfo(result);
#endif
                            iResult = listen(ListenSocket, SOMAXCONN);
                            if (iResult != SOCKET_ERROR) {
                                timeout.tv_sec = 0;
                                timeout.tv_usec = 20000;

                                log_win->printw("About to Wait:");
                                log_win->refresh();

                                for (; EveythingOK && !GlobalExit;) {
                                    Data = new ThreadData;

                                    FD_ZERO(&set); /* clear the set */
                                    FD_SET(ListenSocket, &set); /* add our file descriptor to the set */

                                    iResult = select(FD_SETSIZE, &set, NULL, &set, (timeval *)&timeout);
                                    if (iResult == 0) {
                                        /* a timeout occured */
                                        log_win->DoSpinner();
                                        HandleEvents();
                                        //                                        panel_win->EventHandler();
                                    }
                                    else if (iResult != -1) {
                                        log_win->printw("About to Call Accept:");
                                        log_win->refresh();

                                        // Accept a client socket
                                        Data->ClientSocket = accept(ListenSocket, NULL, NULL);

                                        if (Data->ClientSocket != INVALID_SOCKET) {
                                            log_win->printw("About to Spawn:");
                                            log_win->refresh();
#ifdef _POSIX_VERSION
                                            hThread = pthread_create(&ThreadId, NULL, &SlaveThread, Data);
#else
                                            hThread = CreateThread(
                                                NULL,                   // default security attributes
                                                0,                      // use default stack size  
                                                SlaveThread,            // thread function name
                                                Data,                   // argument to thread function 
                                                0,                      // use default creation flags 
                                                &ThreadId);             // returns the thread identifier
#endif
                                            log_win->printw("About to Wait:");
                                            log_win->refresh();
                                        }
                                        else {
                                            log_win->printw("accept failed with error: %d", WSAGetLastError());
                                            EveythingOK = false;
                                        }
                                    }
                                    else {
                                        if (WSAGetLastError() != EINTR) {
                                            log_win->printw("Select Error %d", WSAGetLastError()); /* an error accured */
                                            EveythingOK = false;
                                        }
                                    }
                                }
                            }
                            else {
                                log_win->printw("listen failed with error: %d", WSAGetLastError());
                                EveythingOK = false;
                            }
                        }
                        else {
                            log_win->printw("bind failed with error: %d", WSAGetLastError());
                            EveythingOK = false;
                        }
                        // No longer need server socket
                        closesocket(ListenSocket);
                    }
                    else {
                        log_win->printw("socket failed with error: %ld", WSAGetLastError());
                        EveythingOK = false;
                    }
                }
                else {
                    log_win->printw("getaddrinfo failed with error: %d", iResult);
                    EveythingOK = false;
                }
#ifdef _POSIX_VERSION
#else
                WSACleanup();
#endif			
            }
            else {
                log_win->printw("WSAStartup failed with error: %d", iResult);
                EveythingOK = false;
            }

            if (!EveythingOK) {
                log_win->printw("Press any key to exit");
                log_win->refresh();
#ifdef _POSIX_VERSION
                mygetch();
#else
                _getch();
#endif
            }

            LOCKMUTEX(threadlock); /* Wait till theads exit */

            delete log_win;
            delete panel_win;
        }
        else {
            EveythingOK = false;
            printf("Complex Window Creation failed");
            printf("Press any key to exit");
#ifdef _POSIX_VERSION
            mygetch();
#else
            _getch();
#endif
        }

        endwin();			/* End curses mode		  */
    }
    else {
        EveythingOK = false;
        printf("initscr failed");
        printf("Press any key to exit");
#ifdef _POSIX_VERSION
        mygetch();
#else
        _getch();
#endif
    }

    if (GPIO::GPIOs != (GPIO *)-1) {
        delete GPIO::GPIOs;
    }

#ifdef _POSIX_VERSION
    pthread_mutex_destroy(&curseslock);
    pthread_mutex_destroy(&threadlock);
#else
    CloseHandle(curseslock);
    CloseHandle(threadlock);
#endif    
    return EveythingOK ? 0 : 1;
}

