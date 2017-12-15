/* ============================================================================
 * Name			: Client.c
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Class for the client's socket operations
 * ============================================================================ */

#include "Server.h"

Server* newServer() {															// Create a new server
	Server* server = (Server*) malloc(sizeof(Server));
	return server;
}

char * server_getMessage(Server* server, int type, int timeout) {				// Receive a network message

	struct pollfd mypoll[1];
	mypoll[0].fd = server->new_socket;
	mypoll[0].events =  POLLIN;
	mypoll[0].revents = -1;

	#if WIN64 || WIN32
		int poll_ = WSAPoll(mypoll, 1, timeout);
	#elif UNIX|| __linux__
		int poll_ = poll(mypoll, 1, timeout);
	#endif

	if( poll_ > 0 ) {
		if (type == 0) {												// receive STRING
			static char json[101];
			int json_size = recv(server->new_socket, json, sizeof(json)-1, 0);
			if (json_size != SOCKET_ERROR) {
				json[json_size] = '\0'; 								// add a NULL terminating character ('\0')
				return json;
			} else
				return NULL;
		} else if (type == 1){											// receive INTEGER
			int tmp_a;
			int32_t ret_a = recv(server->new_socket, &tmp_a, sizeof(tmp_a),0);
			if (ret_a != SOCKET_ERROR) {
				char *buff = malloc (sizeof (int) * 1);				//char buff[8];
				snprintf(buff, sizeof (int), "%d", ntohl(tmp_a));	//sizeof(buff)
				return buff;
			} else
				return NULL;
		} else
			return NULL;
	} else {
		return NULL;													// nothing received....
	}
}

int server_sendMessage(Server* server, int type, int timeout, char * json) {	// Send a network message

	int a;
	if ( (a = getCharTimeout(timeout)) < 0 )
		a = 46;															// skip key

	if (type == 0) {													// send string
		char buff[100];												// 100 because without ('\0')
		char *message1 = buff;
		sprintf(message1, "%s", json);
		send(server->new_socket , message1 , strlen(message1) , 0);
	} else if (type == 1) {												// send integer
		int buff = htonl((uint32_t)a);
		send(server->new_socket , &buff , sizeof(buff), 0);
	}

	return a;
}

int server_open(Server* server, int prot, int nonBlocking){						// Wait for a client and connect

	#if WIN64 || WIN32
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,2), &wsaData) == SOCKET_ERROR) {		//Initialise winsock library with: (version to load, structure holder)
			printf ("Error initialising WSA.\n");
			return -1;
		}
	#endif

	//Creates a socket inside the kernel and returns an integer (socket descriptor)
	server->listenfd = 0;
	if (prot == 0)
		server->listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// IPv4, TCP, IPPROTO_TCP (0 default)
	else if (prot == 1)
		server->listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// IPv4, UDP, IPPROTO_UDP
	if (server->listenfd == INVALID_SOCKET) {
		#if WIN64 || WIN32
			printf("Socket %d rejected with error code : %d", server->listenfd, WSAGetLastError());
		#else
			printf("Socket %d rejected with error code : %d", server->listenfd, errno);
		#endif
		fflush(NULL);
	}

	// memset() copies a char ('0') to the first (sizeof...) elements of the string pointed to
	server->server_port = 5000;
	memset(&server->serv_addr, '0', sizeof(server->serv_addr));
	memset(server->sendBuff, '0', sizeof(server->sendBuff));
	server->serv_addr.sin_family = AF_INET;
	server->serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server->serv_addr.sin_port = htons(server->server_port);

	// Bind() assigns the details specified in the structure serv_addr to the socket
	if( bind(server->listenfd, (struct sockaddr*)&server->serv_addr, sizeof(server->serv_addr)) == SOCKET_ERROR) {
		#if WIN64 || WIN32
			//printf("Bind failed with error code : %d" , WSAGetLastError());
		#else
			printf("Bind failed with error code : %d\n", errno);
		#endif
	}
	listen(server->listenfd, 10);	// Puts the socket in listening mode, for a maximum number of client connections

	///Creates a socket inside the kernel and returns an integer (socket descriptor)
	if (prot == 0)
		server->new_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// IPv4, TCP, IPPROTO_TCP (0 default)
	else if (prot == 1)
		server->new_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// IPv4, UDP, IPPROTO_UDP
	if (server->new_socket == INVALID_SOCKET) {
		#if WIN64 || WIN32
			printf("Socket %d rejected with error code : %d", server->new_socket, WSAGetLastError());
		#else
			printf("Socket %d rejected with error code : %d", server->new_socket, errno);
		#endif
		fflush(NULL);
	}

	// memset() copies a char ('0') to the first (sizeof...) elements of the string pointed to
	memset(&server->client_addr, '0', sizeof(server->client_addr));
	server->client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server->client_addr.sin_family = AF_INET;
	server->client_addr.sin_port = htons(server->server_port);

	//Set socket as Non-Blocking
	if (nonBlocking == 1) {
		#if WIN64 || WIN32
			unsigned long on = 1;
			if (0 != ioctlsocket(server->listenfd, FIONBIO, &on)) {
				perror("ioctlsocket failed.");
			}
			if (0 != ioctlsocket(server->new_socket, FIONBIO, &on)) {
				perror("ioctlsocket failed.");
		}
		#elif UNIX|| __linux__
			int reUse = 1;
			if (setsockopt(server->listenfd, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse)) < 0)
				perror("setsockopt(SO_REUSEADDR) failed");
			if (setsockopt(server->new_socket, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse)) < 0)
				perror("setsockopt(SO_REUSEADDR) failed");
			int flags = fcntl(server->new_socket, F_GETFL, 0);
			fcntl(server->new_socket, F_SETFL, flags | O_NONBLOCK);
		#endif
	}

	//Wait until an incoming connection is ACCEPTED
	unsigned int c = sizeof(struct sockaddr_in);
	if( (server->new_socket = accept(server->listenfd , (struct sockaddr *)&server->client_addr, &c)) != INVALID_SOCKET ) {
		char ipbuf[INET_ADDRSTRLEN];
		snprintf(server->client_ip, sizeof(server->client_ip), "%s", inet_ntop(AF_INET, &server->client_addr.sin_addr.s_addr, ipbuf, sizeof(ipbuf)));
		log2file(1, "Connection from ", server->client_ip);
		return 1;
	} else
		return 0;

}

int server_close(Server* server) {												// Disconnect from a client
	//printf("%s\n", "Server down...");
	#if WIN64 || WIN32
		closesocket(server->listenfd);
		closesocket(server->new_socket);
		WSACleanup();
	#endif
	free(server);
	return 0;
}

/*int main_server(int argc, char *argv[]) {										// Test server operations
	Server* server = newServer();
	server_open(server);
	//server_getMessage(server);
	//server_sendMessage(server, 999);
   	server_close(server);
}*/
