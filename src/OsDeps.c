#ifndef __OsDeps__
#define __OsDeps__

#include "OsIncl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void delay(unsigned long int usec)
{
#ifdef WIN32
        fd_set rdfs;
	struct timeval tv;
        int s = socket(AF_INET,SOCK_STREAM,0);

        FD_ZERO(&rdfs);
        FD_SET(s,&rdfs);
        tv.tv_sec = usec/1000000;
        tv.tv_usec = usec%1000000;

        select(s,&rdfs,NULL,NULL,&tv);
#endif
#ifdef LINUX
	struct timeval tv;
	tv.tv_sec = usec/1000000;
	tv.tv_usec = usec%1000000;
	select(0,NULL,NULL,NULL,&tv);
#endif
}

#ifdef WIN32
#ifdef __cplusplus
void close(int so)
{
        closesocket(so);
}
#else
        #define close(so) closesocket(so)
#endif
#endif

int socketSetup()
{
#ifdef WIN32
	WORD wVersionRequested = MAKEWORD(1,1);
	WSADATA wsaData;
	WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr,"Wrong Version!");
		return(0);
	}
	else return(1);
#else
	return(1);
#endif
}
void socketCleanup()
{
#ifdef WIN32
	WSACleanup();
#endif
}

PID createFork(char *cmdline, unsigned long int lfd)
{
#ifdef WIN32
	PROCESS_INFORMATION fgProc;
	STARTUPINFO         procSU;

char cmd[128];
strcpy(cmd, cmdline);
strcat(cmd,".exe");
	GetStartupInfo(&procSU);
CreateProcess(NULL, cmdline, NULL, NULL, TRUE, 0, NULL, NULL, &procSU, &fgProc);
	return(fgProc.hProcess);
#endif
#ifdef LINUX
	PID pid;
	char *argv[2];
	if((pid = fork()) == 0)
	{
		strcpy(argv[0], cmdline);
		argv[1] = 0;
		if (lfd != 0)
			close(lfd);
		execv(cmdline,argv);
		exit(0);
	}
	return(pid);
#endif
}

void terminateFork(PID pid)
{
#ifdef WIN32
	TerminateProcess(pid, SIGTERM);
#endif
#ifdef LINUX
	kill(pid,SIGTERM);
#endif
}

#endif
