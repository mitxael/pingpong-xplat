/* ============================================================================
 * Name			: PingPong.h
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Header of the PingPong class
 * ============================================================================ */

#include "Tools.h"
#include "Client.h"
#include "Server.h"

#if WIN64 || WIN32
	#include <regex.h>;	/*gnuwin32.sourceforge.net/packages/regex.htm
	Solution > Properties > Configuration Properties > Linker > General > Additional Library Directories > Edit... > C:\Program Files (x86)\GnuWin32\lib
	Solution > Properties > Configuration Properties > Linker > Input > Additional Dependencies > Edit...> "libregex.dll.a libregex.la"
	Solution > Properties > Configuration Properties  > C/C++ > General > Additional Include Directories > Edit... > C:\Program Files (x86)\GnuWin32\include
	Solution > Properties > Configuration Properties > C/C++ > Code Generation > Runtime Library > Multi-threaded DLL
	Windows > System Environment Variables > %PATH% > New > C:\Program Files (x86)\GnuWin32\bin\  */
	#pragma comment(lib,"regex.lib")
#elif UNIX|| __linux__
	#include <regex.h>
#else
	#error Not a supported platform
#endif

#ifndef __PingPong__
#define __PingPong__

	struct pair {
		    int first;
		    int second;
	};

	typedef struct PingPong {
		int board_w;					// board weight
		int board_h;					// board height
		int stick_len;					// stick length
		int step_len;					// ball's step length
		int speed;						// player's move timeout (ms)
		int p1_score;					// points of player 1
		int p2_score;					// points of player 2					
		enum {
			LOCAL = 0,
			SERVER,
			CLIENT
		} mode;							// game mode
		enum {
			DISCONNECTED = 0,
			CONNECTED,
		} connected;					// connections status
		enum {
			RUNNING = 0,
			IDLE,
		} paused;						// indicator of idle game 
		int counter;					// number of performed moves
		int lastPoint;					// number of moves at which last point occurred
		char * status;					// message displayed in the bottom status
		char * ip;						// IP of the server
		char * port;					// Port of the server
		Server* server;					// Game server
		Client* client;					// Game client
		#define p1_UP 113				// q	(72: up arrow)
		#define p1_DOWN 97				// a	(80: down arrow)
		#define p2_UP 112				// p
		#define p2_DOWN 108				// l
		#define escape 27				// esc
		#define skip 46					// .
		#define backspace 32			// backspace [  ]
		//www.theasciicode.com.ar

		struct {
			int x, y_start, y_end;		// position
			int size;					// size of the stick
		} player1, player2;				// left and right players

		struct {
			int x, y;					// position
			struct {
				int x, y;
			} step;						// contains information about how fast the ball moves
			enum {
				LEFT_DOWN = 1,
				LEFT_UP,
				RIGHT_DOWN,
				RIGHT_UP
			} direction;				// contains information about what direction the ball should move in
		} ball;
	} PingPong;

	PingPong* newGame  (int board_w, int board_h, int stick_len, int step_len, int speed, int mode, char * ip, char * port);	// constructor
	void newSet (PingPong* game);					// initializes the player1, player2, ball structures
	void display (PingPong* game);					// prints game process
	void start(PingPong* game);						// start the game
	void finish(PingPong* game, int force);			// finish the game and free memory
	int position (PingPong* game, struct pair t);	// calculates next positions of objects (player1, player2, ball, etc.)
	void reverse (PingPong* game, char axis);		// reverses the direction of the ball, horizontally or vertically
	struct pair getMove (PingPong* game);			// get move from local player
	void turn (PingPong* game);						// get move from remote player
	char *jsonSerializer(PingPong* game);			// encode data in interchangeable format
	int jsonParser(PingPong* game, char *json);		// decode data from interchangeable format
	void idle(PingPong* game, int forced);			// pause the game until a key is pressed

#endif
