/* ============================================================================
 * Name			: Client.h
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Header of the Client class
 * ============================================================================ */

#include "Tools.h"

#ifndef CLIENT_H_
#define CLIENT_H_

	typedef struct Client {															// Client class
		char server_ip[1024];
		u_short server_port;
		struct sockaddr_in serv_addr;
		short int sockfd;
		char recvBuff[1024];
	} Client;

	Client* newClient (char * server_ip, char * server_port);						// Constructor
	char * client_getMessage(Client* client, int type, int timeout);				// Receive data
	int client_sendMessage(Client* client, int type, int timeout, char * json);		// Send data
	int client_open(Client* client, int prot, int nonBlocking);						// Open connection
	int client_close(Client* client);												// Close connection

#endif /* CLIENT_H_ */
