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


#ifdef _POSIX_VERSION

#define SOCKET  int
#define HANDLE  int
#define DWORD   unsigned long
#define closesocket close
#define INVALID_SOCKET  -1
#define SOCKET_ERROR -1
#define WSAGetLastError() errno 

#endif 

#endif /* FUDGE_H */

