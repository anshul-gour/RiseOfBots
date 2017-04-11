/*
 *
 *  RISE OF BOTS
 *
 *  OBJECTS_H
 *
 *
 */

#include "aibots.h"

#include <string.h>
#include <math.h>

#include "OsIncl.h"

#define PLAYERPORT 6670

static int s;
static int unread;

void (*oldsignal)(int);

static void sig_term(int signo)
{
	close(s);
	socketCleanup();
	oldsignal(signo);
}

int NewMsgCount(void)
{
	return unread;
}

int Init(char * thename)
{
	struct sockaddr_in sa;
	char ch,name[20];

	socketSetup();

	s = socket(AF_INET,SOCK_STREAM,0);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(PLAYERPORT);
	sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	for(ch = 0;thename[ch] != '\0' && ch < 18;ch++)
		name[ch] = thename[ch];
	name[ch] = '\0';

	connect(s,(struct sockaddr *)&sa,sizeof(sa));

	oldsignal = signal(SIGTERM,sig_term);
	send(s,name,strlen(name)+1,0);
	recv(s, (char *)&ch,1,0);

	return ch;
}

int GetInfoEdan(int edanIndex, int * x,int * y,char * resource,char *upgradedTo,int *ismoving, int * life,
		unsigned long * lastscan, unsigned long * time)
{
	struct _pspos
	{
		char plen;
		char id;
		char edanid[sizeof(int)];
	} pspos;

	struct _gPLinfo
	{
		int x,y;
		char resource,stage;
		int ismoving,life;
		unsigned long lastscan,time;
		int unrd;
		char ch;
	} gPLinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'G';

	*((int*)&pspos.edanid) = edanIndex;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&gPLinfo,sizeof(gPLinfo),0);


	if(x!=NULL)
		*x = gPLinfo.x;
	if(y!=NULL)
		*y = gPLinfo.y;
	if(resource!=NULL)
		*resource = gPLinfo.resource;
	if(upgradedTo != NULL)
		*upgradedTo = gPLinfo.stage;
	if(ismoving!=NULL)
		*ismoving = gPLinfo.ismoving;
	if(time!=NULL)
		*time = gPLinfo.time;
	if(life!=NULL)
		*life = gPLinfo.life;
	if(lastscan!=NULL)
		*lastscan = gPLinfo.lastscan;
	unread = gPLinfo.unrd;

	if(gPLinfo.ch == 1) 
		return (1);
	else 
		return (0);
}

int GetInfoEnemy(int edanIndex, int * x,int * y,char * resource, int * life)
{
	struct _pspos
	{
		char plen;
		char id;
		char edanid[sizeof(int)];
	} pspos;

	struct _gPLOinfo
	{
		int x,y;
		char resource;
		int life;
		int unrd;
		char ch;
	} gPLOinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'g';

	*((int*)&pspos.edanid) = edanIndex;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&gPLOinfo,sizeof(gPLOinfo),0);


	if(x!=NULL)
		*x = gPLOinfo.x;
	if(y!=NULL)
		*y = gPLOinfo.y;
	if(resource!=NULL)
		*resource = gPLOinfo.resource;
	if(life!=NULL)
		*life = gPLOinfo.life;
	unread = gPLOinfo.unrd;

	if(gPLOinfo.ch == 1) 
		return (1);
	else 
		return (0);
}

int GetInfoNost(int nostIndex, int * x,int * y,int * haveFood,int * haveTimber,
	int * haveMetal,int * life,unsigned long * time)
{
	struct _pspos
	{
		char plen;
		char id;
		char nostid[sizeof(int)];
	} pspos;

	struct _gCTinfo
	{
		int x,y,life;
		char haveFood,haveTimber,haveMetal;
		unsigned long time;
		int unrd;
		char ch;
	} gCTinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'H';
	*((int*)&pspos.nostid) = nostIndex;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&gCTinfo,sizeof(gCTinfo),0);

	
	if(x!=NULL)
		*x = gCTinfo.x;
	if(y!=NULL)
		*y = gCTinfo.y;
	if(haveFood!=NULL)
		*haveFood = gCTinfo.haveFood;
	if(haveTimber!=NULL)
		*haveTimber = gCTinfo.haveTimber;
	if(haveMetal!=NULL)
		*haveMetal = gCTinfo.haveMetal;
	if(life!=NULL)
		*life = gCTinfo.life;
		if(time!=NULL)
		*time = gCTinfo.time;
	unread = gCTinfo.unrd;

	if(gCTinfo.ch == 1) 
		return (1);
	else 
		return (0);
}

int LocateResource(char type,int *x, int *y)
{
	struct _pspos
	{
		char plen;
		char id;
		char type;
	} pspos;

	struct _resourceinfo
	{
		int x;
		int y;
		int unrd;
		char ch;
	} resourceinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'V';
	pspos.type = type;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&resourceinfo,sizeof(resourceinfo),0);

	if(x!=NULL)
		*x = resourceinfo.x;
	if(y!=NULL)
		*y = resourceinfo.y;
	unread = resourceinfo.unrd;

	if(resourceinfo.ch == 1) 
		return (1);
	else 
		return (0);
}

int ScanEdan(int dir,int *scandir,int *scanrange,char *botid,char *resource,int *life)
{
	struct _pspos
	{
		char plen;
		char id;
		char dir[sizeof(int)];
	} pspos;

	struct _scPinfo
	{
		int dir;
		int range;
		char botid;
		char resource;
		int life;
		int unrd;
		char ch;
	} scPinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'x';
	*((int*)&pspos.dir) = dir;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&scPinfo,sizeof(scPinfo),0);

	unread = scPinfo.unrd;

	if( scandir != NULL)
		*(scandir) = scPinfo.dir;
	if( scanrange != NULL)
		*(scanrange) = scPinfo.range;
	if( botid != NULL)
		*(botid) = scPinfo.botid;
	if( resource != NULL)
		*(resource) = scPinfo.resource;
	if( life != NULL)
		*(life) = scPinfo.life;

	if(scPinfo.ch == 1)
		return (1);
	else
		return (0);
}

int ScanNost(int dir,int *scandir,int *scanrange,char *nostid)
{
	struct _pspos
	{
		char plen;
		char id;
		char dir[sizeof(int)];
	} pspos;

	struct _scCinfo
	{
		int dir;
		int range;
		char nostid;
		int unrd;
		char ch;
	} scCinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'y';
	*((int*)&pspos.dir) = dir;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&scCinfo,sizeof(scCinfo),0);

	unread = scCinfo.unrd;

	if( scandir != NULL)
		*(scandir) = scCinfo.dir;
	if( scanrange != NULL)
		*(scanrange) = scCinfo.range;
	if( nostid != NULL)
		*(nostid) = scCinfo.nostid;

	if(scCinfo.ch == 1)
		return (1);
	else
		return (0);
}

int ScanResource(int dir,int *scandir,int *scanrange,char *type)
{
	struct _pspos
	{
		char plen;
		char id;
		char dir[sizeof(int)];
	} pspos;

	struct _scGinfo
	{
		int dir;
		int range;
		char type;
		int unrd;
		char ch;
	} scGinfo;

	pspos.plen = sizeof(pspos);
	pspos.id = 'z';
	*((int*)&pspos.dir) = dir;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&scGinfo,sizeof(scGinfo),0);

	unread = scGinfo.unrd;

	if( scandir != NULL)
		*(scandir) = scGinfo.dir;
	if( scanrange != NULL)
		*(scanrange) = scGinfo.range;
	if( type != NULL)
		*(type) = scGinfo.type;

	if(scGinfo.ch == 1)
		return (1);
	else
		return (0);
}

int IsWall(int relX,int relY)
{
	struct _pspos
	{
		char plen;
		char id;
		char relX[sizeof(int)];
		char relY[sizeof(int)];
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'A';
	*((int*)&pspos.relX) = relX;
	*((int*)&pspos.relY) = relY;

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);
	unread = pckt.unrd;
	return pckt.ch;
}
int PutResource(int dir,int range)
{
	struct _pspos
	{
		char plen;
		char id;
		char dir[sizeof(int)];
		char range[sizeof(int)];
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'L';
	*((int*)&pspos.dir) = dir;
	*((int*)&pspos.range) = range;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);
	return pckt.ch;
}

int PassResourceToEdan(int pl)
{
	struct _pspos
	{
		char plen;
		char id;
		char plid[sizeof(int)];
	}pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;
	
	pspos.plen = sizeof(pspos);
	pspos.id = 'P';
	*( (int*) &pspos.plid ) = pl;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int PassResourceToNost(int nost)
{
	struct _pspos
	{
		char plen;
		char id;
		char ctid[sizeof(int)];
	}pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;
	
	pspos.plen = sizeof(pspos);
	pspos.id = 'Q';
	*( (int*) &pspos.ctid ) = nost;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int GetResourceFromNost(int ct,char resourceType)
{
	struct _pspos
	{
		char plen;
		char id;
		char ctid[sizeof(int)];
		char resourceType;
	}pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	} pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'O';
	*((int*) &pspos.ctid ) = ct;
	pspos.resourceType = resourceType;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int Move(int dir,int steps)
{
	struct _pspos
	{
		char plen;
		char id;
		char dir[sizeof(int)];
		char steps[sizeof(int)];
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	} pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'M';
	*((int*)&pspos.dir) = dir;
	*((int*)&pspos.steps) = steps;

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

void Stop(void)
{
	struct _pspos
	{
		char plen;
		char id;
	} pspos;

	pspos.plen = sizeof(pspos);
	pspos.id = 'S';

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&unread,sizeof(int),0);
}

int Fire(int dir,int range)
{
	struct _pspos
	{
		char plen;
		char id;
		char dir[sizeof(int)];
		char range[sizeof(int)];
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'F';

	*((int*)&pspos.dir) = dir;
	*((int*)&pspos.range) = range;

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&pckt,sizeof(pckt),0);
	
	unread = pckt.unrd;
	return pckt.ch;
}

void BroadCast(char * msg)
{
	char ch;
	struct _pspos
	{
		char plen;
		char id;
		char m[11];
	}pspos;

	pspos.plen = sizeof(pspos);
	pspos.id = 'B';

	for(ch = 0;ch < 10 && msg[ch] != '\0';ch++)
		pspos.m[ch] = msg[ch];

	pspos.m[ch] = '\0';

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&unread,sizeof(int),0);
}

int MsgSend(int edanindex,char *msg)
{
	char ch;
	struct _pspos
	{
		char plen;
		char id;
		char plid[sizeof(int)];
		char m[11];
	}pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'D';

	*( (int*) &pspos.plid )= edanindex;


	for(ch = 0;ch < 10 && msg[ch] != '\0';ch++)
		pspos.m[ch] = msg[ch];

	pspos.m[ch] = '\0';

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int MsgRecv(char *msg)
{
	struct _pspos
	{
		char plen;
		char id;
	}pspos;

	typedef struct _message
	{
		char characters[11];
	}message;

	struct _inmsg
	{
		int index;
		message m;
		int unrd;
	}inmsg;

	pspos.plen = sizeof(pspos);
	pspos.id = 'R';

	send(s, (const char *)&pspos,sizeof(pspos),0);
	recv(s, (char *)&inmsg,sizeof(inmsg),0);

	strcpy(msg,inmsg.m.characters);
	unread = inmsg.unrd;
	return inmsg.index;

}

int MakeNost(int nostIndex, int relX, int relY)
{
	struct _pspos
	{
		char plen;
		char id;
		char nostId[sizeof(int)];
		char relX[sizeof(int)];
		char relY[sizeof(int)];
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'N';
	*((int *)&pspos.nostId) = nostIndex;
 	*((int *)&pspos.relX) = relX;
	*((int *)&pspos.relY) = relY;

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int UpgradeEdan(int nostIndex)
{
	struct _pspos
	{
		char plen;
		char id;
		char nostId[sizeof(int)];
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'U';
	*((int *)&pspos.nostId) = nostIndex;

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int PowerEdan(int nostIndex,char type)
{
	struct _pspos
	{
		char plen;
		char id;
		char nostId[sizeof(int)];
		char type;
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'u';
	*((int *)&pspos.nostId) = nostIndex;
	pspos.type = type;

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int HavePower(char type)
{
	struct _pspos
	{
		char plen;
		char id;
		char type;
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'e';
	pspos.type = type;

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}

int UpgradeNost(int nostIndex ,char type)
{
	struct _pspos
	{
		char plen;
		char id;
		char nostId[sizeof(int)];
		char type;
	} pspos;

	struct _pckt
	{
		char ch;
		int unrd;
	}pckt;

	pspos.plen = sizeof(pspos);
	pspos.id = 'W';
	*((int *)&pspos.nostId) = nostIndex;
	pspos.type = type; 

	send(s,(const char *)&pspos,sizeof(pspos),0);
	recv(s,(char *)&pckt,sizeof(pckt),0);

	unread = pckt.unrd;
	return pckt.ch;
}
