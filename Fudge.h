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

#ifdef _POSIX_VERSION
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

#ifdef _POSIX_VERSION

#define SOCKET  int
#define HANDLE  int
#define DWORD   unsigned long
#define closesocket close
#define INVALID_SOCKET  -1
#define SOCKET_ERROR -1
#define WSAGetLastError() errno 

extern pthread_mutex_t lock; //NCurses not thread  safe/

#define LOCKMUTEX   pthread_mutex_lock(&lock);
#define UNLOCKMUTEX pthread_mutex_unlock(&lock);
#else

extern HANDLE  lock; //NCurses not thread  safe/

#define LOCKMUTEX   WaitForSingleObject(lock, 0xFFFFFFFF );
#define UNLOCKMUTEX ReleaseMutex(lock);

#endif

#endif /* FUDGE_H */

