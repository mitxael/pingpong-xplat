/* ============================================================================
 * Name			: Client.c
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Class for the client's socket operations
 * ============================================================================ */

#include "Client.h"

Client* newClient(char * server_ip, char * server_port) {						// Create the client
	Client* client = (Client*) malloc(sizeof(Client));
	if(*server_ip == '0' ) {
		printf("Ip missing. 127.0.0.1 will be used. \n");
		#if WIN64 || WIN32
		strncpy(client->server_ip, "127.0.0.1", sizeof(client->server_ip)); //strncpy_s(client->server_ip, sizeof(client->server_ip), "127.0.0.1", sizeof(client->server_ip));
		#elif UNIX|| __linux__
			strncpy(client->server_ip, "127.0.0.1", sizeof(client->server_ip));
		#endif
	} else
		#if WIN64 || WIN32
		strncpy(client->server_ip, server_ip, sizeof(client->server_ip)); //strncpy_s(client->server_ip, sizeof(client->server_ip), server_ip, sizeof(client->server_ip));
		#elif UNIX|| __linux__
			strncpy(client->server_ip, server_ip, sizeof(client->server_ip));
		#endif

	if(*server_port == '0' ) {
		printf("Port missing. 5000 will be used. \n");
		client->server_port = 5000;
	} else
		client->server_port = atoi(server_port);
	return client;
}

char * client_getMessage(Client* client, int type, int timeout) {				// Receive a network message

	struct pollfd mypoll[1];
	mypoll[0].fd = client->sockfd;
	mypoll[0].events = POLLIN;
	mypoll[0].revents = -1;

	#if WIN64 || WIN32
		int poll_ = WSAPoll(mypoll, 1, timeout);
	#elif UNIX|| __linux__
		int poll_ = poll(mypoll, 1, timeout);
	#endif

	if( poll_ > 0 ) {
	 	static char json[101];//100+1
	    int json_size;
	    if ( (json_size = recv(client->sockfd , json , sizeof(json)-1 , 0)) != SOCKET_ERROR ) {
	    	json[json_size] = '\0'; //Add a NULL terminating character ('\0')
	    	if (type == 0) {		// Receive string
	        	;
	        } else if (type == 1) { // Receive integer
	    		;					// atoi(json);
	    	}
	    	return json;
	    } else
	    	return NULL;			// nothing
	} else
    	return NULL;				// nothing
}

int client_sendMessage(Client* client, int type, int timeout, char * json) {	// Send a network message

	int a;
	if ((a = getCharTimeout(timeout)) < 0)
		a = 46;

	if (type == 0) {
		char buff[100];			// 100 because without ('\0')
		char *message1 = buff;
		sprintf(message1,"%s",json);
		if ( send(client->sockfd, message1, strlen(message1), 0) < 0)
			printf("Error while sending data.");

	} else if (type == 1) {
		int buff = htonl((uint32_t)a);
		if ( send(client->sockfd, &buff, sizeof(buff), 0) < 0)
			printf("Error while sending data.");
	}

	return a;
}

int client_open(Client* client, int prot, int nonBlocking) {					// Connect to a server

	#if WIN64 || WIN32
		//Initialise winsock library with: (version to load, structure holder)
		WSADATA wsaData;
		if (WSAStartup(MAKEWORD(2,2), &wsaData) == SOCKET_ERROR) {
			printf ("Error initialising WSA.\n");
			return -1;
		}
	#endif

	//Create a socket with inside the kernel and returns an integer (socket descriptor)
	client->sockfd = 0;
	memset(client->recvBuff, '0',sizeof(client->recvBuff));
	if (prot == 0)
		client->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// IPv4, TCP, IPPROTO_TCP (0 default)
	else if (prot == 1)
		client->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// IPv4, UDP, IPPROTO_UDP
	if (client->sockfd == INVALID_SOCKET) {
		#if WIN64 || WIN32
			printf("Socket %d rejected with error code : %d", client->sockfd, WSAGetLastError());
		#else
			printf("Socket %d rejected with error code : %d", client->sockfd, errno);
		#endif
		fflush(NULL);
	}

	//Set socket as Non-Blocking
	if (nonBlocking == 1) {
		int reUse = 1;
		if (setsockopt(client->sockfd, SOL_SOCKET, SO_REUSEADDR, &reUse, sizeof(reUse)) < 0)
			perror("setsockopt(SO_REUSEADDR) failed");
	}

	//Connect to a remote server, creating a sockaddr_in structure
	memset(&client->serv_addr, '0', sizeof(client->serv_addr));
	client->serv_addr.sin_addr.s_addr = inet_addr(client->server_ip);
	client->serv_addr.sin_family = AF_INET;
	client->serv_addr.sin_port = htons(client->server_port);

	//Convert "server_ip" into a network address structure and copy to "&serv_addr"
	if(inet_pton(AF_INET, client->server_ip, &client->serv_addr.sin_addr) <= 0) {
		printf("\n inet_pton error occured\n");
	}

	//Connect the client socket with a server socket (with IP address and port bundled up in a structure)
	#if WIN64 || WIN32
		if(WSAConnect(client->sockfd,(struct sockaddr*)&client->serv_addr,sizeof(client->serv_addr),NULL, NULL, NULL, NULL) == SOCKET_ERROR){//< 0){
			printf(" Server unavailable. (sockfd: %d; wsaerror: %d\n)  ", client->sockfd, WSAGetLastError());
			return 0;
		}
	#else
		if(connect(client->sockfd,(struct sockaddr*)&client->serv_addr,sizeof(client->serv_addr)) == SOCKET_ERROR){//< 0){
			printf(" Server unavailable. (sockfd: %d; errno: %d)\n", client->sockfd, errno);
			return 0;
		}
	#endif
	fflush(NULL);

	// Log established connection
	log2file(2, "Connected to ", client->server_ip);

    return 1;
}

int client_close(Client* client) {												// Disconnect from a server
	//printf("%s\n", "Client down...");
	#if WIN64 || WIN32
		closesocket(client->sockfd);
		WSACleanup();
	#endif
	free(client);
	return 0;
}

/*int main_client(int argc, char *argv[])										// Test client operations
{
	Client* client = newClient(argv[1], argv[2]);
	client_open(client);
	//client_getMessage(client);
	//client_sendMessage(client, 111);
	client_close(client);
}*/
