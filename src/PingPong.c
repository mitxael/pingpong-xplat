/* ============================================================================
 * Name			: PingPong.c
 * Author		: Michael Vasquez Otazu	081556
 * Version		: 1.0
 * Copyright	: ©mitxael
 * Description	: Ping-Pong game that allows two network users to play against
 * ============================================================================
 * Input		: Players move the stick using the keyboard
 * Output		: A console-based graphical board shows a ball bouncing
 * ============================================================================ */

#include "PingPong.h"

PingPong* newGame(int board_w, int board_h, int stick_len, int step_len, int speed, int mode, char * ip, char * port) {
	PingPong* game = (PingPong*) malloc(sizeof(PingPong));
	game->board_w = board_w;
	game->board_h = board_h;
	game->stick_len = stick_len;
	game->step_len = step_len;
	game->speed = speed;
	game->p1_score = 0;
	game->p2_score = 0;
	game->counter = 0;
	game->lastPoint = 0;
	game->connected = DISCONNECTED;
	game->paused = RUNNING;
	game->ball.direction = RIGHT_DOWN;
	game->mode = mode;
	game->ip = ip;
	game->port = port;
	return game;
}

void newSet(PingPong* game) {

	game->player1.size = game->stick_len;
	game->player1.x = 0;
	game->player1.y_start = game->board_h / 2 - (game->player1.size / 2);
	game->player1.y_end = game->player1.y_start + (game->player1.size - 1);

	game->player2.size = game->stick_len;
	game->player2.x = game->board_w - 1;
	game->player2.y_start = game->board_h / 2 - (game->player2.size / 2);
	game->player2.y_end = game->player2.y_start + (game->player2.size - 1);

	game->ball.x = game->board_w / 2;
	game->ball.y = game->board_h / 2;
	game->ball.step.x = game->step_len;
	game->ball.step.y = game->step_len;
}

void display(PingPong* game) { // display on screen the current position of the ball and players
	clear(game);
	int i = 0;				// on-screen X coordinates
	int j = 0;				// on-screen Y coordinates
	while (i < game->board_h) {		// do for all rows
		j = 0;					// restart from first column
		while (j < game->board_w) {	// do for all columns
			if (j == game->player1.x
					&& (i >= game->player1.y_start && i <= game->player1.y_end))// if player1's position
				fputc(123, stdout);	//printf("{");
			else if (j == game->player2.x
					&& (i >= game->player2.y_start && i <= game->player2.y_end))// if player2's position
				fputc(125, stdout);	//printf("}");
			else if (j == game->ball.x && i == game->ball.y)// if ball's position
				fputc(111, stdout);	//printf("o");
			else					// if none's position
				fputc('.', stdout);	//printf(".");
			++j;
		}
		++i;
	}
	printf("\n[P1: %d] - [P2: %d]\t", game->p1_score, game->p2_score);
	printf("\tball (%d, %d)\t P1 (%d, %d)\t P2 (%d, %d)\t", game->ball.x,
			game->ball.y, game->player1.x, game->player1.y_start,
			game->player2.x, game->player2.y_start);
	printf(" <== %s ==>", game->status);
	fflush(stdout);
}

void start(PingPong* game) {

	clear();
	log2file(game->mode, "************************** Game Started *************************", "");

	system("/bin/stty raw");		// make terminal send all keystrokes directly to stdin
	newSet(game);					// initializes the structures
	game->status = (game->mode == 1) ? "Waiting for a client..." : "Looking for a server...";
	display(game);					// set initial position for ball and players

	if (game->mode == 1) {			// Server and client are created once at the game start
		game->server = newServer();
		game->connected = server_open(game->server, 0, 0); // PROT: 0=TCP; 1=UDP
	}
	else if (game->mode == 2) {
		game->client = newClient(game->ip, game->port);
		game->connected = client_open(game->client, 0, 0); // PROT: 0=TCP; 1=UDP
	}

	turn(game);						// Start making moves

	if (game->mode == 1)			// Shutdown server and client
		game->connected = server_close(game->server);
	else if (game->mode == 2)
		game->connected = client_close(game->client);
}

void finish(PingPong* game, int force) {

	if (force == 0) {				// if regular exit, and ask the other player to do so
		char * cmd = "<START><cmd>1<END>";
		if (game->mode == 1) {
			server_sendMessage(game->server, 0, game->speed, cmd);
		}
		else if (game->mode == 2) {
			client_sendMessage(game->client, 0, game->speed, cmd);
		}
	}

	clear(game);

	if (game->p1_score > game->p2_score) {
		if (game->mode == 1)
			printf("You won :)");
		else if (game->mode == 2)
			printf("You lost :(");
	}
	else if (game->p2_score > game->p1_score) {
		if (game->mode == 1)
			printf("You lost :(");
		else if (game->mode == 2)
			printf("You won :)");
	}
	else {
		printf("Game result: Draw!");
	}

	printf("\t(P1: %d - P2: %d) \t\t\t\t\t\t\t", game->p1_score, game->p2_score);
	printf(" The <escape> key has been pressed, the game is over.  ");
	system("/bin/stty cooked");	// set terminal behavior to more normal behavior
	free(game);
	log2file(game->mode, "+++ Game Finished +++", "");
	exit(0);
}

int position(PingPong* game, struct pair t) {// calculate the next position of ball and players

	game->counter++;	// increase number of performed moves

	//Ball hits up, down or players' stick
	if (((game->ball.x == (game->player1.x + 1))
			&& (game->ball.y >= game->player1.y_start
					&& game->ball.y <= game->player1.y_end))
			|| ((game->ball.x == (game->player2.x - 1)
					&& (game->ball.y >= game->player2.y_start
							&& game->ball.y <= game->player2.y_end))))
		reverse(game, 'h');
	else if ((game->ball.y == 0) || (game->ball.y == (game->board_h - 1)))
		reverse(game, 'v');

	//Ball overreaches right/left border
	if ((game->ball.x == 0) || (game->ball.x == (game->board_w - 1))) {	// Ball miss
		if (game->lastPoint < (game->counter - 5 )) { // prevent "ghost" points caused by replicating server's last move
			if (game->ball.x == 0) {
				game->p2_score++;
				game->lastPoint = game->counter;
				log2file(game->mode, "P2 scores!", "");
				beep(440, 1000);
			}
			else if (game->ball.x == (game->board_w - 1)) {
				game->p1_score++;
				game->lastPoint = game->counter;
				log2file(game->mode, "P1 scores!", "");
				beep(440, 1000);
			}
			newSet(game);
		}

	} else { //Ball is still on board
		switch (game->ball.direction) { //Move ball
			case LEFT_DOWN:
				game->ball.x -= game->ball.step.x;
				game->ball.y += game->ball.step.y;
				break;
			case LEFT_UP:
				game->ball.x -= game->ball.step.x;
				game->ball.y -= game->ball.step.y;
				break;
			case RIGHT_UP:
				game->ball.x += game->ball.step.x;
				game->ball.y -= game->ball.step.y;
				break;
			case RIGHT_DOWN:
				game->ball.x += game->ball.step.x;
				game->ball.y += game->ball.step.y;
				break;
		}

		//Move players (if a valid key is pressed)
		int c1 = t.first;
		int c2 = t.second;
		if (c1 == p1_UP) {
			if (game->player1.y_start - 1 >= 0) {
				game->player1.y_start--;
				game->player1.y_end--;
			}
		} else if (c1 == p1_DOWN) {
			if (game->player1.y_end + 1 < game->board_h) {
				game->player1.y_start++;
				game->player1.y_end++;
			}
		} else if (c1 == skip) {
			;	//do not move player, only the ball
		} else if (c1 == backspace) {
			idle(game, 0);	//pause
		} else if (c1 == escape) {	// escape & clear screen
			finish(game, 0);
			return 0;
		}

		if (c2 == p2_UP) {
			if (game->player2.y_start - 1 >= 0) {
				game->player2.y_start--;
				game->player2.y_end--;
			}
		} else if (c2 == p2_DOWN) {
			if (game->player2.y_end + 1 < game->board_h) {
				game->player2.y_start++;
				game->player2.y_end++;
			}
		} else if (c2 == skip) {
			;//do not move player, only the ball
		} else if (c2 == backspace) {
			idle(game, 0);	//pause
		} else if (c2 == escape) { // escape & clear screen
			finish(game, 0);
			return 0;
		}
	}

	return 1;
}

void reverse(PingPong* game, char axis) {
	if (axis == 'v') {	// reverse vertically
		switch (game->ball.direction) {
		case RIGHT_UP:
			game->ball.direction = RIGHT_DOWN;
			break;
		case RIGHT_DOWN:
			game->ball.direction = RIGHT_UP;
			break;
		case LEFT_UP:
			game->ball.direction = LEFT_DOWN;
			break;
		case LEFT_DOWN:
			game->ball.direction = LEFT_UP;
			break;
		}
	} else if (axis == 'h') {	// reverse horizontally
		switch (game->ball.direction) {
		case RIGHT_UP:
			game->ball.direction = LEFT_UP;
			break;
		case RIGHT_DOWN:
			game->ball.direction = LEFT_DOWN;
			break;
		case LEFT_UP:
			game->ball.direction = RIGHT_UP;
			break;
		case LEFT_DOWN:
			game->ball.direction = RIGHT_DOWN;
			break;
		}
	}
	return;
}

struct pair getMove(PingPong* game) {
	int t1 = 0;				// temporary placeholder for player1's move
	int t2 = 0;				// temporary placeholder for player2's move
	char * msg_in;			//  = (char *) malloc(101);
	char * msg_out;			// = (char *) malloc(100);

	switch (game->mode) {
		case 0:// local game
			t1 = getCharTimeout(game->speed);
			break;
		case 1:;// Network game as server
			char info_client[2048];
			snprintf(info_client, sizeof(info_client), "Playing against %s", game->server->client_ip);
			game->status = (game->connected == 0)? "Waiting for a client..." : info_client;

			msg_out = jsonSerializer(game);
			t1 = server_sendMessage(game->server, 0, game->speed, msg_out);
			log2file(game->mode, "Json sent to client", msg_out);

			msg_in = server_getMessage(game->server, 0, game->speed);
			jsonParser(game, msg_in);
			log2file(game->mode, "Json recv from client", msg_in);

			break;
		case 2:;// Network game as client

			char info_server[2048];
			snprintf(info_server, sizeof(info_server), "Playing against %s", game->client->server_ip);
			game->status = (game->connected == 0)? "Looking for a server..." : info_server;

			msg_in = client_getMessage(game->client, 0, game->speed);
			jsonParser(game, msg_in);
			log2file(game->mode, "Json recv from server", msg_in);

			msg_out = jsonSerializer(game);
			t2 = client_sendMessage(game->client, 0, game->speed, msg_out);
			log2file(game->mode, "Json sent to server", msg_out);
			break;
	}

	struct pair t = {t1,t2};
	/*char logText[50];
	snprintf(logText, sizeof(logText), "Key pressed: P1=%d & P2=%d", t.first, t.second);
	log2file(game->mode, logText);*/

	return t;
}

void turn(PingPong* game) {
	struct pair t;							// stores code of a pressed key
	while (1)	{							// while a valid (non-escape) key is pressed
		t = getMove(game);					// get players' 1&2 move
		if (position(game, t))				// determine new positions (ball and players)
			display(game);					// display on new positions (ball and players)
	}
}

char *jsonSerializer(PingPong* game) {

	//www.eskimo.com/~scs/cclass/int/sx5.html
	static char json[100];
	if(json == NULL) {
		return NULL;
	} else {
		sprintf(json, "<START><cmd>0<ballX>%d<ballY>%d<ballZ>%d<p1YS>%d<p1YE>%d<p2YS>%d<p2YE>%d<END>",
				game->ball.x, game->ball.y, game->ball.direction,
				game->player1.y_start, game->player1.y_end,
				game->player2.y_start, game->player2.y_end);
		return json;
	}
}

int jsonParser(PingPong* game, char *json) {

	char * string = json; 			//"<cmd>0<ballX>12<ballY>24<p1YS>32<p1YE>48<p2YS>32<p2YE>48<END>";
	char * pattern = ">*([0-9]+)<";	//"<.*>" : all that is enclosed
	size_t maxMatches = 8;			// number of expressions to be returned
	#define maxGroups (2)			//size_t maxGroups = 2;	// number of parenthesized subexpressions (deep level)

	regex_t regexCompiled;
	regmatch_t arrayOfMatches[maxGroups];

	unsigned int m;				// number of Matches
	unsigned int m_start = 0;	// start from 0,to include all expressions
	unsigned int g;				// number of Groups
	unsigned int g_start = 1;	// start from 1, skipping the first group
	unsigned int offset;		// starting position of next group

	if (json != NULL) {	// Verify that json is not empty
		int i;
		char chk[7] = "<START>";
		char tmp[7];
		for (i=0; i < 7; ++i)
			tmp[i] = json[i];
		if ( strcmp(tmp, chk) == 0 ) { // Verify that json is complete
			int ballX, ballY, ballZ, P1YS, P1YE, P2YS, P2YE;
			if (regcomp(&regexCompiled, pattern, REG_EXTENDED))	{ // make the "pattern" suitable for regexec()
				printf("Unable to compile regular expression.\n");
			} else {
				for (m = m_start; m < maxMatches; m++) {
					if (regexec(&regexCompiled, string, maxGroups, arrayOfMatches, 0)){ // match the "pattern" against the "string"
						break;  // No more matches
					} else {
						for (g = g_start; g < maxGroups; g++)	{
							if (arrayOfMatches[g].rm_so == (size_t)-1) {
								break;  // No more groups
							} else {
								if (g == g_start)
									offset = arrayOfMatches[g].rm_eo;

								#if WIN64 || WIN32
									char stringCopy[100 + 1];	// 100 is the json's standard size
								#elif UNIX|| __linux__
									char stringCopy[strlen(string) + 1];
								#endif

								strcpy(stringCopy, string);
								stringCopy[arrayOfMatches[g].rm_eo] = 0;

								/*char logText[100];
								snprintf(logText, sizeof(logText),"Match %u, Group %u: [%2u-%2u]: %s\n", m, g,		arrayOfMatches[g].rm_so, arrayOfMatches[g].rm_eo,		stringCopy + arrayOfMatches[g].rm_so);
								log2file(game,logText, "");*/

								char * value = malloc(arrayOfMatches[g].rm_eo - arrayOfMatches[g].rm_so);	// size is group's length
								sprintf(value, "%s", stringCopy + arrayOfMatches[g].rm_so);
								int n = atoi(value);	// sscanf(value, "%d", &n);

								switch (m) {
								case 0:
									if (n == 0){
										continue; 			// cmd = update (read position from parameters)
									} else if (n == 1){
										finish(game, 1);	// cmd = exit (finish game)
										return 0;
									} else if (n == 2){
										idle(game, 1);		// cmd = idle (pause game)
										return 0;
									}
									else if (n == 3){
										game->paused = 0;	// cmd = resume (resume game)
										return 0;
									} else
										return -1;			// cmd = unknown (exit parser)
									break;
								case 1:
									ballX = n;
									break;
								case 2:
									ballY = n;
									break;
								case 3:
									ballZ = n;
									break;
								case 4:
									P1YS = n;
									break;
								case 5:
									P1YE = n;
									break;
								case 6:
									P2YS = n;
									break;
								case 7:
									P2YE = n;
									break;
								}
							}
						}
						string += offset;
					}
				}
				regfree(&regexCompiled);

				//Assign values, if suitable
				if ( (game->mode == 2) || ( (abs(game->ball.x - ballX) <= game->step_len) && (abs(game->ball.y - ballY) <= game->step_len) ) ) { // verify validity of received value. Client accepts always
					game->ball.x = ballX;
					game->ball.y = ballY;
					game->ball.direction = ballZ;
				}
				if (game->mode == 2) {	// update p1 only if you are client
					game->player1.y_start = P1YS;
					game->player1.y_end = P1YE;
				}
				if (game->mode == 1) {	// update p2 only if you are client
					game->player2.y_start = P2YS;
					game->player2.y_end = P2YE;
				}
			}
		}
	}
	return 1;
}

void idle(PingPong* game, int forced) {

	game->paused = 1;

	if (forced == 0) {
		char * cmd1 = "<START><cmd>2<END>";
		if (game->mode == 1)
			server_sendMessage(game->server, 0, game->speed, cmd1);
		else if (game->mode == 2)
			client_sendMessage(game->client, 0, game->speed, cmd1);
		log2file(game->mode, "Game paused. Json sent", cmd1);
	}

	int key;
	char * msg = (char *)malloc(101 * sizeof(char));
	while ( game->paused == 1 ) {
		if ( (key = getCharTimeout(game->speed)) == backspace)
			break;
		else if (key == escape)
			finish(game, 0);
		if (game->mode == 1)
			msg = server_getMessage(game->server, 0, game->speed);
		else if (game->mode == 2)
			msg = client_getMessage(game->client, 0, game->speed);
		jsonParser(game, msg);
		//log2file(game->mode, "Game paused. Json rcvd", msg);
	}

	if (key == backspace) {
		char * cmd2 = "<START><cmd>3<END>";
		if (game->mode == 1)
			server_sendMessage(game->server, 0, game->speed, cmd2);
		else if (game->mode == 2)
			client_sendMessage(game->client, 0, game->speed, cmd2);
		log2file(game->mode, "Game resumed. Json sent", cmd2);
	}
	game->paused = 0;
}

/*int main (int argc, char *argv[]) {
	PingPong* game = newGame(80,25,3,1,0);	// weight,height,size,speed,mode
	start(game);
    finish(game);
    return 0;
}*/
