#ifndef __OsIncl__
#define __OsIncl__

#ifdef WIN32
	#include <time.h>
        #include <winsock.h>
	#include <signal.h>
#endif
#ifdef LINUX
	#include <unistd.h>
	#include <signal.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#ifdef WIN32
	#define color_set(a, b) attron(COLOR_PAIR(a))
#endif

#ifndef M_PI
	#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef PID
#ifdef WIN32
	#define PID HANDLE
#endif
#ifdef LINUX
	#define PID int
#endif
#endif

extern void delay(unsigned long int);
extern int socketSetup();
extern void socketCleanup();
extern PID createFork(char *, unsigned long int);
extern void terminateFork(PID);

#ifdef WIN32
#ifdef __cplusplus
        extern void close(int);
#endif
#endif

#endif
