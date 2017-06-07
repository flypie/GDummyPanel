/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Fudge.h
 * Author: John Bradley
 *
 * Created on 30 April 2017, 21:33
 */

#ifndef FUDGE_H
#define FUDGE_H

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#endif

#if defined(_POSIX_VERSION) || defined(_POSIX2_C_VERSION)
#include <termios.h>
#include <pthread.h>
#include <string.h>
#else
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include <windows.h>
#include <process.h>
#undef MOUSE_MOVED
#endif

#include "curses.h"
#include <panel.h>

#ifdef _POSIX_VERSION

#define SOCKET  int
#define HANDLE  int
#define DWORD   unsigned long
#define closesocket close
#define INVALID_SOCKET  -1
#define SOCKET_ERROR -1
#define WSAGetLastError() errno 

extern pthread_mutex_t curseslock; //NCurses not thread  safe/
extern pthread_mutex_t threadlock; //NCurses not thread  safe/

#define LOCKMUTEX(x)   pthread_mutex_lock(&x);
#define UNLOCKMUTEX(x) pthread_mutex_unlock(&x);
#else

extern HANDLE  curseslock; //NCurses not thread  safe/

/* No ; so as not to mess up VisC auto format*/

#define LOCKMUTEX(x)   WaitForSingleObject(x, 0xFFFFFFFF )
#define UNLOCKMUTEX(x) ReleaseMutex(x)

#endif

#define CURSESWHITEONBLACK    COLOR_PAIR(0)
#define CURSESBLACKONWHITE    COLOR_PAIR(1)

#define REDONBLACK      COLOR_PAIR(2)
#define GREENONBLACK    COLOR_PAIR(3)
#define BLUEONBLACK     COLOR_PAIR(4)
#define CYANONBLACK     COLOR_PAIR(5)
#define MAGENTAONBLACK  COLOR_PAIR(6)
#define YELLLOWONBLACK  COLOR_PAIR(7)

#define BLACKONRED      COLOR_PAIR(8)
#define BLACKONGREEN      COLOR_PAIR(9)
#define BLACKONBLUE      COLOR_PAIR(10)
#define BLACKONCYAN      COLOR_PAIR(11)
#define BLACKONMAGENTA      COLOR_PAIR(12)
#define BLACKONYELLOW      COLOR_PAIR(13)

#define REDONWHITE      COLOR_PAIR(14)
#define GREENONWHITE      COLOR_PAIR(15)
#define BLUEONWHITE      COLOR_PAIR(16)
#define CYANONWHITE      COLOR_PAIR(17)
#define MAGENTAONWHITE      COLOR_PAIR(18)
#define YELLLOWONWHITE      COLOR_PAIR(19)

#define WHITEONRED      COLOR_PAIR(20)
#define WHITEONGREEN      COLOR_PAIR(21)
#define WHITEONBLUE      COLOR_PAIR(22)
#define WHITEONCYAN      COLOR_PAIR(23)
#define WHITEONMAGENTA      COLOR_PAIR(24)
#define WHITEONYELLOW      COLOR_PAIR(25)


typedef int EVENTTYPE;

#define MAXSTRLEN  256

extern void rectangle(WINDOW *Win, int y1, int x1, int y2, int x2);
extern char *padstr(char *s, int length);
extern void colorbox(WINDOW *win, chtype color, int hasbox);

#endif /* FUDGE_H */

