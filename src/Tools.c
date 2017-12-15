#include "Tools.h"

void beep(int freq, int dur) {	// Make a sound

	//dur = 5000; freq = 440;
	#if WIN64 || WIN32
		Beep(freq, dur);
	#elif UNIX|| __linux__
		int fd = open("/dev/console", O_WRONLY); // = STDOUT_FILENO; // = open("/dev/tty10", O_RDONLY);
		if (fd > 0) {
			ioctl(fd, KIOCSOUND, 1193180 / freq);
			usleep(dur * 1000);
			ioctl(fd, KIOCSOUND, 0);
		}
		else
			fprintf(stderr, "Failed to open console.\n");
#endif
}

void clear() {	// Clear the screen
	//www.ascii-table.com/ansi-escape-sequences.php
	printf("\033[2J");			// [J : clear the entire screen and moves to home (2)
	printf("\033[1;1H");		// [H : reset the cursor to the top-left (1;1)

	#if WIN64 || WIN32
		HANDLE hOut;
		COORD Position;
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		Position.X = 0;
		Position.Y = 0;
		SetConsoleCursorPosition(hOut, Position);
	#endif
}

int getCharTimeout(int timeout){	// Ask for a char within a time limit

	int a;

	#if WIN64 || WIN32	// Use handles because WSAPoll applies only to sockets
		HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
		DWORD mode, old_mode;
		GetConsoleMode(hStdIn, &old_mode);	// Memorize original input mode
		mode = old_mode ^ ENABLE_MOUSE_INPUT ^ ENABLE_WINDOW_INPUT; //ENABLE_PROCESSED_INPUT ENABLE_LINE_INPUT ENABLE_MOUSE_INPUT
		SetConsoleMode(hStdIn, mode);	// Switch to raw input mode
		FlushConsoleInputBuffer(hStdIn);
		if (WaitForSingleObject(hStdIn, timeout) == WAIT_OBJECT_0)
			a = _getch();				// or ReadConsole(h, &ch, 1, &n, 0);
		else
			a = -1;
		SetConsoleMode(hStdIn, old_mode);	// Switch back to the original input mode
		FlushConsoleInputBuffer(hStdIn);
	#elif UNIX|| __linux__
		struct pollfd mypoll[1];			// or: mypoll = { STDIN_FILENO, POLLIN|POLLPRI, -1 };
		mypoll[0].fd = STDIN_FILENO;
		mypoll[0].events = POLLIN | POLLPRI; // alert when data data is read or out-band
		mypoll[0].revents = -1;
		#if WIN64 || WIN32
			int poll_ = WSAPoll(&mypoll, 1, 50000);
		#elif UNIX|| __linux__
			int poll_ = poll(mypoll, 1, timeout);
		#endif

		if (poll_ > 0) {
			#if WIN64 || WIN32
				a = _getch();
			#elif UNIX|| __linux__
				a = fgetc(stdin);
			#endif
		}
		else {
			a = -1;
		}
	#endif

	return a;
}

int log2file(int referrer, char* text1, char* text2) {	// Save an app message into a text file
	char *info;
	info = (referrer == 1) ? "Server - " : "Client - ";

	char dateTime[20];
	struct tm *sTm;
	time_t now = time(0);
	sTm = gmtime(&now);	// localtime (&now);
	strftime(dateTime, sizeof(dateTime), "%Y-%m-%d %H:%M:%S", sTm);

	FILE *f = fopen("game.log", "a");
	if (f == NULL) {
		printf("\nError opening file!\n");
		return 0;
	}
	else {
		fprintf(f, "\n%s%s - %s *** %s\n", info, dateTime, text1, text2);
		fclose(f);
		return 1;
	}
}
