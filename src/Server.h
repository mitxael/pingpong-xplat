/* ============================================================================
 * Name			: Server.h
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Header of the Server class
 * ============================================================================ */

#include "Tools.h"

#ifndef SERVER_H_
#define SERVER_H_

typedef struct Server {																	// Server class
		char client_ip[1024];
		u_short server_port;
		struct sockaddr_in serv_addr,client_addr;
		short int listenfd;
		short int new_socket;
		char sendBuff[1025];
		long long unsigned int buffSize;
	} Server;

	Server* newServer ();																// Constructor
	char * server_getMessage(Server* server, int type, int timeout);					// Receive data
	int server_sendMessage(Server* server, int type, int timeout, char * json);			// Send data
	int server_open(Server* server, int prot, int nonBlocking);							// Open connection
	int server_close(Server* server);													// Close connection

#endif /* SERVER_H_ */
