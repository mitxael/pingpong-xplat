/* ============================================================================
 * Name			: Main.c
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Main program
 * ============================================================================ */

#include "PingPong.h"
#include "Server.h"
#include "Client.h"
#include "Tools.h"

#if WIN64 || WIN32
	#include <io.h>
	;//Solution > Properties > Configurations Properties > C/C++ > Preprocessor Definitions > Edit... > _CRT_SECURE_NO_WARNINGS
#elif UNIX|| __linux__
	#include <ctype.h>
	#include <setjmp.h>
	#include <sgtty.h>
	#include <signal.h>
	#include <sys/ioctl.h>
#else
	#error Not a supported platform
#endif

int mode;

int menu(){	// Welcome user and ask for the "game mode"

	int option;
	clear();
	printf ("****************************************************\n\033[2;1H"
				"***                                              ***\n\033[3;1H"
				"***              Welcome to NetPong!             ***\n\033[4;1H"
				"***         a game developed by <mitXael>        ***\n\033[5;1H"
				"***                                              ***\n\033[6;1H"
				"****************************************************\n\033[7;1H"
				"*** The resolution of the console must be set to ***\n\033[8;1H"
				"*** 80x28 in order to display graphics correctly ***\n\033[9;1H"
				"****************************************************\n\033[10;1H");
	fflush(stdout);

	for (;;) {
	    printf("\nPlay NetPong as:\n");
	    printf("\t1> SERVER \n\t2> CLIENT \n\t3> Exit\n\n");
	    printf("Enter option: ");
	    option = getchar();
	    switch(option) {
	     case '1':
	    	 printf("\nPlay as SERVER!\n\n");
	    	 return 1;
	      	 break;
	     case '2':
	    	 printf("\nPlay as CLIENT!\n\n");
	    	 return 2;
	    	 break;
	     case '3':
	    	 exit(0);
	     default: printf("\nInvalid option!\n\n");
	    }
	    getchar();
	  }
	return 1;
}

void setSize() {	// Set size of the console
	int w;
	int h;
	#if WIN64 || WIN32
		w = 81;
		h = 21;
		HANDLE wHnd = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE rHnd = GetStdHandle(STD_INPUT_HANDLE);
		//SetConsoleTitle("NETPONG by mitXael");
		SMALL_RECT windowSize = { 0, 0, w, h };
		SetConsoleWindowInfo(wHnd, 1, &windowSize);
		COORD bufferSize = { 10, 10 };
		SetConsoleScreenBufferSize(wHnd, bufferSize);
	#elif UNIX|| __linux__
		w = 80;
		h = 28;
		int fd = open("/dev/tty", O_WRONLY | O_RDWR);	// Open the controlling terminal
		struct winsize ws;
		ws.ws_row = h;
		ws.ws_col = w;
		ws.ws_xpixel=0;
		ws.ws_ypixel=0;
		/*if (ioctl(fd, TIOCGWINSZ, &ws) == 0)	// Get window size of terminal
			printf("%d x %d\n", ws.ws_row, ws.ws_col);*/
		ioctl (fd, TIOCSWINSZ, &ws);	// Set window size of terminal
		close(fd);

		system("konsoleprofile colors=GreenOnBlack");
		/*system("konsoleprofile TerminalRows=240");
		system("konsoleprofile TerminalColumns=800");*/
		//system("konsole --nofork --geometry=580x450-0+0");
	#endif
}

int main(int argc, char* argv[]) {	// Main

	char ip[1024];
	char port[8];
	
	setSize();

	if (argc > 2) {								// if 2 arguments  [IP, PORT]
		mode = 2;								// Client mode
		strncpy(ip, argv[1], sizeof(ip));
		strncpy(port, argv[2], sizeof(port));
	}
	else if (argc > 1) {						// if 1 argument [PORT]
		mode = 1;								// Server mode
		strncpy(ip, "0", sizeof(ip));
		strncpy(port, argv[1], sizeof(port));
	}
	else {									// if no arguments []
		mode = menu();
		if (mode == 1) {						// Server mode
			strncpy(ip, "0", sizeof(ip));
			strncpy(port, "0", sizeof(port));
		}
		else if (mode == 2) {					// Client mode
			printf("\tEnter the Server IP: (enter '0' to set default 127.0.0.1)\n");
			scanf("%s", ip);
			if (ip[0] == '0')
				strncpy(ip, "127.0.0.1", sizeof(ip));
			printf("\tEnter the Server Port: (enter '0' to set default 5000\n");
			scanf("%s", port);
			if (port[0] == '\n')
				strncpy(port, "5000", sizeof(port));
		}
	}
	#if WIN64 || WIN32
		Sleep(2000);
	#elif UNIX|| __linux__
		sleep(2);
	#endif

	PingPong* game = newGame(80, 25, 3, 1, 100, mode, ip, port);	// (weight, height, size, step, speed, mode, ip, port)
	start(game);
	finish(game, 0);

	return 0;
}
