/* ============================================================================
 * Name			: Tools.h
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Header of the Tools class
 * ============================================================================ */

#include <errno.h>
#include <fcntl.h>	//for nonblocking sockets
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if WIN64 || WIN32
	//#include <mstcpip.h>						// faulty library
	#include <stdint.h>
	#include <Ws2tcpip.h>
	#include <Windows.h>
	#include <Winsock2.h>						// header file to be included for winsock functions
	#pragma comment (lib, "Ws2_32.lib")			//library to use winsock functions
	#pragma comment (lib, "Mswsock.lib")
	#pragma comment (lib, "AdvApi32.lib")
	#define STDIN_FILENO 0
#elif UNIX|| __linux__
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <poll.h>
	#include <strings.h>
	#include <sys/ioctl.h>
	#include <sys/kd.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <unistd.h>
	#define SOCKET_ERROR  ((int)-1)
	#define INVALID_SOCKET  ((int)-1)
#else
	#error Not a supported platform
#endif


#ifndef __Tools__
#define __Tools__

	void beep(int freq, int dur);							// play a sound (e.g. when ball goes out of board)
	void clear();											// clears the screen
	int getCharTimeout(int timeout);						// input a char within a time limit
	int log2file(int referrer, char *text1, char *text2);	// write log about the game

#endif
